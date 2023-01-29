#include "MeshComponent.h"
#include "Actor.h"
#include "GL/gl.h"
#include "Game.h"
#include "Mesh.h"
#include "engine/renderer/Buffer.h"
#include "engine/renderer/Renderer.h"
#include "engine/renderer/Shader.h"
#include "engine/renderer/Texture.h"

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
void MeshComponent::Draw(Shader *shader)
{
	if (mMesh)
	{
		shader->SetMatrixUniform("uWorldTransform", mOwner->GetWorldTransform());
		shader->SetFloatUniform("uSpecPower", mMesh->GetSpecPower());
		Ref<Texture> t = mMesh->GetTexture(mTextureIndex);
		if (t)
		{
			t->Bind();
		}
		VertexBuffer *vb = mMesh->GetVertexBuffer();
		vb->Bind();

		glDrawElements(GL_TRIANGLES, mMesh->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}
}
