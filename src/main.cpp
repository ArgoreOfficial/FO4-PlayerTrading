/**
 * Copyright 2024 Argore
 *
 */

// F4SE
#include "f4se/PluginAPI.h"
#include "f4se/PapyrusNativeFunctions.h"

// Common
#include "f4se_common/f4se_version.h"

#include <shlobj.h>	// CSIDL_MYCODUMENTS

#include "TradingTerminal/cTradingTerminal.h"

/* Plugin Query */

extern "C" {
	bool F4SEPlugin_Query( const F4SEInterface* f4se, PluginInfo* info )
	{
		gLog.OpenRelative( CSIDL_MYDOCUMENTS, "\\My Games\\Fallout4\\F4SE\\PlayerTrading.log" );
		gLog.SetPrintLevel( IDebugLog::kLevel_Error );
		gLog.SetLogLevel( IDebugLog::kLevel_DebugMessage );

		// populate info structure
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "PlayerTrading";
		info->version = 1;

		if ( f4se->isEditor )
		{
			_MESSAGE( "loaded in editor, marking as incompatible" );
			return false;
		}
		else if ( f4se->runtimeVersion != RUNTIME_VERSION_1_10_163 )
		{
			_MESSAGE( "unsupported runtime version %d", f4se->runtimeVersion );
			return false;
		}
		// ### do not do anything else in this callback
		// ### only fill out PluginInfo and return true/false
		return true;
	}

	bool F4SEPlugin_Load( const F4SEInterface* f4se )
	{
		if ( !PlayerTrading::trading_terminal.init( f4se ) )
		{
			_MESSAGE( "Failed to Load" );
			return false;
		}

		_MESSAGE( "::F4SEPlugin_Load" );
		_MESSAGE( "  Loaded test-plugin" );

		return true;
	}

};
