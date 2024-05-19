/**
 * Copyright 2024 Argore
 *
 */

#pragma once

#include "f4se/PluginAPI.h"
#include "f4se/PapyrusNativeFunctions.h"
#include "f4se/PapyrusVM.h"
#include <f4se/GameFormComponents.h>

#include <vector>

#include "Client/cClient.h"

namespace TradingTerminal
{
	struct ItemTrade
	{
		int count;
		int form_id;
		std::vector<int> omods;

		void compress( std::vector<int>& _out )
		{
			_out.push_back( count );
			_out.push_back( form_id );
			for ( int& omod : omods )
				_out.push_back( omod );
		}
	};

	static struct
	{
		PluginHandle handle;

		F4SEMessagingInterface* messaging;
		F4SEPapyrusInterface* papyrus;
		F4SETaskInterface* task;
		F4SEObjectInterface* object;

		bool process_keyboard_input = false;

		bool keyboard_state[ 256 ];

		std::vector<ItemTrade> trade_items;
		std::vector<int> sending;
		std::vector<int> receiving;

		cClient client;
	} state;

	static inline const char* EXPORT_SCRIPT = "TradingTerminal";
	
	bool init( const F4SEInterface* f4se );

	bool registerFunctions();
	bool registerEventListener();
	
	void updateKeyboardState();
	void handleKeyboardEvent( int key, bool down );

	// change to std::vector?
	VMArray<BGSMod::Attachment::Mod*> getOMods( BGSInventoryItem& _item );
	VMArray<BGSMod::Attachment::Mod*> getOMods( BGSInventoryItem::Stack* _stack );

	
	// export functions
	
	static void enableKeyboardInput ( StaticFunctionTag* base ) { state.process_keyboard_input = true; }
	static void disableKeyboardInput( StaticFunctionTag* base ) { state.process_keyboard_input = false; }
	void toggleKeyboardInput( StaticFunctionTag* base );

	void listInventoryItems( StaticFunctionTag* base, TESObjectREFR* refr );
	
	void sendItems( StaticFunctionTag* base );
	VMArray<UInt32> receiveItemsInternal( StaticFunctionTag* base );

	void connect();
};