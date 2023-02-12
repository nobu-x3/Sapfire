#include "engpch.h"
#include "Sapfire/core/Application.h"

int main(int argc, char* argv[])
{
	PROFILE_BEGIN_SESSION("Startup", "SapfireProfile_Startup.json");
	Sapfire::Application* application = Sapfire::create_application();
	PROFILE_END_SESSION();

	PROFILE_BEGIN_SESSION("Runtime", "SapfireProfile_Runtime.json");
	application->run();
	PROFILE_END_SESSION();

	PROFILE_BEGIN_SESSION("Shutdown", "SapfireProfile_Shutdown.json");
	delete application;
	PROFILE_END_SESSION();

	return 0;
}
