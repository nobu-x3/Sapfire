#include "engpch.h"

#include "components/component.h"
namespace Sapfire {
	stl::unordered_map<const char*, components::ComponentType> components::ComponentRegistry::s_ComponentTypes = {};
	stl::unordered_map<components::ComponentType, const char*> components::ComponentRegistry::s_ComponentTypeNameMap = {};
	stl::unordered_map<const char*, stl::shared_ptr<components::IComponentList>> components::ComponentRegistry::s_EngineComponentLists = {};
	stl::unordered_map<const char*, stl::shared_ptr<components::CustomComponentList>>
		components::ComponentRegistry::s_CustomComponentLists{};
	components::ComponentType components::ComponentRegistry::s_NextComponentTypeNumber = 0;
} // namespace Sapfire

#include <crtdbg.h>
#include <exception>
#include "core/application.h"
#include "core/memory.h"
#include "core/stl/unique_ptr.h"
#include "render/d3d_util.h"

extern Sapfire::Application* Sapfire::create_application();

int main(int argc, char* argv[]) {
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	try {
		Sapfire::Log::Init();
		PROFILE_BEGIN_SESSION("Startup", "SapfireProfile_Startup.json");
		Sapfire::Application* application = Sapfire::create_application();
		PROFILE_END_SESSION();

		PROFILE_BEGIN_SESSION("Runtime", "SapfireProfile_Runtime.json");
		application->run();
		PROFILE_END_SESSION();
		PROFILE_BEGIN_SESSION("Shutdown", "SapfireProfile_Shutdown.json");
		delete application;
		PROFILE_END_SESSION();
	} catch (Sapfire::d3d::DxException& e) {
		CORE_CRITICAL(Sapfire::d3d::WStringToANSI(e.to_string()));
		MessageBoxW(nullptr, e.to_string().c_str(), L"HR Failed", MB_OK);
		return 0;
	} catch (std::exception& e) {
		CORE_CRITICAL(e.what());
		return 0;
	}
	Sapfire::mem::memdump();
	return 0;
}
