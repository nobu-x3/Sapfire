workspace "Sapfire"
   configurations { "Debug", "Release" }

project "Sapfire"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   targetdir "bin/%{cfg.buildcfg}"
   toolset "clang"

   files { "**.h", "**.c" }

   defines {"SAPEXPORT"}
   includedirs { "src" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
