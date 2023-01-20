#include "MeshComponent.h"
#include "Actor.h"
#include "Game.h"
#include "Mesh.h"
#include "Texture.h"
#include "VertexArray.h"
#include "engine/renderer/Renderer.h"
#include "engine/shader/OpenGLShader.h"

MeshComponent::MeshComponent(Actor *owner) : Component(owner), mMesh(nullptr), mTextureIndex(0)
{
	owner->GetGame()->GetRenderer()->AddMeshComponent(this);
}

MeshComponent::~MeshComponent()
{
	mOwner->GetGame()->GetRenderer()->RemoveMeshComponent(this);
}

void MeshComponent::SetMesh(Mesh *mesh)
{
	mMesh = mesh;
	mOwner->GetGame()->GetRenderer()->LinkShaderToMeshComp(mesh->GetShaderName(), this);
}
void MeshComponent::Draw(OpenGLShader *shader)
{
	if (mMesh)
	{
		shader->SetMatrixUniform("uWorldTransform", mOwner->GetWorldTransform());
		shader->SetFloatUniform("uSpecPower", mMesh->GetSpecPower());
		Texture *t = mMesh->GetTexture(mTextureIndex);
		if (t)
		{
			t->SetActive();
		}
		VertexArray *va = mMesh->GetVertexArray();
		va->SetActive();
		glDrawElements(GL_TRIANGLES, va->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
	}
}
