#pragma once

class TestApp
{
	public:
	TestApp();
	~TestApp();

	public:
	void Tick();

	private:
	void OnEvent(class Event &e);
	bool OnWindowClose(class WindowCloseEvent &e);

	private:
	std::unique_ptr<class Window> mWindow;
	bool mRunning;
};
