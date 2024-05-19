scriptname TradingTerminal native hidden

function enableKeyboardInput() native global

function disableKeyboardInput() native global

function toggleKeyboardInput() native global

function ListInventoryItems( ObjectReference _ref ) native global

; internal, do not use
int[] function ReceiveItemsInternal() native global

function ReceiveItems( ObjectReference _container ) global
    int[] items = ReceiveItemsInternal()

    int i = 0
    ; item format:
    ; int count, int formID, int[] omods
    while i < items.Length
        int count = items[ i ]
        int mod_counter = 0
        i += 1

        while count > 0
            ObjectReference item_ref = Game.GetPlayer().PlaceAtMe( Game.GetForm( items[ i ] ) )
            
            mod_counter = i + 1
            while items[ mod_counter ] != 0
                item_ref.AttachMod( game.GetForm( items[ mod_counter ] ) as ObjectMod )
                mod_counter += 1
            endwhile
            count -= 1

            _container.AddItem( item_ref )
        endwhile
        
        i = mod_counter + 1
    endwhile
    
endfunction