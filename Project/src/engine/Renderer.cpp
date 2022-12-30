#include "Renderer.h"
#include "engine/Math.h"
#include "engine/Mesh.h"
#include "engine/MeshComponent.h"
#include "engine/Shader.h"
#include "engine/SpriteComponent.h"
#include "engine/Texture.h"
#include "engine/VertexArray.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <algorithm>

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
	// Set profile to core
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// Set version
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	// Request a color buffer with 8-bits per RGBA channel
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	// Enable double buffering
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// GPU (hardware) accel
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	mWindow =
	    SDL_CreateWindow("Rocket", 100, 100, static_cast<int>(width), static_cast<int>(height), SDL_WINDOW_OPENGL);
	if (!mWindow)
	{
		SDL_Log("Unable to initialize window: %s", SDL_GetError());
		return false;
	}

	// OpenGL context
	mContext = SDL_GL_CreateContext(mWindow);
	/* Mesh *mesh = new Mesh(); */
	/* if (mesh->Load("../Assets/Cube.sfmesh", this)) */
	/* { */
	/* 	SDL_Log("Mesh loaded"); */
	/* } */

	// GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		SDL_Log("Failed to initialize GLEW.");
		return false;
	}
	glGetError(); // to clean benign error code
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
	mSpriteShader->Unload();
	SDL_GL_DeleteContext(mContext);
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
}

void Renderer::Draw()
{
	// set color to grey
	glClearColor(0.86f, 0.86f, 0.86f, 1.0f);

	// clear color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Mesh components
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	mMeshShader->SetActive();
	mMeshShader->SetMatrixUniform("uViewProj", mView * mProjection);
	for (auto mesh : mMeshComponents)
	{
		mesh->Draw(mMeshShader);
	}

	// Sprite components
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,	     // srcFactor is srcAlpha
		    GL_ONE_MINUS_SRC_ALPHA); // destFact is 1 - srcAlpha
	// TODO: draw scene
	mSpriteShader->SetActive();
	mSpriteVerts->SetActive();
	for (auto sprite : mSprites)
	{
		sprite->Draw(mSpriteShader);
	}
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
Texture *Renderer::LoadTexture(const char *fileName)
{
	Texture *tex = nullptr;
	auto iter = mTextures.find(fileName);
	if (iter != mTextures.end())
	{
		tex = iter->second;
	}
	else
	{
		tex = new Texture();
		if (tex->Load(fileName))
		{
			mTextures.emplace(fileName, tex);
		}
		else
		{
			delete tex;
			tex = nullptr;
		}
	}
	return tex;
}

Mesh *Renderer::LoadMesh(const char *fileName)
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
void Renderer::UnloadData()
{
	// Destroy textures
	for (auto i : mTextures)
	{
		i.second->Unload();
		delete i.second;
	}
	delete mSpriteVerts;
	mTextures.clear();
}
bool Renderer::LoadShaders()
{
	// 2D stuff
	mSpriteShader = new Shader();
	if (!mSpriteShader->Load("../Shaders/Sprite.vert", "../Shaders/Sprite.frag"))
	{
		return false;
	}
	mSpriteShader->SetActive();
	Matrix4 viewProj = Matrix4::CreateSimpleViewProj(1024.f, 768.f);
	mSpriteShader->SetMatrixUniform("uViewProj", viewProj);

	// 3D stuff
	mMeshShader = new Shader();
	if (!mMeshShader->Load("../Shaders/BasicMesh.vert", "../Shaders/BasicMesh.frag"))
	{
		return false;
	}

	mMeshShader->SetActive();
	mView = Matrix4::CreateLookAt(Vector3::Zero,  // Camera pos
				      Vector3::UnitX, // Target pos
				      Vector3::UnitZ  // Up
	);
	mProjection = Matrix4::CreatePerspectiveFOV(Math::ToRadians(75.f), mScreenWidth, mScreenHeight, 25.0f, 10000.f);
	mMeshShader->SetMatrixUniform("uViewProj", mView * mProjection);
	return true;
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

	mSpriteVerts = new VertexArray(vertices, 4, indices, 6);
}
