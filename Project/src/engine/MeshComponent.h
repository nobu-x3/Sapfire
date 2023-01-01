#pragma once

#include "Component.h"
class MeshComponent : public Component
{
	public:
	MeshComponent(class Actor *owner);
	~MeshComponent();
	virtual void Draw(class Shader *shader);
	inline virtual void SetMesh(class Mesh *mesh) { mMesh = mesh; }
	inline class Mesh *GetMesh() { return mMesh; }
	inline void SetTextureIndex(size_t index) { mTextureIndex = index; }

	private:
	class Mesh *mMesh;
	size_t mTextureIndex;
};
