project "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    includedirs
    {
        "%{wks.location}/Engine/deps/spdlog/include",
        "%{wks.location}/Engine/src",
        "%{wks.location}/Engine/deps",
        "%{IncludeDir.glm}",
    }
    links
	{
		"Engine"
	}
    filter "system:windows"
    systemversion "latest"

filter "configurations:Debug"
    runtime "Debug"
    symbols "on"