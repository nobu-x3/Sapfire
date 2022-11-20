#include "AnimSpriteComponent.h"
#include "SpriteComponent.h"

AnimSpriteComponent::AnimSpriteComponent(class Actor *owner, int drawOrder)
    : SpriteComponent(owner, drawOrder), mAnimFps(24), mCurrentFrame(0)
{
}

void AnimSpriteComponent::SetAnimTextures(const std::vector<SDL_Texture *> &textures)
{
	mAnimTextures = textures;
	mCurrentFrame = 0;
	SetTexture(mAnimTextures[0]);
}

void AnimSpriteComponent::Update(float deltaTime)
{
	SpriteComponent::Update(deltaTime);

	if (mAnimTextures.size() > 0)
	{
		mCurrentFrame += mAnimFps * deltaTime;

		// loop
		while (mCurrentFrame >= mAnimTextures.size())
			mCurrentFrame -= mAnimTextures.size();

		SetTexture(mAnimTextures[static_cast<int>(mCurrentFrame)]);
	}
}
