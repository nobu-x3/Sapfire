#pragma once

#include "Component.h"
class MeshComponent : public Component
{
	public:
	MeshComponent(class Actor *owner);
	~MeshComponent();
	virtual void Draw(class OpenGLShader *shader);
	virtual void SetMesh(class Mesh *mesh);
	inline void SetTextureIndex(size_t index) { mTextureIndex = index; }

	private:
	class Mesh *mMesh;
	size_t mTextureIndex;
};
