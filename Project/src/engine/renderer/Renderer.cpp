#include "Renderer.h"
#include "engine/Math.h"
#include "engine/Mesh.h"
#include "engine/MeshComponent.h"
#include "engine/SpriteComponent.h"
#include "engine/events/WindowEvent.h"
#include "engine/renderer/Shader.h"
#include "engine/renderer/Texture.h"
#include "engine/renderer/VertexArray.h"
#include "engine/renderer/opengl/OpenGLContext.h"
#include <SDL2/SDL.h>

RendererAPI Renderer::sRendererAPI = RendererAPI::OpenGL;
WindowAPI Renderer::sWindowAPI = WindowAPI::SDL;

Renderer::Renderer(Game *game) : mGame(game), mSpriteShader(nullptr)
{
}

bool Renderer::Initialize(float width, float height)
{
	int sdlResult = SDL_Init(SDL_INIT_VIDEO);
	mScreenWidth = width;
	mScreenHeight = height;
	if (sdlResult != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}
	mWindow =
	    SDL_CreateWindow("Rocket", 100, 100, static_cast<int>(width), static_cast<int>(height), SDL_WINDOW_OPENGL);
	if (!mWindow)
	{
		SDL_Log("Unable to initialize window: %s", SDL_GetError());
		return false;
	}
	mRenderingContext = new OpenGLContext(mWindow);
	mRenderingContext->Init();

	if (!LoadShaders())
	{
		SDL_Log("Failed to load shaders, exiting");
		return false;
	}

	CreateSpriteVerts();
	return true;
}

void Renderer::Shutdown()
{
	/* delete mSpriteVerts; */
	mSpriteShader->Unload();
	delete mSpriteShader;
	for (auto pair : mShaderMeshCompMap)
	{
		pair.first->Unload();
		delete pair.first;
		pair.second.clear();
	}
	mShaderMeshCompMap.clear();
	// SDL_GL_DeleteContext(mContext);
	delete mRenderingContext;
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
}

void Renderer::Draw()
{
	// set color to grey
	glClearColor(0.83f, 0.83f, 0.83f, 1.0f);

	// clear color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Mesh components
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	auto viewProj = mView * mProjection;
	for (auto pair : mShaderMeshCompMap)
	{
		pair.first->Bind();
		pair.first->SetMatrixUniform("uViewProj", mView * mProjection);
		SetLightUniforms(pair.first);
		for (auto meshComp : pair.second)
		{
			meshComp->Draw(pair.first);
		}
	}

	// Sprite components
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	// TODO: draw scene
	mSpriteShader->Bind();
	/* mSpriteVerts->SetActive(); */
	/* for (auto sprite : mSprites) */
	/* { */
	/* 	/1* sprite->Draw(mSpriteShader); *1/ */
	/* } */
	SDL_GL_SwapWindow(mWindow);
}

void Renderer::AddSprite(SpriteComponent *sprite)
{
	// Find insertion point in sorted vector
	int drawOrder = sprite->GetDrawOrder();
	auto iter = mSprites.begin();
	for (; iter != mSprites.end(); ++iter)
	{
		if (drawOrder < (*iter)->GetDrawOrder())
			break;
	}

	mSprites.insert(iter, sprite);
}

void Renderer::RemoveSprite(SpriteComponent *sprite)
{
	auto iter = std::find(mSprites.begin(), mSprites.end(), sprite);
	if (iter != mSprites.end())
		mSprites.erase(iter);
}

void Renderer::AddMeshComponent(MeshComponent *mesh)
{
	mMeshComponents.emplace_back(mesh);
}

void Renderer::RemoveMeshComponent(MeshComponent *mesh)
{
	auto iter = std::find(mMeshComponents.begin(), mMeshComponents.end(), mesh);
	mMeshComponents.erase(iter);
}
Ref<Texture> Renderer::GetTexture(const char *fileName)
{
	Ref<Texture> tex = nullptr;
	auto iter = mTextures.find(fileName);
	if (iter != mTextures.end())
	{
		tex = iter->second;
	}
	else
	{
		tex = Texture::Create(fileName);
		mTextures.emplace(fileName, tex);
	}
	return tex;
}

Mesh *Renderer::GetMesh(const char *fileName)
{
	Mesh *m = nullptr;
	auto iter = mMeshes.find(fileName);
	if (iter != mMeshes.end())
	{
		m = iter->second;
	}
	else
	{
		m = new Mesh();
		if (m->Load(fileName, this))
		{
			mMeshes.emplace(fileName, m);
		}
		else
		{
			delete m;
			m = nullptr;
		}
	}
	return m;
}

void Renderer::LinkShaderToMeshComp(const std::string &fileName, MeshComponent *meshComp)
{
	Shader *shader = GetShader(fileName);
	if (shader == nullptr)
	{
		return;
	}
	auto iter = mShaderMeshCompMap.find(shader);
	if (iter != mShaderMeshCompMap.end())
	{
		iter->second.emplace_back(meshComp);
	}
	else
	{
		std::vector<MeshComponent *> vec;
		vec.emplace_back(meshComp);
		mShaderMeshCompMap.emplace(shader, vec);
	}
}
Shader *Renderer::GetShader(const std::string &fileName)
{
	Shader *sh = nullptr;
	auto iter = mShaders.find(fileName);
	if (iter != mShaders.end())
	{
		sh = iter->second;
	}
	else
	{
		sh = Shader::Create();
		if (sh->Load(fileName + ".vert", fileName + ".frag"))
		{
			mShaders.emplace(fileName, sh);
			LoadShader(sh);
		}
		else
		{
			SDL_Log("Could not find shader %s.", fileName.c_str());
			delete sh;
			sh = nullptr;
		}
	}
	return sh;
}

void Renderer::UnloadData()
{
	mTextures.clear();
	for (auto mesh : mMeshes)
	{
		mesh.second->Unload();
		delete mesh.second;
	}
	mMeshes.clear();
}

void Renderer::LoadShader(Shader *sh)
{
	sh->Bind();
	mView = Matrix4::CreateLookAt(Vector3::Zero,  // Camera pos
				      Vector3::UnitX, // Target pos
				      Vector3::UnitZ  // Up
	);
	mProjection = Matrix4::CreatePerspectiveFOV(Math::ToRadians(70.f), mScreenWidth, mScreenHeight, 25.0f, 10000.f);
	sh->SetMatrixUniform("uViewProj", mView * mProjection);
}

bool Renderer::LoadShaders()
{
	// 2D stuff
	mSpriteShader = Shader::Create();
	if (!mSpriteShader->Load("../Shaders/Sprite.vert", "../Shaders/Sprite.frag"))
	{
		return false;
	}
	mSpriteShader->Bind();
	Matrix4 viewProj = Matrix4::CreateSimpleViewProj(1024.f, 768.f);
	mSpriteShader->SetMatrixUniform("uViewProj", viewProj);

	return true;
}

void Renderer::SetLightUniforms(Shader *shader)
{
	Matrix4 invertedView = mView;
	invertedView.Invert();
	shader->SetVectorUniform("uCameraPos", invertedView.GetTranslation());
	shader->SetVectorUniform("uAmbientLight", mAmbientLight);
	shader->SetVectorUniform("uDirLight.mDirection", mDirectionalLight.mDirection);
	shader->SetVectorUniform("uDirLight.mDiffuseColor", mDirectionalLight.mDiffuseColor);
	shader->SetVectorUniform("uDirLight.mSpecColor", mDirectionalLight.mSpecColor);
	for (int i = 0; i < mPointLights.size(); ++i)
	{
		shader->SetVectorUniform("uPointLights[" + std::to_string(i) + "].mPosition",
					 mPointLights[i].mPosition);
		shader->SetVectorUniform("uPointLights[" + std::to_string(i) + "].mDiffuseColor",
					 mPointLights[i].mDiffuseColor);
		shader->SetVectorUniform("uPointLights[" + std::to_string(i) + "].mSpecColor",
					 mPointLights[i].mSpecColor);
		shader->SetFloatUniform("uPointLights[" + std::to_string(i) + "].mRadius", mPointLights[i].mRadius);
		shader->SetFloatUniform("uPointLights[" + std::to_string(i) + "].mIntensity",
					mPointLights[i].mIntensity);
	}
}
void Renderer::CreateSpriteVerts()
{
	float vertices[] = {
	    -0.5f, 0.5f,  0.f, 0.f, 0.f, 0.f, 0.f, 0.f, // top left
	    0.5f,  0.5f,  0.f, 0.f, 0.f, 0.f, 1.f, 0.f, // top right
	    0.5f,  -0.5f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, // bottom right
	    -0.5f, -0.5f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f	// bottom left
	};

	unsigned int indices[] = {0, 1, 2, 2, 3, 0};

	/* mSpriteVerts = */
}
