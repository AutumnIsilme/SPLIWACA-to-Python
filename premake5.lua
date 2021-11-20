workspace "SPLIWACA"
	architecture "x64"
	
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["spdlog"] = "Spliwaca/vendor/spdlog/include"
IncludeDir["timing"] = "Spliwaca/vendor/timing"

startproject "Spliwaca"

--group "Dependencies"
--	include "Supernova/vendor/GLFW"
--	include "Supernova/vendor/Glad"
--	include "Supernova/vendor/imgui"

group ""

project "Spliwaca"
	location "Spliwaca"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	--pchheader "splwpch.h"
	--pchsource "Spliwaca/src/splwpch.cpp"
	
	files
	{
		"Spliwaca/Spliwaca/src/**.h",
		"Spliwaca/Spliwaca/src/**.cpp",
		"Spliwaca/vendor/timing/**.h",
		"Spliwaca/vendor/spdlog/include/**.h"
	}
	
	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}
	
	includedirs
	{
		"Spliwaca/Spliwaca/src",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.timing}"
	}
	
--[[	links
	{
		"GLFW",
		"GLAD",
		"imgui",
		"opengl32.lib"
	}
	
	filter "system:windows"
		systemversion "latest"
		
		defines
		{
			"PLATFORM_WINDOWS",
			"SN_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}
		
		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\"")
		}]]

	filter "system:windows"
		systemversion "latest"
		defines {
			"SPLW_WINDOWS"
		}
		
		
	filter "configurations:Debug"
		defines "SPLW_DEBUG"
		runtime "Debug"
		symbols "on"
		
	filter "configurations:Release"
		defines "SPLW_RELEASE"
		runtime "Release"
		optimize "on"
	
	filter "configurations:Dist"
		defines "SPLW_DIST"
		runtime "Release"
		optimize "on"
		
--[[
project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}
	
	includedirs
	{
		"Supernova/src",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glm}",
		"Supernova/vendor"
	}
	
	links
	{
		"Supernova"
	}
	
	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"
		
		defines
		{
			"PLATFORM_WINDOWS",
		}
		
	filter "configurations:Debug"
		defines "DEBUG"
		symbols "on"
		
	filter "configurations:Release"
		defines "RELEASE"
		optimize "on"
	
	filter "configurations:Dist"
		defines "DIST"
		optimize "on"
]]