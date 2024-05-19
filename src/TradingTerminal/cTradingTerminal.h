#pragma once

#include "f4se/PluginAPI.h"
#include "f4se/PapyrusNativeFunctions.h"
#include "f4se/PapyrusVM.h"
#include <f4se/GameFormComponents.h>

#include <vector>

#include "Client/cClient.h"

#include <mutex>

enum eMessageType
{
	MessageType_NONE = 0,
	MessageType_ConfirmTrade = 1,
	MessageType_TradeData = 2
};

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

namespace PlayerTrading
{
	class cTradingTerminal
	{
	public:

		static inline const char* EXPORT_SCRIPT = "TradingTerminal";

		bool init( const F4SEInterface* f4se );

		bool registerFunctions();
		bool registerEventListener();
		void updateKeyboardState();
		void handleKeyboardEvent( int key, bool down );

		// change to std::vector?
		VMArray<BGSMod::Attachment::Mod*> getOMods( BGSInventoryItem& _item );
		VMArray<BGSMod::Attachment::Mod*> getOMods( BGSInventoryItem::Stack* _stack );

		void prepareInventoryItems();
		void sendItemsInternal();
		void clearContainer();

		static void handleNewPacket( char* _data, int _size );

		// export functions
		static void toggleKeyboardInput( StaticFunctionTag* base ); // remove?
		
		static void copyTradeCode( StaticFunctionTag* _base );
		static bool hasReceivedItems( StaticFunctionTag* _base );
		static VMArray<UInt32> receiveItemsInternal( StaticFunctionTag* _base );
		
		static void connect( StaticFunctionTag* _base );
		static bool checkConnection( StaticFunctionTag* _base );
		static void sendItems( StaticFunctionTag* _base, TESObjectREFR* _container );


		PluginHandle m_handle;

		F4SEMessagingInterface* m_messaging;
		F4SEPapyrusInterface* m_papyrus;
		F4SETaskInterface* m_task;
		F4SEObjectInterface* m_object;

		TESObjectREFR* m_container;
		bool m_marked_for_send = true;

		bool m_process_keyboard_input = false;
		bool m_keyboard_state[ 256 ]; // remove?

		std::mutex m_inventory_mutex;

		std::vector<ItemTrade> m_trade_items;
		std::vector<int> m_inventory_buffer;
		std::vector<UInt32> m_receiving;

		cClient m_client;

	};

	inline cTradingTerminal trading_terminal;
}
