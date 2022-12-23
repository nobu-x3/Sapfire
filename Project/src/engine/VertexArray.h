#pragma once

class VertexArray
{
	public:
	VertexArray(const float *verts, unsigned int numVerts, const unsigned int *indices, unsigned int numIndices);
	~VertexArray();

	void SetActivate();
	inline unsigned int GetNumIndices() const { return mNumIndices; }
	inline unsigned int GetNumVerts() const { return mNumVerts; }

	private:
	unsigned int mNumVerts;	    // # of vertices in the vertex buffer
	unsigned int mNumIndices;   // # of infices in the index buffer
	unsigned int mVertexBuffer; // id of the vertex buffer
	unsigned int mIndexBuffer;  // id of the index buffer
	unsigned int mVertexArray;  // id of the vertex array object
};
