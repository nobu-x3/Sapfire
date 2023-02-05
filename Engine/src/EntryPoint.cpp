#include "engpch.h"
#include "Application.h"

int main(int argc, char *argv[])
{
	Application *application = CreateApplication();
	application->Run();
	delete application;
	return 0;
}
