#include "SpriteComponent.h"
#include "Actor.h"
#include "Component.h"
#include "GL/glew.h"
#include "Game.h"
#include "Math.h"
#include "Shader.h"
#include "Texture.h"
#include <SDL_log.h>

SpriteComponent::SpriteComponent(Actor *owner, int drawOrder)
    : Component(owner), mTexture(nullptr), mDrawOrder(drawOrder), mTexWidth(0), mTexHeight(0)
{
	owner->GetGame()->AddSprite(this);
}

void SpriteComponent::SetTexture(Texture *texture)
{
	mTexture = texture;
	mTexWidth = texture->GetWidth();
	mTexHeight = texture->GetHeight();
}

SpriteComponent::~SpriteComponent()
{
	mOwner->GetGame()->RemoveSprite(this);
}

void SpriteComponent::Draw(Shader &shader)
{
	if (mTexture)
	{
		// scale quad by width & height of texture
		Matrix4 scaleMat = Matrix4::CreateScale(static_cast<float>(mTexWidth), static_cast<float>(mTexHeight),
							1.0f); // TODO: optimize
		Matrix4 world = scaleMat * mOwner->GetWorldTransform();
		shader.SetMatrixUniform("uWorldTransform", world);
		SDL_Log("DRAW:: %f, %f, %f", world.GetTranslation().x, world.GetTranslation().y,
			world.GetTranslation().z);
		mTexture->SetActive();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}
}
