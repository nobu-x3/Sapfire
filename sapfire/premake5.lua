
project "Sapfire"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "bin/%{cfg.buildcfg}"
   toolset "clang"

   files { "src/**.h", "src/**.c" }

   defines {"SAPEXPORT"}
   includedirs { "src", "vendor" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
