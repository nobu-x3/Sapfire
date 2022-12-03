#include "AnimSpriteComponent.h"
#include "SpriteComponent.h"
#include <SDL_log.h>

AnimSpriteComponent::AnimSpriteComponent(class Actor *owner, int drawOrder)
    : SpriteComponent(owner, drawOrder), mAnimFps(24), mCurrentFrame(0), mCurrentAnimIndex(0), mOriginAnimIndex(0)
{
}

void AnimSpriteComponent::AddAnim(AnimData &anim)
{
	mAnimDatas.emplace_back(anim);
	mNameIndexMap.emplace(anim.AnimName, mAnimDatas.size() - 1);
	mCurrentFrame = 0;
	//	SetTexture(mAnimTextures[0]);
}

void AnimSpriteComponent::Update(float deltaTime)
{
	SpriteComponent::Update(deltaTime);

	if (mCurrentAnimIndex >= mAnimDatas.size())
		return;

	PlayAnimation(mCurrentAnimIndex, deltaTime);
}

void AnimSpriteComponent::PlayAnimation(int index, float deltaTime)
{
	if (mAnimDatas[index].AnimTextures.size() > 0)
	{
		auto animData = mAnimDatas[index];

		mCurrentFrame += mAnimFps * deltaTime;

		// loop
		if (animData.Looping)
		{

			while (mCurrentFrame >= animData.AnimTextures.size())
				mCurrentFrame -= animData.AnimTextures.size();
		}
		else
		{
			if (mCurrentFrame >= animData.AnimTextures.size())
				PlayAnimation(mOriginAnimIndex);
		}

		SetTexture(animData.AnimTextures[static_cast<int>(mCurrentFrame)]);
	}
}

void AnimSpriteComponent::PlayAnimation(int index)
{
	if (index > mAnimDatas.size() - 1 || index < 0)
	{
		SDL_Log("Given animation does not exist!");
		return;
	}

	if (mAnimDatas[mCurrentAnimIndex].Looping)
		mOriginAnimIndex = mCurrentAnimIndex;
	mCurrentAnimIndex = index;
	mCurrentFrame = 0.f;
}
void AnimSpriteComponent::PlayAnimation(std::string name)
{
	auto iter = mNameIndexMap.find(name);
	if (iter == mNameIndexMap.end())
	{
		SDL_Log("Animation with name %s does not exist", name.c_str());
		return;
	}

	PlayAnimation(iter->second);
}
