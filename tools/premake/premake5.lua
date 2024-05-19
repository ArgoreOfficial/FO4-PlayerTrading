
workspace "PlayerTrading"
	configurations { "Debug", "Release" }
	platforms { "x64", "x86" }
	location "../../"
	startproject "PlayerTrading"

	configuration "Debug"
		defines { "ARG_DEBUG" }


include "f4se.lua"
include "f4se_common.lua"
include "common.lua"

include "sandbox_client.lua"
include "PlayerTrading.lua"