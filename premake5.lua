-- premake5.lua
workspace "schiffe_versenken"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "schiffe_versenken"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["glm"] = "vendor/glm"
IncludeDir["spd_log"] = "vendor/spdlog/include"
IncludeDir["SFML"] = "vendor/SFML/SFML-3.0.0/include"
IncludeDir["ImGui"] = "vendor/imgui"




group "Dependencies"
	include "Vendor/imgui"
group ""

include "schiffe_versenken"

