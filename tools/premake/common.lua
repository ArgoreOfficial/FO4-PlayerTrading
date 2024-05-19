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
	
	forceincludes { "$(SolutionDir)f4se-core/common/common/IPrefix.h" }

	includedirs {
		"../../f4se-core/common/"
	}