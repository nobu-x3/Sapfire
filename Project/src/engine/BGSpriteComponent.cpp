#include "BGSpriteComponent.h"
#include "SpriteComponent.h"
#include "engine/Texture.h"
#include <SDL_rect.h>
#include <SDL_render.h>

BGSpriteComponent::BGSpriteComponent(Actor *owner, int drawOrder)
    : SpriteComponent(owner, drawOrder), mScreenSize(Vector2(0, 0)), mScrollSpeed(0)
{
}

void BGSpriteComponent::SetBGTextures(const std::vector<Texture *> &bgTextures)
{
	int count = 0;
	for (auto tex : bgTextures)
	{
		BGTexture temp;
		temp.mTexture = tex;

		temp.mOffset.x = count * mScreenSize.x;
		temp.mOffset.y = 0;
		mBGTextures.emplace_back(temp);
		count++;
	}
}

void BGSpriteComponent::Update(float deltaTime)
{
	SpriteComponent::Update(deltaTime);
	for (auto &bg : mBGTextures)
	{
		bg.mOffset.x += mScrollSpeed * deltaTime;

		// Infinite scrolling, looping bg
		if (bg.mOffset.x < -mScreenSize.x)
		{
			bg.mOffset.x = (mBGTextures.size() - 1) * mScreenSize.x - 1;
		}
	}
}

/* void BGSpriteComponent::Draw(SDL_Renderer *renderer) */
/* { */
/* 	for (auto &bg : mBGTextures) */
/* 	{ */
/* 		SDL_Rect rect; */
/* 		rect.w = static_cast<int>(mScreenSize.x); */
/* 		rect.h = static_cast<int>(mScreenSize.y); */
/* 		rect.x = static_cast<int>(mOwner->GetPosition().x - rect.w / 2.f + bg.mOffset.x); */
/* 		rect.y = static_cast<int>(mOwner->GetPosition().y - rect.h / 2.f + bg.mOffset.y); */
/* 		SDL_RenderCopy(renderer, bg.mTexture, nullptr, &rect); */
/* 	} */
/* } */
