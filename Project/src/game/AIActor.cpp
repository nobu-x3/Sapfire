#include "AIActor.h"
#include "IdleState.h"
#include "engine/AnimSpriteComponent.h"
#include "engine/Game.h"
#include "engine/StateMachine.h"
AIActor::AIActor(Game *game) : Actor(game)
{
	mStateMachine = new StateMachine(this);
	mAnimSpriteComponent = new AnimSpriteComponent(this);
	std::vector<SDL_Texture *> idleAnim = {game->LoadTexture("../Assets/Character05.png")};
	std::vector<SDL_Texture *> walkingAnim = {
	    game->LoadTexture("../Assets/Character01.png"), game->LoadTexture("../Assets/Character02.png"),
	    game->LoadTexture("../Assets/Character03.png"), game->LoadTexture("../Assets/Character04.png"),
	    game->LoadTexture("../Assets/Character05.png"), game->LoadTexture("../Assets/Character06.png"),
	    game->LoadTexture("../Assets/Character07.png"), game->LoadTexture("../Assets/Character08.png"),
	    game->LoadTexture("../Assets/Character09.png"),
	};

	std::vector<SDL_Texture *> jumpAnim = {
	    game->LoadTexture("../Assets/Character07.png"), game->LoadTexture("../Assets/Character08.png"),
	    game->LoadTexture("../Assets/Character09.png"), game->LoadTexture("../Assets/Character10.png"),
	    game->LoadTexture("../Assets/Character11.png"), game->LoadTexture("../Assets/Character12.png"),
	    game->LoadTexture("../Assets/Character13.png"), game->LoadTexture("../Assets/Character14.png"),
	    game->LoadTexture("../Assets/Character15.png")};

	AnimData idleData("idle", idleAnim, true);
	AnimData walkingData("walk", walkingAnim, true);
	AnimData jumpData("jump", jumpAnim, false);

	mAnimSpriteComponent->AddAnim(idleData);
	mAnimSpriteComponent->AddAnim(walkingData);
	mAnimSpriteComponent->AddAnim(jumpData);

	IdleState *idleState = new IdleState(mStateMachine, mAnimSpriteComponent); // this might leak memory
	mStateMachine->RegisterState(idleState);

	mStateMachine->ChangeState("idle");
}
