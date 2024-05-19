project "sandbox_client"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	
	targetdir "../../bin"
	objdir "../../bin/obj/"

	location "../../build"

	files { 
		"../../sandbox-src/**.h", 
		"../../sandbox-src/**.cpp",

		"../../src/Client/**.h",
		"../../src/Client/**.cpp"
	}
	
	includedirs {
		"../../src"
	}
	