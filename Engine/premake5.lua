project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    pchheader "engpch.h"
    pchsource "src/engpch.cpp"

    -- include "%{wks.location}/"
    files
    {
        "src/**.h",
        "src/**.cpp",
        "deps/stb_image/**.h",
        "deps/stb_image/**.cpp",
        "deps/glm/glm/**.hpp",
        "deps/glm/glm/**.inl",
        "%{wks.location}/Engine/deps/SOIL2/src/**.h",
        "%{wks.location}/Engine/deps/SOIL2/src/**.c"
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
        "%{IncludeDir.spglog}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.glm}"
    }

    links
    {
        "SDL2",
        "Glad",
        "ImGui",
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


