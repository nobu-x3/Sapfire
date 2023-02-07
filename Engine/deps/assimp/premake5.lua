workspace "assimp"
architecture "x86_64"
configurations { "Debug", "Release" }
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
project "assimp"
    kind "SharedLib"
    language "C++"
    staticruntime "off"
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    files { "include/*"}

    filter "configurations:Debug"
            runtime "Debug"
            symbols "on"

    filter "configurations:Release"
            runtime "Release"
            optimize "on"