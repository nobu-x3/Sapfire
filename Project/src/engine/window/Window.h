#pragma once
#include <string>
struct WindowProperties {
  std::string Title;
  unsigned int Width, Height;
  WindowProperties(const std::string &title, unsigned int width,
                   unsigned int height)
      : Title(title), Width(width), Height(height) {}
};

class Window {
public:
  virtual ~Window() {}
  virtual void OnBegin() = 0;
  virtual void OnEnd() = 0;
  virtual void OnSubmit() = 0;
  virtual unsigned int GetWidth() = 0;
  virtual unsigned int GetHeight() = 0;
  virtual void *GetNativeWindow() = 0;
  static Window *Create(const WindowProperties &props);
};
