#pragma once

class TestApp
{
	public:
	TestApp();
	~TestApp();

	private:
	std::unique_ptr<class Window> mWindow;
};
