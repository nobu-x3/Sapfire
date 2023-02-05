include "dependencies.lua"
workspace "Sapfire"
architecture "x86_64"
startproject "Sandbox"
configurations { "Debug", "Release" }
flags { "MultiProcessorCompile" }
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
group "Core"
    include "Engine"
group ""

group "Misc"
    include "Sandbox"
group ""