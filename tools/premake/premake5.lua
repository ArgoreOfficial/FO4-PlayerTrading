PROJECT_NAME = "PlayerTrading"

workspace (PROJECT_NAME)
	configurations { "Debug", "Release" }
	platforms { "x64", "x86" }
	location "../../"
	startproject (PROJECT_NAME)

	forceincludes { "$(SolutionDir)f4se-core/common/common/IPrefix.h" }

	configuration "Debug"
		defines { "ARG_DEBUG" }

project "f4se"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	
	targetdir "../../bin"
	objdir "../../bin/obj/"

	location "../../build"

	characterset "MBCS"

	files { 
		"../../f4se-core/f4se/f4se/**.h",
		"../../f4se-core/f4se/f4se/**.cpp",
		"../../f4se-core/f4se/f4se/exports.def",
	}

	includedirs {
		"../../f4se-core",
		"../../f4se-core/f4se",
		"../../f4se-core/common"
	}
    
	links { 
		"common", 
		"f4se_common"
	}

	defines {
		"WIN32",
		"_WINDOWS",
		"F4SE_EXPORTS",
		"RUNTIME",
		"RUNTIME_VERSION=0x010A0A30"
	}

project "f4se_common"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	
	targetdir "../../bin"
	objdir "../../bin/obj/"

	location "../../build"

	characterset "MBCS"

	files { 
		"../../f4se-core/f4se/f4se_common/**.h",
		"../../f4se-core/f4se/f4se_common/**.cpp",
	}

	includedirs {
		"../../f4se-core/f4se",
		"../../f4se-core/common"
	}

project "common"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	
	targetdir "../../bin"
	objdir "../../bin/obj/"

	location "../../build"

	characterset "MBCS"

	files { 
		"../../f4se-core/common/**.h",
		"../../f4se-core/common/**.cpp",
	}
	
	includedirs {
		"../../f4se-core/common/"
	}


project (PROJECT_NAME)
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	
	targetdir "../../bin"
	objdir "../../bin/obj/"

	postbuildcommands { 'copy "$(TargetPath)" "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Fallout 4\\Data\\F4SE\\Plugins\\$(TargetFileName)"' }

	location "../../build"

	characterset "MBCS"

	files { 
		"../../src/**.h", 
		"../../src/**.cpp",
		"../../src/exports.def"
	}

	includedirs {
		"../../src",
		"../../f4se-core",
		"../../f4se-core/f4se",
		"../../f4se-core/common"
	}

	links { 
		"f4se" 
	}
