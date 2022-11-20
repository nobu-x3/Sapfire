#pragma once

#include "SpriteComponent.h"
class AnimSpriteComponent : public SpriteComponent
{
      public:
	AnimSpriteComponent(class Actor *owner, int drawOrder = 100);
	~AnimSpriteComponent() = default;

	void Update(float deltaTime) override;
	void SetAnimTextures(const std::vector<SDL_Texture *> &textures);
	inline float GetAnimFps() const { return mAnimFps; }
	inline void SetAnimFps(float fps) { mAnimFps = fps; }

      private:
	std::vector<SDL_Texture *> mAnimTextures;
	float mCurrentFrame;
	float mAnimFps;
};
