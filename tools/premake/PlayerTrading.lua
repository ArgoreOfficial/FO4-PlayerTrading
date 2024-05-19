project "PlayerTrading"
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

    forceincludes { "$(SolutionDir)f4se-core/common/common/IPrefix.h" }

	includedirs {
		"../../src",
		"../../f4se-core",
		"../../f4se-core/f4se",
		"../../f4se-core/common",
		"../../include"
	}

	links { 
		"f4se" 
	}