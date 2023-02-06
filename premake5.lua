include "dependencies.lua"
workspace "Sapfire"
architecture "x86_64"
startproject "Sandbox"
configurations { "Debug", "Release" }
flags { "MultiProcessorCompile" }
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
include "Engine/deps/GLFW"
include "Engine/deps/Glad"
include "Engine/deps/ImGui"
group ""

project "Sapfire"
    location "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    pchheader "engpch.h"
    pchsource "%{wks.location}/Engine/src/engpch.cpp"

    -- include "%{wks.location}/"
    files
    {
        "Engine/src/**.h",
        "Engine/src/**.cpp",
        "%{wks.location}/Engine/deps/stb_image/**.h",
        "%{wks.location}/Engine/deps/stb_image/**.cpp",
        "%{wks.location}/Engine/deps/glm/glm/**.hpp",
        "%{wks.location}/Engine/deps/glm/glm/**.inl"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS", "GLFW_INCLUDE_NONE"
    }

    includedirs
    {
        "Engine/src",
        "Engine/deps",
        "%{IncludeDir.Assimp}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.imgui}"
    }

    links
    {
        "GLFW",
        "Glad",
        "ImGui",
    }

    filter "system:windows"
        systemversion "latest"
        links {"opengl32.lib",}
        defines
        {
        }
    
    filter "system:linux"
        systemversion "latest"

        defines
        {
            "USE_SDL"
        }

    filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
        defines { "DEBUG" }
        links 
        {
            "%{Libs.Assimp_Debug}"
        }

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
        links 
        {
            "%{Libs.Assimp_Release}"
        }

project "Sandbox"
    kind "ConsoleApp"
    location "Sandbox"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    files
    {
        "Sandbox/src/**.h",
        "Sandbox/src/**.cpp"
    }
    includedirs
    {
        "Sandbox/src",
        "%{IncludeDir.spdlog}",
        "%{wks.location}/Engine/src",
        "%{wks.location}/Engine/deps",
        "%{IncludeDir.glm}",
        "%{IncludeDir.glew}",
        "%{IncludeDir.imgui}"
    }

    links
    {
        "Sapfire"
    }

    filter "system:windows"
    systemversion "latest"

    filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
        links
        {
            "%{Libs.Assimp_Debug}"
        }

        postbuildcommands
        {
            '{COPY} "%{Binaries.Assimp_Debug}" "%{cfg.targetdir}"',
        }
        defines { "DEBUG" }

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
        links
        {
            "%{Libs.Assimp_Release}"
        }

        postbuildcommands
        {
            '{COPY} "%{Binaries.Assimp_Release}" "%{cfg.targetdir}"',
        }