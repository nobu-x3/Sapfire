include "Dependencies.lua"

workspace "Sapfire"
   configurations { "Debug", "Release" }

   language "C++"
   cppdialect "C++20"
   targetdir "bin/%{cfg.buildcfg}"
   toolset "clang"

   group "Core"
   include "sapfire"
   group ""
