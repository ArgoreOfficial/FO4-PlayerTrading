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

	forceincludes { "$(SolutionDir)f4se-core/common/common/IPrefix.h" }

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