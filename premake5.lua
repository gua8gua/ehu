workspace "ehu"
	architecture "x64"
	
	configurations{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Ehu"
	location "Ehu"
	kind "SharedLib"
	language "C++"
	buildoptions { "/utf-8" }

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs{
		"%{prj.name}/vendor/spdlog/include"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "10.0.26100.0"

		defines{
			"EHU_PLATFORM_WINDOWS",
			"EHU_BUILD_DLL",
		}

		postbuildcommands {
  			"{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/SandBox"
		}

	filter "configurations:Debug"
		defines "EHU_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "EHU_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "EHU_DIST"
		optimize "On"

project "SandBox"
	location "SandBox"
	kind "ConsoleApp"
	language "C++"
	buildoptions { "/utf-8" }

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs{
		"Ehu/vendor/spdlog/include",
		"Ehu/src"
	}

	links{
		"Ehu"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "10.0.26100.0"

		defines{
			"EHU_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "EHU_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "EHU_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "EHU_DIST"
		optimize "On"