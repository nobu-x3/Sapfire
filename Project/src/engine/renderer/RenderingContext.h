#pragma once
class RenderingContext {
public:
  virtual ~RenderingContext() {}
  virtual void Init() = 0;
  virtual void OnBegin() = 0;
  virtual void OnEnd() = 0;
  virtual void SwapWindow() = 0;
};
