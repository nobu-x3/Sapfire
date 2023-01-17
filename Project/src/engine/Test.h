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

	private:
	std::unique_ptr<class Window> mWindow;
	bool mRunning;
};
