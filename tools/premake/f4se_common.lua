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

	forceincludes { "$(SolutionDir)f4se-core/common/common/IPrefix.h" }
	
	includedirs {
		"../../f4se-core/f4se",
		"../../f4se-core/common"
	}