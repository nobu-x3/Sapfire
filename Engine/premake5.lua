project "Engine"
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
        "src/**.h",
        "src/**.cpp",
        "deps/stb_image/**.h",
        "deps/stb_image/**.cpp",
        "deps/glm/glm/**.hpp",
        "deps/glm/glm/**.inl"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS"
    }
    includedirs
    {
        "src",
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


