include "./vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "ValorCore"
	architecture "x86_64"
	startproject "ValorCLI"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	solution_items
	{
		".editorconfig"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Core"
	include "Valor"
group ""

group "Tools"
	include "ValorCLI"
	include "MagicBitboardGenerator"
group ""
