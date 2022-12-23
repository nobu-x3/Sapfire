#include "SpriteComponent.h"
#include "Actor.h"
#include "Component.h"
#include "GL/glew.h"
#include "Game.h"
#include "Math.h"
#include "SDL2/SDL_image.h"
#include "Shader.h"
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

void SpriteComponent::Draw(Shader &shader)
{
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}
