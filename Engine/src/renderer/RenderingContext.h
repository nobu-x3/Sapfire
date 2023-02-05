#pragma once
class RenderingContext
{
	public:
	virtual ~RenderingContext() {}
	virtual void Init() = 0;
	virtual void SwapBuffers() = 0;
	virtual void Shutdown() = 0;
};
