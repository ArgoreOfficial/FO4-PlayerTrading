#include "cTradingTerminal.h"

#include "f4se/PapyrusDelayFunctors.h"

#include "f4se/GameReferences.h"
#include "f4se/PapyrusEvents.h"
#include "f4se/PapyrusObjectReference.h"
#include "f4se/GameExtraData.h"
#include "f4se/GameRTTI.h"

#include "clipboardxx.hpp"

bool PlayerTrading::cTradingTerminal::init( const F4SEInterface* f4se )
{
#ifdef ARG_DEBUG
	FILE* tmp = nullptr;
	AllocConsole();
	freopen_s( &tmp, "CONOUT$", "w", stdout );
#endif

	{
		m_handle    = f4se->GetPluginHandle();
		m_messaging = (F4SEMessagingInterface*)f4se->QueryInterface( kInterface_Messaging );
		m_papyrus   = (F4SEPapyrusInterface*)  f4se->QueryInterface( kInterface_Papyrus );
		m_task      = (F4SETaskInterface*)     f4se->QueryInterface( kInterface_Task );
		m_object    = (F4SEObjectInterface*)   f4se->QueryInterface( kInterface_Object );
	}

	if ( !registerFunctions() )     return false;
	if ( !registerEventListener() ) return false;

	union
	{
		char ipv4[ 4 ] = { 127, 0, 0, 1 };
		int v;
		float f;
	} test;

	m_client.create( 9000 );

	return true;
}

bool PlayerTrading::cTradingTerminal::registerFunctions()
{
	// register functions
	auto registerFuncs = []( VirtualMachine* vm )
		{
			vm->RegisterFunction( new NativeFunction0( "ToggleKeyboardInput",  EXPORT_SCRIPT, toggleKeyboardInput, vm ) );
			vm->RegisterFunction( new NativeFunction0( "Connect",              EXPORT_SCRIPT, connect, vm ) );
			vm->RegisterFunction( new NativeFunction0( "CheckConnection",      EXPORT_SCRIPT, checkConnection, vm ) );
			vm->RegisterFunction( new NativeFunction1( "ListInventoryItems",   EXPORT_SCRIPT, listInventoryItems, vm ) );
			vm->RegisterFunction( new NativeFunction0( "ReceiveItemsInternal", EXPORT_SCRIPT, receiveItemsInternal, vm ) );
			vm->RegisterFunction( new NativeFunction0( "CopyTradeCode",        EXPORT_SCRIPT, copyTradeCode, vm ) );
			vm->RegisterFunction( new NativeFunction0( "HasReceivedItems",     EXPORT_SCRIPT, hasReceivedItems, vm ) );
			return true;
		};

	if ( !m_papyrus || !m_papyrus->Register( registerFuncs ) )
		return false;

	return true;
}

bool PlayerTrading::cTradingTerminal::registerEventListener()
{
	// register listener
	auto listener = []( F4SEMessagingInterface::Message* msg )
		{
			switch ( msg->type )
			{
			case F4SEMessagingInterface::kMessage_GameDataReady: break;
			case F4SEMessagingInterface::kMessage_PostLoadGame:
			{
				struct OnTick : public IF4SEDelayFunctor
				{
					OnTick() { }

					const char* ClassName() const override { return EXPORT_SCRIPT; }
					UInt32 ClassVersion() const override { return 1; }
					bool Save( const F4SESerializationInterface* intfc ) override { return true; }
					bool Load( const F4SESerializationInterface* intfc, UInt32 version ) override { return true; }
					bool Run( VMValue& resultOut ) override
					{
						if ( !trading_terminal.m_process_keyboard_input )
							return true;

						trading_terminal.updateKeyboardState();

						return true;
					}
					bool ShouldReschedule( SInt32& delayMSOut ) override { delayMSOut = 1; return true; }
					bool ShouldResumeStack( UInt32& stackIdOut ) override { return false; }
				};

				trading_terminal.m_object->GetDelayFunctorManager().Enqueue( new OnTick() );
			} break;
			}
		};

	if ( !m_messaging || !m_messaging->RegisterListener( m_handle, "F4SE", listener ) )
		return false;

	return true;
}

void PlayerTrading::cTradingTerminal::updateKeyboardState()
{
	for ( int i = 0; i < 256; i++ )
	{
		bool keystate = ( ( GetKeyState( i ) & 0x8000 ) > 0 );
		if ( keystate != m_keyboard_state[ i ] )
			handleKeyboardEvent( i, keystate );

		m_keyboard_state[ i ] = keystate;
	}
}

void PlayerTrading::cTradingTerminal::handleKeyboardEvent( int key, bool down )
{
	if ( !down )
		return;

	switch ( key )
	{
	case VK_TAB:    m_process_keyboard_input = false; break;
	case VK_RETURN: printf( "\n" ); break;
	case VK_SPACE:  printf( " " ); break;
	}

	if ( key >= 'A' && key <= 'Z' )
		printf( "%c", key );

	if ( key >= 0x30 && key <= 0x39 ) // 0-9 number keys
		printf( "%i", key - 0x30 );
}

VMArray<BGSMod::Attachment::Mod*> PlayerTrading::cTradingTerminal::getOMods( BGSInventoryItem& _item )
{
	return getOMods( _item.stack );
}

VMArray<BGSMod::Attachment::Mod*> PlayerTrading::cTradingTerminal::getOMods( BGSInventoryItem::Stack* _stack )
{
	ExtraDataList* extraDataList = _stack->extraData;

	VMArray<BGSMod::Attachment::Mod*> omods;
	if ( !extraDataList ) return omods;

	BSExtraData* extraData = extraDataList->GetByType( ExtraDataType::kExtraData_ObjectInstance );
	if ( !extraData ) return omods;

	BGSObjectInstanceExtra* objectModData = DYNAMIC_CAST( extraData, BSExtraData, BGSObjectInstanceExtra );
	if ( !objectModData ) return omods;

	auto data = objectModData->data;
	if ( !data || !data->forms ) return omods;

	for ( UInt32 i = 0; i < data->blockSize / sizeof( BGSObjectInstanceExtra::Data::Form ); i++ )
	{
		BGSMod::Attachment::Mod* objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast( LookupFormByID( data->forms[ i ].formId ), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod );
		omods.Push( &objectMod );
	}

	return omods;
}

void PlayerTrading::cTradingTerminal::toggleKeyboardInput( StaticFunctionTag* base )
{
	trading_terminal.m_process_keyboard_input = !trading_terminal.m_process_keyboard_input;

	printf( "Toggled Keyboard Input\n" );
	// BSFixedString n = "Toggled";
	// CallGlobalFunctionNoWait1<BSFixedString>( "Debug", "Notification", n );
}

void PlayerTrading::cTradingTerminal::listInventoryItems( StaticFunctionTag* base, TESObjectREFR* refr )
{
	if ( !refr )
		return;

	// does refr->GetFullName() work on containers?
#ifdef ARG_DEBUG
	printf( "  ---------- %s ---------- \n", refr->GetFullName() );
#endif
	BGSInventoryList* inventory = refr->inventoryList;
	if ( !inventory )
		return;

	inventory->inventoryLock.LockForRead();
	for ( int i = 0; i < inventory->items.count; i++ )
	{
		BGSInventoryItem& item = inventory->items[ i ];
		BGSInventoryItem::Stack* itemstack = item.stack;
		do
		{
		#ifdef ARG_DEBUG
			printf( "item: %s(%i) [%02x]\n", item.form->GetFullName(), item.stack->count, item.form->formID );
		#endif
			trading_terminal.m_sending.push_back( item.stack->count );
			trading_terminal.m_sending.push_back( item.form->formID );


			// get extra data, if any
			/*
			for ( UInt32 i = 1; i < 0xD6; i++ )
			{
				if ( !itemstack->extraData )
					continue;
				if ( !itemstack->extraData->HasType( i ) )
					continue;

			#ifdef ARG_DEBUG
				// printf( "  extra: %s\n", getExtraDataType( i ) ); // stringify kExtraData enum
			#endif
			}
			*/

			// get object mods, if any
			VMArray<BGSMod::Attachment::Mod*> omods = trading_terminal.getOMods( itemstack ); // identical to PapyrusObjectReference.cpp -> papyrusObjectReference::GetAllMods
			for ( int i = 0; i < omods.Length(); i++ )
			{
				BGSMod::Attachment::Mod* omod;
				omods.Get( &omod, i );
			#ifdef ARG_DEBUG
				printf( "  mod: %s [%02x]\n", omod->fullName.name.c_str(), omod->formID );
			#endif
				trading_terminal.m_sending.push_back( omod->formID );
			}

			itemstack = itemstack->next;
			trading_terminal.m_sending.push_back( 0 );
		} while ( itemstack );
	}

	inventory->inventoryLock.UnlockRead();

	sendItems( base );
}

void PlayerTrading::cTradingTerminal::sendItems( StaticFunctionTag* base )
{
	// send items
	trading_terminal.m_sending.insert( trading_terminal.m_sending.begin(), ePacketType::PacketType_Send );

	for ( int i = 0; i < trading_terminal.m_sending.size(); i++ )
		printf( "[%02x]", trading_terminal.m_sending[ i ] );

	trading_terminal.m_client.sendDataToConnected( trading_terminal.m_sending.data(), trading_terminal.m_sending.size() * sizeof( int ) );
}

void PlayerTrading::cTradingTerminal::copyTradeCode( StaticFunctionTag* _base )
{
	clipboardxx::clipboard clipboard;
	uAddressCode code = trading_terminal.m_client.getExternalIP();
	clipboard.copy( std::to_string( code.c ) );
}

bool PlayerTrading::cTradingTerminal::hasReceivedItems( StaticFunctionTag* _base )
{
	return false;
}

VMArray<UInt32> PlayerTrading::cTradingTerminal::receiveItemsInternal( StaticFunctionTag* base )
{
	std::vector<UInt32> vec =
	{
		1,  0x9983b,  0x12eced, 0x997c9, 0x99836, 0x1877fa, 0x4f21d, 0,
		10, 0x0a,                                                    0,
		1,  0x18796c, 0x46d8e,  0x46d97, 0x18e59c,                   0
	};

	return vec;
}

void PlayerTrading::cTradingTerminal::connect( StaticFunctionTag* base )
{
	clipboardxx::clipboard clipboard;
	std::string code_str = clipboard.paste();
	
	try
	{
		uAddressCode code_union;
		code_union.c = std::stoi( code_str );
		trading_terminal.m_client.connect( code_union.dump().c_str(), 8000 );
	}
	catch ( ... ) { }

	Sleep( 10 );
}

bool PlayerTrading::cTradingTerminal::checkConnection( StaticFunctionTag* base )
{
	bool connected = trading_terminal.m_client.m_connected_addr != 0;

	BSFixedString n = "Failed to Connect";
	if ( connected )
		n = "Connected";

	CallGlobalFunctionNoWait1<BSFixedString>( "Debug", "Notification", n );

	return connected;
}

void PlayerTrading::cTradingTerminal::confirmTrade( StaticFunctionTag* _base, TESObjectREFR* _container )
{
	cClient& client = trading_terminal.m_client;

	if ( client.m_received_data )
	{
		std::vector<int> data = client.getReceivedData();
		if ( data[ 0 ] == PacketType_ConfirmTrade )

			return;
	}

	int buffer[ 1 ];
	buffer[ 0 ] = PacketType_ConfirmTrade;
	trading_terminal.m_client.sendDataToConnected( buffer, sizeof( buffer ) );
}
