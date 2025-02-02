project "MagicBitboardGenerator"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	linkoptions { "/ignore:4099,4006" }

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"src"
	}

	filter "system:Windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "VL_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "VL_RELEASE"
		runtime "Release"
		optimize "on"
