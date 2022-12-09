#pragma once

#include "SpriteComponent.h"
#include <string>
#include <unordered_map>
#include <vector>
struct AnimData
{
	std::string AnimName;
	std::vector<SDL_Texture *> AnimTextures;
	bool Looping;

	AnimData() : AnimName(""), Looping(false) {}

	AnimData(std::string name, std::vector<SDL_Texture *> texts, bool looping)
	    : AnimName(name), AnimTextures(texts), Looping(looping)
	{
	}
};
class AnimSpriteComponent : public SpriteComponent
{
      public:
	AnimSpriteComponent(class Actor *owner, int drawOrder = 100);
	~AnimSpriteComponent() = default;

	void Update(float deltaTime) override;
	void AddAnim(AnimData &data);
	void PlayAnimation(std::string name);
	void PlayAnimation(int index);
	inline float GetAnimFps() const { return mAnimFps; }
	inline void SetAnimFps(float fps) { mAnimFps = fps; }
	inline void AddAnimation(AnimData animationData)
	{
		mAnimDatas.emplace_back(animationData);
		mNameIndexMap.emplace(animationData.AnimName, mAnimDatas.size() - 1);
	}

      private:
	void PlayAnimation(int index, float deltaTime);

	std::vector<AnimData> mAnimDatas;
	std::unordered_map<std::string, int> mNameIndexMap;
	int mCurrentAnimIndex;
	int mOriginAnimIndex;
	float mCurrentFrame;
	float mAnimFps;
	int direction;
};
