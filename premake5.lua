workspace "Engine"
architecture "x86_64"
configurations { "Debug", "Release" }
flags { "MultiProcessorCompile" }

filter { "configurations:Debug" }
symbols "On"

filter { "configurations:Release" }
optimize "On"

filter {}

outputdir = "builds/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
-- IncludeDir["ImGui"] = "Project/libs/imgui"
IncludeDir["glad"] = "Project/libs/glad/include"
IncludeDir["SDL2"] = "Project/libs/SDL/include"
IncludeDir["spdlog"] = "Project/libs/spdlog/include"
IncludeDir["assimp"] = "Project/libs/assimp/include"
IncludeDir["glm"] = "Project/libs/glm/glm"

-- group "Dependencies"
--     include "Project/libs/imgui"
--     include "Project/libs/glad/include"
--     include "Project/libs/SDL/include"
--     include "Project/libs/spdlog/include"
--     include "Project/libs/assimp/include"
--     include "Project/libs/glm/glm"

-- group ""

project "Engine"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    pchheader "engpch.h"
    pchsource "Project/src/engine/engpch.cpp"

    files
    {
        "Project/src/**.h",
        "Project/src/**.cpp"
    }

    includedirs
    {
        "Project/src",
        "%{IncludeDir.SDL2}",
        -- "%{IncludeDir.ImGui}",
        "%{IncludeDir.glad}",
        "%{IncludeDir.spglog}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.glm}"
    }

    links
    {
        "SDL2",
        "Glad",
        "ImGui",
    }

    filter "system:windows"
		systemversion "latest"

		defines
		{
		}
