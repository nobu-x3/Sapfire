#include "Game.h"
#include "BGSpriteComponent.h"
#include "GL/glew.h"
#include "SDL2/SDL_image.h"
#include "Shader.h"
#include "SpriteComponent.h"
#include "VertexArray.h"
#include "game/AIActor.h"
#include <SDL.h>
#include <SDL_error.h>
#include <SDL_log.h>
#include <SDL_render.h>
#include <SDL_surface.h>
#include <SDL_timer.h>
#include <SDL_video.h>
#include <algorithm>
#include <iostream>
const int thickness = 15;
const int paddleH = 150;

Game::Game()
{
	mWindow = nullptr;
	mIsRunning = true;
	mTicksCount = 0;
}

Game::~Game()
{
	while (!mActors.empty())
	{
		delete mActors.back();
	}
}

void Game::LoadData()
{
	mAiActor = new AIActor(this);
	mAiActor->SetPosition(Vector2(512.f, 384.f));
	// Create actor for the background (this doesn't need a subclass)
	Actor *bgActor = new Actor(this);
	bgActor->SetPosition(Vector2(512.0f, 384.0f));
	// Create the "far back" background
	BGSpriteComponent *bg = new BGSpriteComponent(bgActor);
	bg->SetScreenSize(Vector2(1024.0f, 768.0f));
	std::vector<SDL_Texture *> bgtexs = {LoadTexture("../Assets/Farback01.png"),
					     LoadTexture("../Assets/Farback02.png")};
	bg->SetBGTextures(bgtexs);
	bg->SetScrollSpeed(-100.0f);
	// Create the closer background
	bg = new BGSpriteComponent(bgActor, 50);
	bg->SetScreenSize(Vector2(1024.0f, 768.0f));
	bgtexs = {LoadTexture("../Assets/Stars.png"), LoadTexture("../Assets/Stars.png")};
	bg->SetBGTextures(bgtexs);
	bg->SetScrollSpeed(-200.0f);

	Actor *testActor = new Actor(this);
	SpriteComponent *spriteComp = new SpriteComponent(testActor);
}

bool Game::LoadShaders()
{
	mSpriteShader = std::make_unique<Shader>();
	if (!mSpriteShader->Load("../Shaders/Basic.vert", "../Shaders/Basic.frag"))
	{
		return false;
	}
	return true;
}

void Game::CreateSpriteVerts()
{
	float vertices[] = {
	    -0.5f, 0.5f,  0.f, 0.f, 0.f, // top left
	    0.5f,  0.5f,  0.f, 1.f, 0.f, // top right
	    0.5f,  -0.5f, 0.f, 1.f, 1.f, // bottom right
	    -0.5f, -0.5f, 0.f, 0.f, 1.f	 // bottom left
	};

	unsigned int indices[] = {0, 1, 2, 2, 3, 0};

	mSpriteVerts = std::make_unique<VertexArray>(vertices, 4, indices, 6);
}
void Game::UnloadData()
{
	// Delete actors
	// Because ~Actor calls RemoveActor, have to use a different style loop
	while (!mActors.empty())
	{
		delete mActors.back();
	}

	// Destroy textures
	for (auto i : mTextures)
	{
		SDL_DestroyTexture(i.second);
	}
	mTextures.clear();
}

bool Game::Initialize()
{
	int sdlResult = SDL_Init(SDL_INIT_VIDEO);
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
	mWindow = SDL_CreateWindow("Rocket", 100, 100, 1024, 768, SDL_WINDOW_OPENGL);
	if (!mWindow)
	{
		SDL_Log("Unable to initialize window: %s", SDL_GetError());
		return false;
	}

	// OpenGL context
	mContext = SDL_GL_CreateContext(mWindow);

	// GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		SDL_Log("Failed to initialize GLEW.");
		return false;
	}
	glGetError(); // to clean benign error code
	LoadShaders();

	/* mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); */
	/* if (!mRenderer) */
	/* { */
	/* 	SDL_Log("Unable to initialize renderer: %s", SDL_GetError()); */
	/* 	return false; */
	/* } */
	if(IMG_Init(IMG_INIT_PNG) == 0)
	{
		SDL_Log("Unable to initalize SDL_Image: %s", SDL_GetError());
	}

	/*	SDL_Texture *texture = LoadTexture("../Assets/Character01.png");
		Actor *testActor = new Actor(this);
		testActor->SetPosition(Vector2(512.f, 384.f));
		SpriteComponent *testSpriteComp = new SpriteComponent(testActor, 1);
		testSpriteComp->SetTexture(texture);

		testActor->AddComponent(testSpriteComp);*/ // THIS LEAKS MEMORY
	CreateSpriteVerts();
	LoadData();
	mTicksCount = SDL_GetTicks();
	return true;
}

SDL_Texture *Game::LoadTexture(const char *fileName)
{
	SDL_Texture *tex = nullptr;
	/* // Is the texture already in the map? */
	/* auto iter = mTextures.find(fileName); */
	/* if (iter != mTextures.end()) */
	/* { */
	/* 	tex = iter->second; */
	/* } */
	/* else */
	/* { */
	/* 	// Load from file */
	/* 	SDL_Surface *surf = IMG_Load(fileName); */
	/* 	if (!surf) */
	/* 	{ */
	/* 		SDL_Log("Failed to load texture file %s", fileName); */
	/* 		return nullptr; */
	/* 	} */

	/* 	// Create texture from surface */
	/* 	tex = SDL_CreateTextureFromSurface(mRenderer, surf); */
	/* 	SDL_FreeSurface(surf); */
	/* 	if (!tex) */
	/* 	{ */
	/* 		SDL_Log("Failed to convert surface to texture for %s", fileName); */
	/* 		return nullptr; */
	/* 	} */

	/* 	mTextures.emplace(fileName, tex); */
	/* } */
	return tex;
}

void Game::Update()
{
	while (mIsRunning)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::Shutdown()
{
	UnloadData();
	IMG_Quit();
	SDL_GL_DeleteContext(mContext);
	SDL_DestroyWindow(mWindow);
	/* SDL_DestroyRenderer(mRenderer); */
	SDL_Quit();
}
void Game::AddActor(Actor* actor)
{
	if (mUpdatingActors)
		mPendingActors.emplace_back(actor);
	else
		mActors.emplace_back(actor);
}

void Game::RemoveActor(Actor *actor)
{
	auto iter = std::find(mActors.begin(), mActors.end(), actor);
	if (iter != mActors.end())
		mActors.erase(iter);
}

void Game::AddSprite(SpriteComponent *sprite)
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

void Game::RemoveSprite(SpriteComponent *sprite)
{
	auto iter = std::find(mSprites.begin(), mSprites.end(), sprite);
	if (iter != mSprites.end())
		mSprites.erase(iter);
}

void Game::ProcessInput()
{
	SDL_Event event;

	// returns true if there are events in the q
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			mIsRunning = false;
			break;
		}
	}
	const Uint8 *state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_ESCAPE])
	{
		mIsRunning = false;
	}

	mUpdatingActors = true;
	for (auto actor : mActors)
	{
		actor->ProcessInput(state);
	}
	mUpdatingActors = false;
}

void Game::UpdateGame()
{
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16)); // limits to 60 fps

	// calc delta time
	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.f;
	mTicksCount = SDL_GetTicks();

	// clamp delta time to avoid big simulation jumps during debugging
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	mUpdatingActors = true;
	for(auto actor : mActors)
	{
		actor->Update(deltaTime);
	}
	mUpdatingActors = false;

	for(auto pending : mPendingActors)
	{
		mActors.emplace_back(pending);
	}
	mPendingActors.clear();

	std::vector<Actor*> actorsPendingRemoval;
	for(auto actor : mActors)
	{
		if (actor->GetState() == Actor::EPendingRemoval)
			actorsPendingRemoval.emplace_back(actor);
	}

	for(auto actor : actorsPendingRemoval)
	{
		delete actor;
	}
}

void Game::GenerateOutput()
{
	// set color to gray
	glClearColor(0.86f, 0.86f, 0.86f, 1.0f);

	// clear color buffer
	glClear(GL_COLOR_BUFFER_BIT);

	// TODO: draw scene
	mSpriteShader->SetActive();
	mSpriteVerts->SetActive();
	for (auto sprite : mSprites)
	{
		sprite->Draw(*mSpriteShader);
	}
	SDL_GL_SwapWindow(mWindow);
}
