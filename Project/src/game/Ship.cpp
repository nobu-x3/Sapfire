#include "Ship.h"
#include "engine/AnimSpriteComponent.h"
#include "engine/Game.h"
#include "engine/InputComponent.h"
#include "engine/SpriteComponent.h"
#include <SDL_scancode.h>
#include <vector>

Ship::Ship(Game *game) : Actor(game)
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

	mInputComponent = new InputComponent(this);
	mInputComponent->SetMaxAngularSpeed(5.0f);
	mInputComponent->SetMaxForwardSpeed(210.0f);
}

void Ship::ActorInput(const uint8_t *state)
{
	if (state[SDL_SCANCODE_SPACE])
	{
		mAnimComponent->PlayAnimation("anim_character");
	}
}
