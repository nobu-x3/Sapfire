#include "SpriteComponent.h"
#include "Actor.h"
#include "Component.h"
#include "GL/glew.h"
#include "Game.h"
#include "Math.h"
#include "engine/Texture.h"
#include "engine/renderer/Renderer.h"
#include "engine/renderer/opengl/OpenGLShader.h"
#include <SDL_render.h>

#include "Actor.h"
#include "Game.h"
#include "SpriteComponent.h"
#include "Texture.h"

SpriteComponent::SpriteComponent(Actor *owner, int drawOrder)
    : Component(owner), mTexture(nullptr), mDrawOrder(drawOrder), mTexWidth(0), mTexHeight(0)
{
	mOwner->GetGame()->GetRenderer()->AddSprite(this);
}

SpriteComponent::~SpriteComponent()
{
	mOwner->GetGame()->GetRenderer()->RemoveSprite(this);
}

void SpriteComponent::Draw(OpenGLShader *shader)
{
	if (mTexture)
	{
		// Scale the quad by the width/height of texture
		Matrix4 scaleMat =
		    Matrix4::CreateScale(static_cast<float>(mTexWidth), static_cast<float>(mTexHeight), 1.0f);

		Matrix4 world = scaleMat * mOwner->GetWorldTransform();

		// Since all sprites use the same shader/vertices,
		// the game first sets them active before any sprite draws

		// Set world transform
		shader->SetMatrixUniform("uWorldTransform", world);
		// Set current texture
		mTexture->SetActive();
		// Draw quad
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}
}

void SpriteComponent::SetTexture(Texture *texture)
{
	mTexture = texture;
	// Set width/height
	mTexWidth = texture->GetWidth();
	mTexHeight = texture->GetHeight();
}
