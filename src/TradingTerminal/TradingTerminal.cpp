/**
 * Copyright 2024 Argore
 *
 */

#include "TradingTerminal.h"

#include "f4se/PapyrusDelayFunctors.h"

#include "f4se/GameReferences.h"
#include "f4se/PapyrusEvents.h"
#include "f4se/PapyrusObjectReference.h"
#include "f4se/GameExtraData.h"
#include "f4se/GameRTTI.h"

////////////////////////////////////////////////////////////////////////////////

bool TradingTerminal::init( const F4SEInterface* f4se )
{
#ifdef ARG_DEBUG
	FILE* tmp = nullptr;
	AllocConsole();
	freopen_s( &tmp, "CONOUT$", "w", stdout );
#endif

	{
		state.handle = f4se->GetPluginHandle();
		state.messaging = (F4SEMessagingInterface*)f4se->QueryInterface( kInterface_Messaging );
		state.papyrus   = (F4SEPapyrusInterface*)  f4se->QueryInterface( kInterface_Papyrus );
		state.task      = (F4SETaskInterface*)     f4se->QueryInterface( kInterface_Task );
		state.object    = (F4SEObjectInterface*)   f4se->QueryInterface( kInterface_Object );
	}

	if ( !registerFunctions() )     return false;
	if ( !registerEventListener() ) return false;
	
	union
	{
		char ipv4[ 4 ] = { 127, 0, 0, 1 };
		int v;
		float f;
	} test;

	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool TradingTerminal::registerFunctions()
{
	// register functions
	auto registerFuncs = []( VirtualMachine* vm )
		{
			vm->RegisterFunction( new NativeFunction0( "EnableKeyboardInput",  EXPORT_SCRIPT, enableKeyboardInput,  vm ) );
			vm->RegisterFunction( new NativeFunction0( "DisableKeyboardInput", EXPORT_SCRIPT, disableKeyboardInput, vm ) );
			vm->RegisterFunction( new NativeFunction0( "ToggleKeyboardInput",  EXPORT_SCRIPT, toggleKeyboardInput,  vm ) );
			vm->RegisterFunction( new NativeFunction1( "ListInventoryItems",   EXPORT_SCRIPT, listInventoryItems,   vm ) );
			vm->RegisterFunction( new NativeFunction0( "ReceiveItemsInternal", EXPORT_SCRIPT, receiveItemsInternal, vm ) );
			return true;
		};

	if ( !state.papyrus || !state.papyrus->Register( registerFuncs ) )
		return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool TradingTerminal::registerEventListener()
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
					if ( !state.process_keyboard_input )
						return true;
					
					updateKeyboardState();

					return true;
				}
				bool ShouldReschedule( SInt32& delayMSOut ) override { delayMSOut = 1; return true; }
				bool ShouldResumeStack( UInt32& stackIdOut ) override { return false; }
				};

				state.object->GetDelayFunctorManager().Enqueue( new OnTick() );
			} break;
			}
		};

	if ( !state.messaging || !state.messaging->RegisterListener( state.handle, "F4SE", listener ) )
		return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////

void TradingTerminal::updateKeyboardState()
{
	for ( int i = 0; i < 256; i++ )
	{
		bool keystate = ( ( GetKeyState( i ) & 0x8000 ) > 0 );
		if ( keystate != state.keyboard_state[ i ] )
			handleKeyboardEvent( i, keystate );

		state.keyboard_state[ i ] = keystate;
	}
}

////////////////////////////////////////////////////////////////////////////////

void TradingTerminal::handleKeyboardEvent( int key, bool down )
{
	if ( !down )
		return;

	switch ( key )
	{
	case VK_TAB:    state.process_keyboard_input = false; break;
	case VK_RETURN: printf( "\n" ); break;
	case VK_SPACE:  printf( " " ); break;
	}

	if ( key >= 'A' && key <= 'Z' )
		printf( "%c", key );

	if ( key >= 0x30 && key <= 0x39 ) // 0-9 number keys
		printf( "%i", key - 0x30 );
}

////////////////////////////////////////////////////////////////////////////////

void TradingTerminal::toggleKeyboardInput( StaticFunctionTag* base )
{
	state.process_keyboard_input = !state.process_keyboard_input;

	printf( "Toggled Keyboard Input\n" );
	// BSFixedString n = "Toggled";
	// CallGlobalFunctionNoWait1<BSFixedString>( "Debug", "Notification", n );
}

////////////////////////////////////////////////////////////////////////////////

VMArray<BGSMod::Attachment::Mod*> TradingTerminal::getOMods( BGSInventoryItem& _item )
{
	return getOMods( _item.stack );
}

////////////////////////////////////////////////////////////////////////////////

VMArray<BGSMod::Attachment::Mod*> TradingTerminal::getOMods( BGSInventoryItem::Stack* _stack )
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

////////////////////////////////////////////////////////////////////////////////

const char* getExtraDataType( int _v )
{
	switch ( _v )
	{

		case kExtraData_Havok:                            return "Havok";                            break;
		case kExtraData_Cell3D:                           return "Cell3D";                           break;
		case kExtraData_CellWaterType:                    return "CellWaterType";                    break;
		case kExtraData_RegionList:                       return "RegionList";                       break;
		case kExtraData_SeenData:                         return "SeenData";                         break;
		case kExtraData_EditorID:                         return "EditorID";                         break;
		case kExtraData_CellMusicType:                    return "CellMusicType";                    break;
		case kExtraData_CellSkyRegion:                    return "CellSkyRegion";                    break;
		case kExtraData_ProcessMiddleLow:                 return "ProcessMiddleLow";                 break;
		case kExtraData_DetachTime:                       return "DetachTime";                       break;
		case kExtraData_PersistentCell:                   return "PersistentCell";                   break;
		case kExtraData_Keywords:                         return "Keywords";                         break;
		case kExtraData_Action:                           return "Action";                           break;
		case kExtraData_StartingPosition:                 return "StartingPosition";                 break;
		case kExtraData_AnimGraphManager:                 return "AnimGraphManager";                 break;
		case kExtraData_Biped:                            return "Biped";                            break;
		case kExtraData_UsedMarkers:                      return "UsedMarkers";                      break;
		case kExtraData_DistantData:                      return "DistantData";                      break;
		case kExtraData_RagDollData:                      return "RagDollData";                      break;
		case kExtraData_PreVisRefs:                       return "PreVisRefs";                       break;
		case kExtraData_InitActions:                      return "InitActions";                      break;
		case kExtraData_EssentialProtected:               return "EssentialProtected";               break;
		case kExtraData_PackageStartLocation:             return "PackageStartLocation";             break;
		case kExtraData_Package:                          return "Package";                          break;
		case kExtraData_TresPassPackage:                  return "TresPassPackage";                  break;
		case kExtraData_RunOncePacks:                     return "RunOncePacks";                     break;
		case kExtraData_ReferenceHandle:                  return "ReferenceHandle";                  break;
		case kExtraData_Follower:                         return "Follower";                         break;
		case kExtraData_LevCreaModifier:                  return "LevCreaModifier";                  break;
		case kExtraData_Ghost:                            return "Ghost";                            break;
		case kExtraData_OriginalReference:                return "OriginalReference";                break;
		case kExtraData_Ownership:                        return "Ownership";                        break;
		case kExtraData_Global:                           return "Global";                           break;
		case kExtraData_Rank:                             return "Rank";                             break;
		case kExtraData_Count:                            return "Count";                            break;
		case kExtraData_Health:                           return "Health";                           break;
		case kExtraData_RangedDistOverride:               return "RangedDistOverride";               break;
		case kExtraData_TimeLeft:                         return "TimeLeft";                         break;
		case kExtraData_Charge:                           return "Charge";                           break;
		case kExtraData_Light:                            return "Light";                            break;
		case kExtraData_Lock:                             return "Lock";                             break;
		case kExtraData_Teleport:                         return "Teleport";                         break;
		case kExtraData_MapMarker:                        return "MapMarker";                        break;
		case kExtraData_LeveledCreature:                  return "LeveledCreature";                  break;
		case kExtraData_LeveledItem:                      return "LeveledItem";                      break;
		case kExtraData_Scale:                            return "Scale";                            break;
		case kExtraData_Seed:                             return "Seed";                             break;
		case kExtraData_MagicCaster:                      return "MagicCaster";                      break;
		case kExtraData_MasterFileCell:                   return "MasterFileCell";                   break;
		case kExtraData_PlayerCrimeList:                  return "PlayerCrimeList";                  break;
		case kExtraData_ObjectInstance:                   return "ObjectInstance";                   break;
		case kExtraData_EnableStateParent:                return "EnableStateParent";                break;
		case kExtraData_EnableStateChildren:              return "EnableStateChildren";              break;
		case kExtraData_ItemDropper:                      return "ItemDropper";                      break;
		case kExtraData_DroppedItemList:                  return "DroppedItemList";                  break;
		case kExtraData_RandomTeleportMarker:             return "RandomTeleportMarker";             break;
		case kExtraData_SavedHavokData:                   return "SavedHavokData";                   break;
		case kExtraData_CannotWear:                       return "CannotWear";                       break;
		case kExtraData_Poison:                           return "Poison";                           break;
		case kExtraData_LastFinishedSequence:             return "LastFinishedSequence";             break;
		case kExtraData_SavedAnimation:                   return "SavedAnimation";                   break;
		case kExtraData_NorthRotation:                    return "NorthRotation";                    break;
		case kExtraData_SpawnContainer:                   return "SpawnContainer";                   break;
		case kExtraData_FriendHits:                       return "FriendHits";                       break;
		case kExtraData_HeadingTarget:                    return "HeadingTarget";                    break;
		case kExtraData_RefractionProperty:               return "RefractionProperty";               break;
		case kExtraData_StartingWorldOrCell:              return "StartingWorldOrCell";              break;
		case kExtraData_Hotkey:                           return "Hotkey";                           break;
		case kExtraData_EditorRef3DData:                  return "EditorRef3DData";                  break;
		case kExtraData_EditiorRefMoveData:               return "EditiorRefMoveData";               break;
		case kExtraData_InfoGeneralTopic:                 return "InfoGeneralTopic";                 break;
		case kExtraData_HasNoRumors:                      return "HasNoRumors";                      break;
		case kExtraData_Sound:                            return "Sound";                            break;
		case kExtraData_TerminalState:                    return "TerminalState";                    break;
		case kExtraData_LinkedRef:                        return "LinkedRef";                        break;
		case kExtraData_LinkedRefChildren:                return "LinkedRefChildren";                break;
		case kExtraData_ActivateRef:                      return "ActivateRef";                      break;
		case kExtraData_ActivateRefChildren:              return "ActivateRefChildren";              break;
		case kExtraData_CanTalkToPlayer:                  return "CanTalkToPlayer";                  break;
		case kExtraData_ObjectHealth:                     return "ObjectHealth";                     break;
		case kExtraData_CellImageSpace:                   return "CellImageSpace";                   break;
		case kExtraData_NavMeshPortal:                    return "NavMeshPortal";                    break;
		case kExtraData_ModelSwap:                        return "ModelSwap";                        break;
		case kExtraData_Radius:                           return "Radius";                           break;
		case kExtraData_FactionChanges:                   return "FactionChanges";                   break;
		case kExtraData_DismemberedLimbs:                 return "DismemberedLimbs";                 break;
		case kExtraData_ActorCause:                       return "ActorCause";                       break;
		case kExtraData_MultiBound:                       return "MultiBound";                       break;
		case kExtraData_MultiBoundData:                   return "MultiBoundData";                   break;
		case kExtraData_MultiBoundRef:                    return "MultiBoundRef";                    break;
		case kExtraData_ReflectedRefs:                    return "ReflectedRefs";                    break;
		case kExtraData_ReflectorRefs:                    return "ReflectorRefs";                    break;
		case kExtraData_EmittanceSource:                  return "EmittanceSource";                  break;
		case kExtraData_RadioData:                        return "RadioData";                        break;
		case kExtraData_CombatStyle:                      return "CombatStyle";                      break;
		case kExtraData_Primitive:                        return "Primitive";                        break;
		case kExtraData_OpenCloseActivateRef:             return "OpenCloseActivateRef";             break;
		case kExtraData_AnimNoteReceiver:                 return "AnimNoteReceiver";                 break;
		case kExtraData_Ammo:                             return "Ammo";                             break;
		case kExtraData_PatrolRefData:                    return "PatrolRefData";                    break;
		case kExtraData_PackageData:                      return "PackageData";                      break;
		case kExtraData_OcclusionShape:                   return "OcclusionShape";                   break;
		case kExtraData_CollisionData:                    return "CollisionData";                    break;
		case kExtraData_SayTopicInfoOnceADay:             return "SayTopicInfoOnceADay";             break;
		case kExtraData_EncounterZone:                    return "EncounterZone";                    break;
		case kExtraData_SayTopicInfo:                     return "SayTopicInfo";                     break;
		case kExtraData_OcclusionPlaneRefData:            return "OcclusionPlaneRefData";            break;
		case kExtraData_PortalRefData:                    return "PortalRefData";                    break;
		case kExtraData_Portal:                           return "Portal";                           break;
		case kExtraData_Room:                             return "Room";                             break;
		case kExtraData_HealthPerc:                       return "HealthPerc";                       break;
		case kExtraData_RoomRefData:                      return "RoomRefData";                      break;
		case kExtraData_GuardedRefData:                   return "GuardedRefData";                   break;
		case kExtraData_CreatureAwakeSound:               return "CreatureAwakeSound";               break;
		case kExtraData_Horse:                            return "Horse";                            break;
		case kExtraData_IgnoredBySandbox:                 return "IgnoredBySandbox";                 break;
		case kExtraData_CellAcousticSpace:                return "CellAcousticSpace";                break;
		case kExtraData_ReservedMarkers:                  return "ReservedMarkers";                  break;
		case kExtraData_TransitionCellCount:              return "TransitionCellCount";              break;
		case kExtraData_WaterLightRefs:                   return "WaterLightRefs";                   break;
		case kExtraData_LitWaterRefs:                     return "LitWaterRefs";                     break;
		case kExtraData_RadioRepeater:                    return "RadioRepeater";                    break;
		case kExtraData_ActivateLoopSound:                return "ActivateLoopSound";                break;
		case kExtraData_PatrolRefInUseData:               return "PatrolRefInUseData";               break;
		case kExtraData_AshPileRef:                       return "AshPileRef";                       break;
		case kExtraData_CreatureMovementSound:            return "CreatureMovementSound";            break;
		case kExtraData_FollowerSwimBreadcrumbs:          return "FollowerSwimBreadcrumbs";          break;
		case kExtraData_AliasInstanceArray:               return "AliasInstanceArray";               break;
		case kExtraData_Location:                         return "Location";                         break;
		case kExtraData_MasterLocation:                   return "MasterLocation";                   break;
		case kExtraData_LocationRefType:                  return "LocationRefType";                  break;
		case kExtraData_PromotedRef:                      return "PromotedRef";                      break;
		case kExtraData_AnimationSequencer:               return "AnimationSequencer";               break;
		case kExtraData_OutfitItem:                       return "OutfitItem";                       break;
		case kExtraData_LeveledItemBase:                  return "LeveledItemBase";                  break;
		case kExtraData_LightData:                        return "LightData";                        break;
		case kExtraData_SceneData:                        return "SceneData";                        break;
		case kExtraData_BadPosition:                      return "BadPosition";                      break;
		case kExtraData_HeadTrackingWeight:               return "HeadTrackingWeight";               break;
		case kExtraData_FromAlias:                        return "FromAlias";                        break;
		case kExtraData_ShouldWear:                       return "ShouldWear";                       break;
		case kExtraData_FavorCost:                        return "FavorCost";                        break;
		case kExtraData_AttachedArrows3D:                 return "AttachedArrows3D";                 break;
		case kExtraData_TextDisplayData:                  return "TextDisplayData";                  break;
		case kExtraData_AlphaCutoff:                      return "AlphaCutoff";                      break;
		case kExtraData_Enchantment:                      return "Enchantment";                      break;
		case kExtraData_Soul:                             return "Soul";                             break;
		case kExtraData_ForcedTarget:                     return "ForcedTarget";                     break;
		case kExtraData_SoundOutputOverride:              return "SoundOutputOverride";              break;
		case kExtraData_UniqueID:                         return "UniqueID";                         break;
		case kExtraData_Flags:                            return "Flags";                            break;
		case kExtraData_RefrPath:                         return "RefrPath";                         break;
		case kExtraData_DecalGroup:                       return "DecalGroup";                       break;
		case kExtraData_LockList:                         return "LockList";                         break;
		case kExtraData_ForcedLandingMarker:              return "ForcedLandingMarker";              break;
		case kExtraData_LargeRefOwnerCells:               return "LargeRefOwnerCells";               break;
		case kExtraData_CellWaterEnvMap:                  return "CellWaterEnvMap";                  break;
		case kExtraData_CellGrassData:                    return "CellGrassData";                    break;
		case kExtraData_TeleportName:                     return "TeleportName";                     break;
		case kExtraData_Interaction:                      return "Interaction";                      break;
		case kExtraData_WaterData:                        return "WaterData";                        break;
		case kExtraData_WaterCurrentZoneData:             return "WaterCurrentZoneData";             break;
		case kExtraData_AttachRef:                        return "AttachRef";                        break;
		case kExtraData_AttachRefChildren:                return "AttachRefChildren";                break;
		case kExtraData_GroupConstraint:                  return "GroupConstraint";                  break;
		case kExtraData_ScriptedAnimDependence:           return "ScriptedAnimDependence";           break;
		case kExtraData_CachedScale:                      return "CachedScale";                      break;
		case kExtraData_RaceData:                         return "RaceData";                         break;
		case kExtraData_GIDBuffer:                        return "GIDBuffer";                        break;
		case kExtraData_MissingRefIDs:                    return "MissingRefIDs";                    break;
		case kExtraData_BendableSplineParams:             return "BendableSplineParams";             break;
		case kExtraData_MaterialSwap:                     return "MaterialSwap";                     break;
		case kExtraData_InstanceData:                     return "InstanceData";                     break;
		case kExtraData_PowerArmor:                       return "PowerArmor";                       break;
		case kExtraData_InputEnableLayer:                 return "InputEnableLayer";                 break;
		case kExtraData_ProjectedDecalData:               return "ProjectedDecalData";               break;
		case kExtraData_WorkshopExtraData:                return "WorkshopExtraData";                break;
		case kExtraData_RadioReceiver:                    return "RadioReceiver";                    break;
		case kExtraData_CulledBone:                       return "CulledBone";                       break;
		case kExtraData_ActorValueStorage:                return "ActorValueStorage";                break;
		case kExtraData_DirectAtTarget:                   return "DirectAtTarget";                   break;
		case kExtraData_ActivateText:                     return "ActivateText";                     break;
		case kExtraData_CombinedRefs:                     return "CombinedRefs";                     break;
		case kExtraData_ObjectBreakable:                  return "ObjectBreakable";                  break;
		case kExtraData_SavedDynamicIdles:                return "SavedDynamicIdles";                break;
		case kExtraData_IgnoredAttractKeywords:           return "IgnoredAttractKeywords";           break;
		case kExtraData_ModRank:                          return "ModRank";                          break;
		case kExtraData_InteriorLODWorldspace:            return "InteriorLODWorldspace";            break;
		case kExtraData_BoneScaleMap:                     return "BoneScaleMap";                     break;
		case kExtraData_FXPickNodes:                      return "FXPickNodes";                      break;
		case kExtraData_PowerArmorPreload:                return "PowerArmorPreload";                break;
		case kExtraData_AnimGraphPreload:                 return "AnimGraphPreload";                 break;
		case kExtraData_AnimSounds:                       return "AnimSounds";                       break;
		case kExtraData_PowerLinks:                       return "PowerLinks";                       break;
		case kExtraData_ObjectSavedUnrecoverableSubgraph: return "ObjectSavedUnrecoverableSubgraph"; break;
		case kExtraData_RefWeaponSounds:                  return "RefWeaponSounds";                  break;
		case kExtraData_InvestedGold:                     return "InvestedGold";                     break;
		case kExtraData_FurnitureEntryData:               return "FurnitureEntryData";               break;
		case kExtraData_VoiceType:                        return "VoiceType";                        break;
	}
	return "err";
}

void TradingTerminal::listInventoryItems( StaticFunctionTag* base, TESObjectREFR* refr )
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
		
			// get extra data, if any
			for ( UInt32 i = 1; i < 0xD6; i++ )
			{
				if ( !itemstack->extraData )
					continue;
				if ( !itemstack->extraData->HasType( i ) )
					continue;
				
			#ifdef ARG_DEBUG
				printf( "  extra: %s\n", getExtraDataType( i ) ); // stringify kExtraData enum
			#endif
			}

			// get object mods, if any
			VMArray<BGSMod::Attachment::Mod*> omods = getOMods( itemstack ); // identical to PapyrusObjectReference.cpp -> papyrusObjectReference::GetAllMods
			for ( int i = 0; i < omods.Length(); i++ )
			{
				BGSMod::Attachment::Mod* omod;
				omods.Get( &omod, i );
			#ifdef ARG_DEBUG
				printf( "  mod: %s [%02x]\n", omod->fullName.name.c_str(), omod->formID);
			#endif
			}

			itemstack = itemstack->next;
		} while ( itemstack );
	}

	inventory->inventoryLock.UnlockRead();
}

////////////////////////////////////////////////////////////////////////////////

void TradingTerminal::sendItems( StaticFunctionTag* base )
{
	
}

////////////////////////////////////////////////////////////////////////////////

VMArray<UInt32> TradingTerminal::receiveItemsInternal( StaticFunctionTag* base )
{
	std::vector<UInt32> vec =
	{
		1,  0x9983b,  0x12eced, 0x997c9, 0x99836, 0x1877fa, 0x4f21d, 0,
		10, 0x0a,                                                    0,
		1,  0x18796c, 0x46d8e,  0x46d97, 0x18e59c,                   0
	};

	return vec;
}

////////////////////////////////////////////////////////////////////////////////

void TradingTerminal::connect()
{

}

////////////////////////////////////////////////////////////////////////////////