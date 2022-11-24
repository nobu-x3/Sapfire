#include "Ship.h"
#include "AnimSpriteComponent.h"
#include "Game.h"
#include "SpriteComponent.h"
#include <SDL_scancode.h>
#include <vector>

Ship::Ship(Game *game) : Actor(game), mRightSpeed(0.0f), mDownSpeed(0.0f)
{
	// Create an animated sprite component
	mAnimComponent = new AnimSpriteComponent(this);
	std::vector<SDL_Texture *> anims = {
	    game->LoadTexture("../Assets/Ship01.png"),
	    game->LoadTexture("../Assets/Ship02.png"),
	    game->LoadTexture("../Assets/Ship03.png"),
	    game->LoadTexture("../Assets/Ship04.png"),
	};

	std::vector<SDL_Texture *> anims1 = {
	    game->LoadTexture("../Assets/Character01.png"), game->LoadTexture("../Assets/Character02.png"),
	    game->LoadTexture("../Assets/Character03.png"), game->LoadTexture("../Assets/Character04.png"),
	    game->LoadTexture("../Assets/Character05.png"), game->LoadTexture("../Assets/Character06.png"),
	    game->LoadTexture("../Assets/Character07.png"), game->LoadTexture("../Assets/Character08.png"),
	    game->LoadTexture("../Assets/Character09.png"),
	};
	AnimData data("anim_flying", anims, true);
	AnimData data1("anim_character", anims1, false);

	mAnimComponent->AddAnim(data);
	mAnimComponent->AddAnim(data1);
}

void Ship::UpdateActor(float deltaTime)
{
	Actor::UpdateActor(deltaTime);
	// Update position based on speeds and delta time
	Vector2 pos = GetPosition();
	pos.x += mRightSpeed * deltaTime;
	pos.y += mDownSpeed * deltaTime;
	// Restrict position to left half of screen
	if (pos.x < 25.0f)
	{
		pos.x = 25.0f;
	}
	else if (pos.x > 500.0f)
	{
		pos.x = 500.0f;
	}
	if (pos.y < 25.0f)
	{
		pos.y = 25.0f;
	}
	else if (pos.y > 743.0f)
	{
		pos.y = 743.0f;
	}
	SetPosition(pos);
}

void Ship::ProcessKeyboard(const uint8_t *state)
{
	mRightSpeed = 0.0f;
	mDownSpeed = 0.0f;
	// right/left
	if (state[SDL_SCANCODE_D])
	{
		mRightSpeed += 250.0f;
	}
	if (state[SDL_SCANCODE_A])
	{
		mRightSpeed -= 250.0f;
	}
	// up/down
	if (state[SDL_SCANCODE_S])
	{
		mDownSpeed += 300.0f;
	}
	if (state[SDL_SCANCODE_W])
	{
		mDownSpeed -= 300.0f;
	}
	if (state[SDL_SCANCODE_SPACE])
	{
		mAnimComponent->PlayAnimation("anim_character");
	}
}
