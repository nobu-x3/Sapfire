#pragma once
#include "Math.h"
#include "SpriteComponent.h"
#include <vector>
class BGSpriteComponent : public SpriteComponent
{
      public:
	BGSpriteComponent(class Actor *owner, int drawOrder = 10);
	~BGSpriteComponent() = default;
	void Update(float deltaTime) override;
	void Draw(SDL_Renderer *renderer) override;
	void SetBGTextures(const std::vector<SDL_Texture *> &bgTextures);
	inline Vector2 GetScreenSize() const { return mScreenSize; }
	inline void SetScreenSize(Vector2 size) { mScreenSize = size; }
	inline float GetScrollSpeed() const { return mScrollSpeed; }
	inline void SetScrollSpeed(float speed) { mScrollSpeed = speed; }

      private:
	struct BGTexture
	{
		SDL_Texture *mTexture;
		Vector2 mOffset;
	};
	std::vector<BGTexture> mBGTextures;
	Vector2 mScreenSize;
	float mScrollSpeed;
};