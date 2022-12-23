#pragma once
#include "Component.h"
class SDL_Texture;
class SpriteComponent : public Component
{
	public:
	  SpriteComponent(class Actor *owner, int drawOrder = 100);
	  ~SpriteComponent();
	  void SetTexture(SDL_Texture *texture);
	  virtual void Draw(class Shader &shader);

	  inline int GetDrawOrder() const { return mDrawOrder; }
	  inline int GetTextureWidth() const { return mTexWidth; }
	  inline int GetTextureHeight() const { return mTexHeight; }

	protected:
	  // Texture to draw
	  SDL_Texture *mTexture;
	  // Draw order for painters algo
	  int mDrawOrder;
	  // width and height
	  int mTexWidth;
	  int mTexHeight;
};
