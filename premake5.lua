include "dependencies.lua"
workspace "Sapfire"
architecture "x86_64"
startproject "Sandbox"
configurations { "Debug", "Release" }
flags { "MultiProcessorCompile" }
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
project "Sapfire"
    location "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    -- pchheader "engpch.h"
    -- pchsource "src/engpch.cpp"

    -- include "%{wks.location}/"
    files
    {
        "Engine/src/**.h",
        "Engine/src/**.cpp",
        "Engine/deps/stb_image/**.h",
        "Engine/deps/stb_image/**.cpp",
        "Engine/deps/glm/glm/**.hpp",
        "Engine/deps/glm/glm/**.inl"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS"
    }
    includedirs
    {
        "Engine/src",
        "%{IncludeDir.SDL2}",
        "%{IncludeDir.glew}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.SOIL2}"
    }

    links
    {
        "%{Libs.glew}",
        "%{Libs.SDL2}",
        "%{Libs.spdlog}",
        "%{Libs.assimp}",
        "%{Libs.SOIL2}",
        "winmm",
        "imm32",
        "version",
        "setupapi",
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
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
        "%{wks.location}/Engine/deps/spdlog/include",
        "%{wks.location}/Engine/src",
        "%{wks.location}/Engine/deps",
        "%{IncludeDir.glm}",
        "%{IncludeDir.glew}"
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