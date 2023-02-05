#pragma once

#include "Core.h"

class Texture
{
	public:
	virtual ~Texture() = default;
	virtual void Bind() = 0;
	virtual int GetWidth() const = 0;
	virtual int GetHeight() const = 0;
	virtual RendererID GetID() const = 0;

	static Ref<Texture> Create(const std::string &path);
};
