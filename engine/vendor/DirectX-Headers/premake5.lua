project "DirectX-Headers"
	kind "StaticLib"
	language "C++"
	staticruntime "off"
    targetdir("%{wks.location}/bin/" .. outputdir .. "/vendor/%{prj.name}")
    objdir("%{wks.location}/bin-int/" .. outputdir .. "/vendor/%{prj.name}")
    targetname "DirectX-Headers"
	files
	{
		"include/directx/**.h",
		"src/d3dx12_property_format_table.cpp"
	}

	includedirs
	{
		"include/directx",
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"_CRT_SECURE_NO_WARNINGS",
		}

	filter "configurations:debug"
		symbols "On"

	filter "configurations:release"
		optimize "On"
