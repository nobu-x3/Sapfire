#include "SpriteComponent.h"
#include "Actor.h"
#include "Component.h"
#include "Game.h"
#include "Math.h"
#include "SDL2/SDL_image.h"
#include <SDL_render.h>

SpriteComponent::SpriteComponent(Actor *owner, int drawOrder)
    : Component(owner), mTexture(nullptr), mDrawOrder(drawOrder), mTexWidth(0), mTexHeight(0)
{
	owner->GetGame()->AddSprite(this);
}

SpriteComponent::~SpriteComponent()
{
	mOwner->GetGame()->RemoveSprite(this);
}

void SpriteComponent::SetTexture(SDL_Texture *texture)
{
	mTexture = texture;
	SDL_QueryTexture(texture, nullptr, nullptr, &mTexWidth, &mTexHeight);
}

void SpriteComponent::Draw(SDL_Renderer *renderer)
{
	SDL_Rect rect;
	rect.w = static_cast<int>(mTexWidth * mOwner->GetScale());
	rect.h = static_cast<int>(mTexHeight * mOwner->GetScale());
	rect.x = static_cast<int>(mOwner->GetPosition().x - rect.w / 2.f);
	rect.y = static_cast<int>(mOwner->GetPosition().y - rect.h / 2.f);

	SDL_RenderCopyEx(renderer, mTexture, nullptr, &rect, -Math::ToDegrees(mOwner->GetRotation()), nullptr,
			 SDL_FLIP_NONE);
}
