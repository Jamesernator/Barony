/*-------------------------------------------------------------------------------

	BARONY
	File: playerinventory.cpp
	Desc: contains player inventory related functions

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "../main.hpp"
#include "../draw.hpp"
#include "../game.hpp"
#include "../stat.hpp"
#include "../items.hpp"
#include "../shops.hpp"
#include "../sound.hpp"
#include "../net.hpp"
#include "../magic/magic.hpp"
#include "../menu.hpp"
#include "../player.hpp"
#include "interface.hpp"
#ifdef STEAMWORKS
#include <steam/steam_api.h>
#include "../steam.hpp"
#endif

//Prototype helper functions for player inventory helper functions.
void itemContextMenu();


SDL_Surface* inventory_mode_item_img = NULL;
SDL_Surface* inventory_mode_item_highlighted_img = NULL;
SDL_Surface* inventory_mode_spell_img = NULL;
SDL_Surface* inventory_mode_spell_highlighted_img = NULL;
int inventory_mode = INVENTORY_MODE_ITEM;

selectBehavior_t itemSelectBehavior = BEHAVIOR_MOUSE;

void warpMouseToSelectedInventorySlot()
{
	SDL_WarpMouseInWindow(screen, INVENTORY_STARTX + (selected_inventory_slot_x * INVENTORY_SLOTSIZE) + (INVENTORY_SLOTSIZE / 2), INVENTORY_STARTY + (selected_inventory_slot_y * INVENTORY_SLOTSIZE) + (INVENTORY_SLOTSIZE / 2));
}

/*-------------------------------------------------------------------------------

	itemUseString

	Returns a string with the verb cooresponding to the item which is
	to be used

-------------------------------------------------------------------------------*/

char* itemUseString(const Item* item)
{
	if ( itemCategory(item) == WEAPON )
	{
		if ( itemIsEquipped(item, clientnum) )
		{
			return language[323];
		}
		else
		{
			return language[324];
		}
	}
	else if ( itemCategory(item) == ARMOR )
	{
		switch ( item->type )
		{
			case WOODEN_SHIELD:
			case BRONZE_SHIELD:
			case IRON_SHIELD:
			case STEEL_SHIELD:
			case STEEL_SHIELD_RESISTANCE:
			case CRYSTAL_SHIELD:
			case MIRROR_SHIELD:
			case ABYSSAL_SHIELD:
			case NECRO_SHIELD:
				if ( itemIsEquipped(item, clientnum) )
				{
					return language[325];
				}
				else
				{
					return language[326];
				}
			default:
				break;
		}
		if ( itemIsEquipped(item, clientnum) )
		{
			return language[327];
		}
		else
		{
			return language[328];
		}
	}
	else if ( itemCategory(item) == AMULET )
	{
		if ( itemIsEquipped(item, clientnum) )
		{
			return language[327];
		}
		else
		{
			return language[328];
		}
	}
	else if ( itemCategory(item) == POTION )
	{
		return language[329];
	}
	else if ( itemCategory(item) == SCROLL )
	{
		return language[330];
	}
	else if ( itemCategory(item) == MAGICSTAFF )
	{
		if ( itemIsEquipped(item, clientnum) )
		{
			return language[323];
		}
		else
		{
			return language[324];
		}
	}
	else if ( itemCategory(item) == RING )
	{
		if ( itemIsEquipped(item, clientnum) )
		{
			return language[327];
		}
		else
		{
			return language[331];
		}
	}
	else if ( itemCategory(item) == SPELLBOOK )
	{
		return language[330];
	}
	else if ( itemCategory(item) == GEM )
	{
		if ( itemIsEquipped(item, clientnum) )
		{
			return language[323];
		}
		else
		{
			return language[324];
		}
	}
	else if ( itemCategory(item) == THROWN )
	{
		if ( itemIsEquipped(item, clientnum) )
		{
			return language[323];
		}
		else
		{
			return language[324];
		}
	}
	else if ( itemCategory(item) == TOOL )
	{
		switch ( item->type )
		{
			case TOOL_PICKAXE:
				if ( itemIsEquipped(item, clientnum) )
				{
					return language[323];
				}
				else
				{
					return language[324];
				}
			case TOOL_TINOPENER:
				return language[1881];
			case TOOL_MIRROR:
				return language[332];
			case TOOL_LOCKPICK:
			case TOOL_SKELETONKEY:
				if ( itemIsEquipped(item, clientnum) )
				{
					return language[333];
				}
				else
				{
					return language[334];
				}
			case TOOL_TORCH:
			case TOOL_LANTERN:
			case TOOL_CRYSTALSHARD:
			case TOOL_GREENTORCH:
			case TOOL_CANDLE:
			case TOOL_CANDLE_TIMELESS:
				if ( itemIsEquipped(item, clientnum) )
				{
					return language[335];
				}
				else
				{
					return language[336];
				}
			case TOOL_BLINDFOLD:
				if ( itemIsEquipped(item, clientnum) )
				{
					return language[327];
				}
				else
				{
					return language[328];
				}
			case TOOL_TOWEL:
			case TOOL_UNIHORN:
				return language[332];
			case TOOL_GLASSES:
				if ( itemIsEquipped(item, clientnum) )
				{
					return language[327];
				}
				else
				{
					return language[331];
				}
			case TOOL_BEARTRAP:
				return language[337];
			case TOOL_ALEMBIC:
				return language[3339];
			case TOOL_METAL_SCRAP:
			case TOOL_MAGIC_SCRAP:
				return language[1881];
				break;
			default:
				break;
		}
	}
	else if ( itemCategory(item) == FOOD )
	{
		return language[338];
	}
	else if ( itemCategory(item) == BOOK )
	{
		return language[330];
	}
	else if ( itemCategory(item) == SPELL_CAT )
	{
		return language[339];
	}
	return language[332];
}

/*-------------------------------------------------------------------------------

	updateAppraisalItemBox

	draws the current item being appraised

-------------------------------------------------------------------------------*/

void updateAppraisalItemBox()
{
	SDL_Rect pos;
	Item* item;
	int x, y;

	x = INVENTORY_STARTX;
	y = INVENTORY_STARTY;

	// appraisal item box
	if ( (item = uidToItem(appraisal_item)) != NULL && appraisal_timer > 0 )
	{
		if ( !shootmode )
		{
			pos.x = x + 16;
			pos.y = y + INVENTORY_SIZEY * INVENTORY_SLOTSIZE + 16;
		}
		else
		{
			pos.x = 16;
			pos.y = 16;
		}
		int w1, w2;
		TTF_SizeUTF8(ttf12, language[340], &w1, NULL);
		TTF_SizeUTF8(ttf12, item->getName(), &w2, NULL);
		w2 += 48;
		pos.w = std::max(w1, w2) + 8;
		pos.h = 68;
		drawTooltip(&pos);

		char tempstr[64] = { 0 };
		snprintf(tempstr, 63, language[341], (((double)(appraisal_timermax - appraisal_timer)) / ((double)appraisal_timermax)) * 100);
		ttfPrintText( ttf12, pos.x + 8, pos.y + 8, tempstr );
		if ( !shootmode )
		{
			pos.x = x + 24;
			pos.y = y + INVENTORY_SIZEY * INVENTORY_SLOTSIZE + 16 + 24;
		}
		else
		{
			pos.x = 24;
			pos.y = 16 + 24;
		}
		ttfPrintText( ttf12, pos.x + 40, pos.y + 8, item->getName() );
		pos.w = 32;
		pos.h = 32;
		drawImageScaled(itemSprite(item), NULL, &pos);
	}
}

void select_inventory_slot(int x, int y)
{
	if ( x < 0 )   //Wrap around left boundary.
	{
		x = INVENTORY_SIZEX - 1;
	}
	if ( x >= INVENTORY_SIZEX )   //Wrap around right boundary.
	{
		x = 0;
	}

	bool warpInv = true;

	if ( y < 0 )   //Wrap around top to bottom.
	{
		y = INVENTORY_SIZEY - 1;
		if ( hotbarGamepadControlEnabled() )
		{
			hotbarHasFocus = true; //Warp to hotbar.
			float percentage = static_cast<float>(x + 1) / static_cast<float>(INVENTORY_SIZEX);
			selectHotbarSlot((percentage + 0.09) * NUM_HOTBAR_SLOTS - 1);
			warpMouseToSelectedHotbarSlot();
		}
	}
	if ( y >= INVENTORY_SIZEY )   //Hit bottom. Wrap around or go to shop/chest?
	{
		if ( openedChest[clientnum] )
		{
			//Do not want to wrap around if opened chest or shop.
			warpInv = false;
			y = INVENTORY_SIZEY - 1; //Keeps the selected slot within the inventory, to warp back to later.

			if ( numItemsInChest() > 0 )   //If chest even has an item...
			{
				//Then warp cursor to chest.
				selectedChestSlot = 0; //Warp to first chest slot.
				int warpX = CHEST_INVENTORY_X + (inventoryoptionChest_bmp->w / 2);
				int warpY = CHEST_INVENTORY_Y + (inventoryoptionChest_bmp->h / 2)  + 16;
				SDL_WarpMouseInWindow(screen, warpX, warpY);
			}
		}
		else if ( gui_mode == GUI_MODE_SHOP )
		{
			warpInv = false;
			y = INVENTORY_SIZEY - 1; //Keeps the selected slot within the inventory, to warp back to later.

			//Warp into shop inventory if shopkeep has any items.
			if ( shopinvitems[0] )
			{
				selectedShopSlot = 0;
				warpMouseToSelectedShopSlot();
			}
		}
		else if ( identifygui_active )
		{
			warpInv = false;
			y = INVENTORY_SIZEY - 1;

			//Warp into identify GUI "inventory"...if there is anything there.
			if ( identify_items[0] )
			{
				selectedIdentifySlot = 0;
				warpMouseToSelectedIdentifySlot();
			}
		}
		else if ( removecursegui_active )
		{
			warpInv = false;
			y = INVENTORY_SIZEY - 1;

			//Warp into Remove Curse GUI "inventory"...if there is anything there.
			if ( removecurse_items[0] )
			{
				selectedRemoveCurseSlot = 0;
				warpMouseToSelectedRemoveCurseSlot();
			}
		}
		else if ( GenericGUI.isGUIOpen() )
		{
			warpInv = false;
			y = INVENTORY_SIZEY - 1;

			//Warp into GUI "inventory"...if there is anything there.
			if ( GenericGUI.itemsDisplayed[0] )
			{
				GenericGUI.selectedSlot = 0;
				GenericGUI.warpMouseToSelectedSlot();
			}
		}

		if ( warpInv )   //Wrap around to top.
		{
			y = 0;

			if ( hotbarGamepadControlEnabled() )
			{
				hotbarHasFocus = true;
				float percentage = static_cast<float>(x + 1) / static_cast<float>(INVENTORY_SIZEX);
				selectHotbarSlot((percentage + 0.09) * NUM_HOTBAR_SLOTS - 1);
				warpMouseToSelectedHotbarSlot();
			}
		}
	}

	selected_inventory_slot_x = x;
	selected_inventory_slot_y = y;
}

/*-------------------------------------------------------------------------------

	updatePlayerInventory

	Draws and processes everything related to the player's inventory window

-------------------------------------------------------------------------------*/

Item* selectedItem = nullptr;
int selectedItemFromHotbar = -1;
bool toggleclick = false;

bool itemMenuOpen = false;
int itemMenuX = 0;
int itemMenuY = 0;
int itemMenuSelected = 0;
Uint32 itemMenuItem = 0;

/*void releaseItem(int x, int y) {
	node_t* node = nullptr;
	node_t* nextnode = nullptr;

	/*
	 * So, here is what must happen:
	 * * If mouse behavior mode, toggle drop!
	 * * If gamepad behavior mode, toggle release if x key pressed.
	 * * * However, keep in mind that you want a mouse click to trigger drop, just in case potato. You know, controller dying or summat. Don't wanna jam game.
	 */
/*

	//Determine if should drop.
	bool dropCondition = false;
	//Check mouse behavior first.
	if (itemSelectBehavior == BEHAVIOR_MOUSE) {
		if ( (!mousestatus[SDL_BUTTON_LEFT] && !toggleclick) || (mousestatus[SDL_BUTTON_LEFT] && toggleclick)) {
			//Releasing mouse button drops the item.
			dropCondition = true;
		}
	} else if (itemSelectBehavior == BEHAVIOR_GAMEPAD) {
		if (*inputPressed(joyimpulses[INJOY_MENU_LEFT_CLICK]))
		{
			//Pressing the item pick up button ("x" by default) again will drop the item.
			dropCondition = true;
		}
	}

	if (dropCondition)
	{
	}
}*/

void releaseItem(int x, int y) //TODO: This function uses toggleclick. Conflict with inventory context menu?
{
	if ( !selectedItem )
	{
		return;
	}

	node_t* node = nullptr;
	node_t* nextnode = nullptr;

	if ( *inputPressed(joyimpulses[INJOY_MENU_CANCEL]))
	{
		if (selectedItemFromHotbar >= -1 && selectedItemFromHotbar < NUM_HOTBAR_SLOTS)
		{
			//Warp cursor back into hotbar, for gamepad convenience.
			SDL_WarpMouseInWindow(screen, (HOTBAR_START_X) + (selectedItemFromHotbar * hotbar_img->w) + (hotbar_img->w / 2), (STATUS_Y) - (hotbar_img->h / 2));
			hotbar[selectedItemFromHotbar].item = selectedItem->uid;
		}
		else
		{
			//Warp cursor back into inventory, for gamepad convenience.
			SDL_WarpMouseInWindow(screen, INVENTORY_STARTX + (selectedItem->x * INVENTORY_SLOTSIZE) + (INVENTORY_SLOTSIZE / 2), INVENTORY_STARTY + (selectedItem->y * INVENTORY_SLOTSIZE) + (INVENTORY_SLOTSIZE / 2));
		}

		selectedItem = nullptr;
		*inputPressed(joyimpulses[INJOY_MENU_CANCEL]) = 0;
		return;
	}

	//TODO: Do proper refactoring.
	if ( selectedItem && itemCategory(selectedItem) == SPELL_CAT && selectedItem->appearance >= 1000 )
	{
		if ( canUseShapeshiftSpellInCurrentForm(*selectedItem) == 0 )
		{
			selectedItem = nullptr;
			return;
		}
	}

	// releasing items
	if ( (!mousestatus[SDL_BUTTON_LEFT] && !toggleclick) || (mousestatus[SDL_BUTTON_LEFT] && toggleclick) || ( (*inputPressed(joyimpulses[INJOY_MENU_LEFT_CLICK])) && toggleclick) )
	{
		*inputPressed(joyimpulses[INJOY_MENU_LEFT_CLICK]) = 0;
		if (openedChest[clientnum] && itemCategory(selectedItem) != SPELL_CAT)
		{
			if (mousex >= CHEST_INVENTORY_X && mousey >= CHEST_INVENTORY_Y
			        && mousex < CHEST_INVENTORY_X + inventoryChest_bmp->w
			        && mousey < CHEST_INVENTORY_Y + inventoryChest_bmp->h)
			{
				if (selectedItem->count > 1)
				{
					openedChest[clientnum]->addItemToChestFromInventory(
					    clientnum, selectedItem, false);
					toggleclick = true;
				}
				else
				{
					openedChest[clientnum]->addItemToChestFromInventory(
					    clientnum, selectedItem, false);
					selectedItem = NULL;
					toggleclick = false;
				}
			}
		}

		if (selectedItem)
		{
			if (mousex >= x && mousey >= y
			        && mousex < x + INVENTORY_SIZEX * INVENTORY_SLOTSIZE
			        && mousey < y + INVENTORY_SIZEY * INVENTORY_SLOTSIZE)
			{
				// within inventory
				int oldx = selectedItem->x;
				int oldy = selectedItem->y;
				selectedItem->x = (mousex - x) / INVENTORY_SLOTSIZE;
				selectedItem->y = (mousey - y) / INVENTORY_SLOTSIZE;
				for (node = stats[clientnum]->inventory.first; node != NULL;
				        node = nextnode)
				{
					nextnode = node->next;
					Item* tempItem = (Item*) (node->element);
					if (tempItem == selectedItem)
					{
						continue;
					}

					toggleclick = false;
					if (tempItem->x == selectedItem->x
					        && tempItem->y == selectedItem->y)
					{
						if (itemCategory(selectedItem) != SPELL_CAT
						        && itemCategory(tempItem) == SPELL_CAT)
						{
							//It's alright, the item can go here. The item sharing this x is just a spell, but the item being moved isn't a spell.
						}
						else if (itemCategory(selectedItem) == SPELL_CAT
						         && itemCategory(tempItem) != SPELL_CAT)
						{
							//It's alright, the item can go here. The item sharing this x isn't a spell, but the item being moved is a spell.
						}
						else
						{
							//The player just dropped an item onto another item.
							tempItem->x = oldx;
							tempItem->y = oldy;
							selectedItem = tempItem;
							toggleclick = true;
							break;
						}
					}
				}
				if (!toggleclick)
				{
					selectedItem = NULL;
				}

				playSound(139, 64); // click sound
			}
			else if (itemCategory(selectedItem) == SPELL_CAT)
			{
				//Outside inventory. Spells can't be dropped.
				hotbar_slot_t* slot = getHotbar(mousex, mousey);
				if (slot)
				{
					//Add spell to hotbar.
					Item* tempItem = uidToItem(slot->item);
					if (tempItem)
					{
						slot->item = selectedItem->uid;
						selectedItem = tempItem;
						toggleclick = true;
					}
					else
					{
						slot->item = selectedItem->uid;
						selectedItem = NULL;
						toggleclick = false;
					}
					playSound(139, 64); // click sound
				}
				else
				{
					selectedItem = NULL;
				}
			}
			else
			{
				// outside inventory
				hotbar_slot_t* slot = getHotbar(mousex, mousey);
				if (slot)
				{
					//Add item to hotbar.
					Item* tempItem = uidToItem(slot->item);
					if (tempItem)
					{
						slot->item = selectedItem->uid;
						selectedItem = tempItem;
						toggleclick = true;
					}
					else
					{
						slot->item = selectedItem->uid;
						selectedItem = NULL;
						toggleclick = false;
					}
					playSound(139, 64); // click sound
				}
				else
				{
					if (selectedItem->count > 1)
					{
						if ( dropItem(selectedItem, clientnum) )
						{
							selectedItem = NULL;
						}
						toggleclick = true;
					}
					else
					{
						dropItem(selectedItem, clientnum);
						selectedItem = NULL;
						toggleclick = false;
					}
				}
			}
		}
		if (mousestatus[SDL_BUTTON_LEFT])
		{
			mousestatus[SDL_BUTTON_LEFT] = 0;
		}
	}
}

void cycleInventoryTab()
{
	if ( inventory_mode == INVENTORY_MODE_ITEM)
	{
		inventory_mode = INVENTORY_MODE_SPELL;
	}
	else
	{
		//inventory_mode == INVENTORY_MODE_SPELL
		inventory_mode = INVENTORY_MODE_ITEM;
	}
}

/*
 * Because the mouseInBounds() function looks at the omousex for whatever reason.
 * And changing that function to use mousex has, through much empirical study, been proven to be a bad idea. Things will break.
 * So, this function is used instead for one thing and only one thing: the gold borders that follow the mouse through the inventory.
 *
 * Places used so far:
 * * Here. In updatePlayerInventory(), where it draws the gold borders around the inventory tile the mouse is hovering over.
 * * drawstatus.cpp: drawing the gold borders around the hotbar slot the mouse is hovering over
 */
bool mouseInBoundsRealtimeCoords(int x1, int x2, int y1, int y2)
{
	if (mousey >= y1 && mousey < y2)
		if (mousex >= x1 && mousex < x2)
		{
			return true;
		}

	return false;
}

void drawBlueInventoryBorder(const Item& item, int x, int y)
{
	SDL_Rect pos;
	pos.x = x + item.x * INVENTORY_SLOTSIZE + 2;
	pos.y = y + item.y * INVENTORY_SLOTSIZE + 1;
	pos.w = INVENTORY_SLOTSIZE;
	pos.h = INVENTORY_SLOTSIZE;

	Uint32 color = SDL_MapRGBA(mainsurface->format, 0, 0, 255, 127);
	drawBox(&pos, color, 127);
}

void updatePlayerInventory()
{
	bool disableMouseDisablingHotbarFocus = false;
	SDL_Rect pos, mode_pos;
	node_t* node, *nextnode;
	int x, y;

	x = INVENTORY_STARTX;
	y = INVENTORY_STARTY;

	// draw translucent box
	pos.x = x;
	pos.y = y;
	pos.w = INVENTORY_SIZEX * INVENTORY_SLOTSIZE;
	pos.h = INVENTORY_SIZEY * INVENTORY_SLOTSIZE;
	drawRect(&pos, 0, 224);

	if ( game_controller )
	{
		if ( gui_mode == GUI_MODE_SHOP )
		{
			if ( *inputPressed(joyimpulses[INJOY_MENU_CYCLE_SHOP_LEFT]) )
			{
				*inputPressed(joyimpulses[INJOY_MENU_CYCLE_SHOP_LEFT]) = 0;
				cycleShopCategories(-1);
			}
			if ( *inputPressed(joyimpulses[INJOY_MENU_CYCLE_SHOP_RIGHT]) )
			{
				*inputPressed(joyimpulses[INJOY_MENU_CYCLE_SHOP_RIGHT]) = 0;
				cycleShopCategories(1);
			}
		}

		if ( selectedChestSlot < 0 && selectedShopSlot < 0 
			&& selectedIdentifySlot < 0 && selectedRemoveCurseSlot < 0 
			&& !itemMenuOpen && game_controller->handleInventoryMovement()
			&& GenericGUI.selectedSlot < 0 )
		{
			if ( selectedChestSlot < 0 && selectedShopSlot < 0 
				&& selectedIdentifySlot < 0 && selectedRemoveCurseSlot < 0
				&& GenericGUI.selectedSlot < 0 ) //This second check prevents the extra mouse warp.
			{
				if ( !hotbarHasFocus )
				{
					warpMouseToSelectedInventorySlot();
				}
				else
				{
					disableMouseDisablingHotbarFocus = true;
				}
			}
		}
		else if ( selectedChestSlot >= 0 && !itemMenuOpen && game_controller->handleChestMovement() )
		{
			if ( selectedChestSlot < 0 )
			{
				//Move out of chest. Warp cursor back to selected inventory slot.
				warpMouseToSelectedInventorySlot();
			}
		}
		else if ( selectedShopSlot >= 0 && !itemMenuOpen && game_controller->handleShopMovement() )
		{
			if ( selectedShopSlot < 0 )
			{
				warpMouseToSelectedInventorySlot();
			}
		}
		else if ( selectedIdentifySlot >= 0 && !itemMenuOpen && game_controller->handleIdentifyMovement() )
		{
			if ( selectedIdentifySlot < 0 )
			{
				warpMouseToSelectedInventorySlot();
			}
		}
		else if ( selectedRemoveCurseSlot >= 0 && !itemMenuOpen && game_controller->handleRemoveCurseMovement() )
		{
			if ( selectedRemoveCurseSlot < 0 )
			{
				warpMouseToSelectedInventorySlot();
			}
		}
		else if ( GenericGUI.selectedSlot >= 0 && !itemMenuOpen && game_controller->handleRepairGUIMovement() )
		{
			if ( GenericGUI.selectedSlot < 0 )
			{
				GenericGUI.warpMouseToSelectedSlot();
			}
		}

		if ( *inputPressed(joyimpulses[INJOY_MENU_INVENTORY_TAB]) )
		{
			*inputPressed(joyimpulses[INJOY_MENU_INVENTORY_TAB]) = 0;
			cycleInventoryTab();
		}

		if ( lastkeypressed == 300 )
		{
			lastkeypressed = 0;
		}

		if ( *inputPressed(joyimpulses[INJOY_MENU_MAGIC_TAB]) )
		{
			*inputPressed(joyimpulses[INJOY_MENU_MAGIC_TAB]) = 0;
			cycleInventoryTab();
		}
	}

	if ( !command && *inputPressed(impulses[IN_AUTOSORT]) )
	{
		autosortInventory();
		//quickStackItems();
		*inputPressed(impulses[IN_AUTOSORT]) = 0;
		playSound(139, 64);
	}

	// draw grid
	pos.x = x;
	pos.y = y;
	pos.w = INVENTORY_SIZEX * INVENTORY_SLOTSIZE;
	pos.h = INVENTORY_SIZEY * INVENTORY_SLOTSIZE;
	drawLine(pos.x, pos.y, pos.x, pos.y + pos.h, SDL_MapRGB(mainsurface->format, 150, 150, 150), 255);
	drawLine(pos.x, pos.y, pos.x + pos.w, pos.y, SDL_MapRGB(mainsurface->format, 150, 150, 150), 255);
	for ( x = 0; x <= INVENTORY_SIZEX; x++ )
	{
		drawLine(pos.x + x * INVENTORY_SLOTSIZE, pos.y, pos.x + x * INVENTORY_SLOTSIZE, pos.y + pos.h, SDL_MapRGB(mainsurface->format, 150, 150, 150), 255);
	}
	for ( y = 0; y <= INVENTORY_SIZEY; y++ )
	{
		drawLine(pos.x, pos.y + y * INVENTORY_SLOTSIZE, pos.x + pos.w, pos.y + y * INVENTORY_SLOTSIZE, SDL_MapRGB(mainsurface->format, 150, 150, 150), 255);
	}

	if ( !itemMenuOpen 
		&& selectedChestSlot < 0 && selectedShopSlot < 0 
		&& selectedIdentifySlot < 0 && selectedRemoveCurseSlot < 0
		&& GenericGUI.selectedSlot < 0 )
	{
		//Highlight (draw a gold border) currently selected inventory slot (for gamepad).
		//Only if item menu is not open, no chest slot is selected, no shop slot is selected, no Identify GUI slot is selected, and no Remove Curse GUI slot is selected.
		pos.w = INVENTORY_SLOTSIZE;
		pos.h = INVENTORY_SLOTSIZE;
		for (x = 0; x < INVENTORY_SIZEX; ++x)
		{
			for (y = 0; y < INVENTORY_SIZEY; ++y)
			{
				pos.x = INVENTORY_STARTX + x * INVENTORY_SLOTSIZE;
				pos.y = INVENTORY_STARTY + y * INVENTORY_SLOTSIZE;

				//Cursor moved over this slot, highlight it.
				if (mouseInBoundsRealtimeCoords(pos.x, pos.x + pos.w, pos.y, pos.y + pos.h))
				{
					selected_inventory_slot_x = x;
					selected_inventory_slot_y = y;
					if ( hotbarHasFocus && !disableMouseDisablingHotbarFocus )
					{
						hotbarHasFocus = false; //Utter bodge to fix hotbar nav on OS X.
					}
				}

				if ( x == selected_inventory_slot_x && y == selected_inventory_slot_y && !hotbarHasFocus )
				{
					Uint32 color = SDL_MapRGBA(mainsurface->format, 255, 255, 0, 127);
					drawBox(&pos, color, 127);
				}
			}
		}
	}


	// draw contents of each slot
	x = INVENTORY_STARTX;
	y = INVENTORY_STARTY;
	for ( node = stats[clientnum]->inventory.first; node != NULL; node = nextnode )
	{
		nextnode = node->next;
		Item* item = (Item*)node->element;

		if ( item == selectedItem || (inventory_mode == INVENTORY_MODE_ITEM && itemCategory(item) == SPELL_CAT) || (inventory_mode == INVENTORY_MODE_SPELL && itemCategory(item) != SPELL_CAT) )
		{
			//Item is selected, or, item is a spell but it's item inventory mode, or, item is an item but it's spell inventory mode...(this filters out items)
			if ( !(inventory_mode == INVENTORY_MODE_ITEM && itemCategory(item) == SPELL_CAT) || (inventory_mode == INVENTORY_MODE_SPELL && itemCategory(item) != SPELL_CAT) )
			{
				if ( item == selectedItem )
				{
					//Draw blue border around the slot if it's the currently grabbed item.
					drawBlueInventoryBorder(*item, x, y);
				}
			}
			continue;
		}

		pos.x = x + item->x * (INVENTORY_SLOTSIZE) + 2;
		pos.y = y + item->y * (INVENTORY_SLOTSIZE) + 1;
		pos.w = (INVENTORY_SLOTSIZE) - 2;
		pos.h = (INVENTORY_SLOTSIZE) - 2;
		if (!item->identified)
		{
			// give it a yellow background if it is unidentified
			drawRect(&pos, SDL_MapRGB(mainsurface->format, 128, 128, 0), 125); //31875
		}
		else if (item->beatitude < 0)
		{
			// give it a red background if cursed
			drawRect(&pos, SDL_MapRGB(mainsurface->format, 128, 0, 0), 125);
		}
		else if (item->beatitude > 0)
		{
			// give it a green background if blessed (light blue if colorblind mode)
			if (colorblind)
			{
				drawRect(&pos, SDL_MapRGB(mainsurface->format, 100, 245, 255), 65);
			}
			else
			{
				drawRect(&pos, SDL_MapRGB(mainsurface->format, 0, 255, 0), 65);
			}
		}
		if ( item->status == BROKEN )
		{
			drawRect(&pos, SDL_MapRGB(mainsurface->format, 160, 160, 160), 64);
		}

		if ( itemMenuOpen && item == uidToItem(itemMenuItem) )
		{
			//Draw blue border around the slot if it's the currently context menu'd item.
			drawBlueInventoryBorder(*item, x, y);
		}

		// draw item
		pos.x = x + item->x * INVENTORY_SLOTSIZE + 4 * uiscale_inventory;
		pos.y = y + item->y * INVENTORY_SLOTSIZE + 4 * uiscale_inventory;
		pos.w = 32 * uiscale_inventory;
		pos.h = 32 * uiscale_inventory;
		if ( itemSprite(item) )
		{
			drawImageScaled(itemSprite(item), NULL, &pos);
		}

		bool greyedOut = false;
		if ( players[clientnum] && players[clientnum]->entity && players[clientnum]->entity->effectShapeshift != NOTHING )
		{
			// shape shifted, disable some items
			if ( !item->usableWhileShapeshifted(stats[clientnum]) )
			{
				SDL_Rect greyBox;
				greyBox.x = x + item->x * (INVENTORY_SLOTSIZE)+2;
				greyBox.y = y + item->y * (INVENTORY_SLOTSIZE)+1;
				greyBox.w = (INVENTORY_SLOTSIZE)-2;
				greyBox.h = (INVENTORY_SLOTSIZE)-2;
				drawRect(&greyBox, SDL_MapRGB(mainsurface->format, 64, 64, 64), 144);
				greyedOut = true;
			}
		}
		if ( !greyedOut && client_classes[clientnum] == CLASS_SHAMAN 
			&& item->type == SPELL_ITEM && !(playerUnlockedShamanSpell(clientnum, item)) )
		{
			SDL_Rect greyBox;
			greyBox.x = x + item->x * (INVENTORY_SLOTSIZE)+2;
			greyBox.y = y + item->y * (INVENTORY_SLOTSIZE)+1;
			greyBox.w = (INVENTORY_SLOTSIZE)-2;
			greyBox.h = (INVENTORY_SLOTSIZE)-2;
			drawRect(&greyBox, SDL_MapRGB(mainsurface->format, 64, 64, 64), 144);
			greyedOut = true;
		}

		// item count
		if ( item->count > 1 )
		{
			if ( uiscale_inventory < 1.5 )
			{
				printTextFormatted(font8x8_bmp, pos.x + pos.w - 8 * uiscale_inventory, pos.y + pos.h - 8 * uiscale_inventory, "%d", item->count);
			}
			else
			{
				printTextFormatted(font12x12_bmp, pos.x + pos.w - 12, pos.y + pos.h - 12, "%d", item->count);
			}
		}

		// item equipped
		if ( itemCategory(item) != SPELL_CAT )
		{
			if ( itemIsEquipped(item, clientnum) )
			{
				pos.x = x + item->x * INVENTORY_SLOTSIZE + 2;
				pos.y = y + item->y * INVENTORY_SLOTSIZE + INVENTORY_SLOTSIZE - 18;
				pos.w = 16;
				pos.h = 16;
				drawImage(equipped_bmp, NULL, &pos);
			}
			else if ( item->status == BROKEN )
			{
				pos.x = x + item->x * INVENTORY_SLOTSIZE + 2;
				pos.y = y + item->y * INVENTORY_SLOTSIZE + INVENTORY_SLOTSIZE - 18;
				pos.w = 16;
				pos.h = 16;
				drawImage(itembroken_bmp, NULL, &pos);
			}
		}
		else
		{
			spell_t* spell = getSpellFromItem(item);
			if ( selected_spell == spell 
				&& (selected_spell_last_appearance == item->appearance || selected_spell_last_appearance == -1) )
			{
				pos.x = x + item->x * INVENTORY_SLOTSIZE + 2;
				pos.y = y + item->y * INVENTORY_SLOTSIZE + INVENTORY_SLOTSIZE - 18;
				pos.w = 16;
				pos.h = 16;
				drawImage(equipped_bmp, NULL, &pos);
			}
		}
	}
	// autosort button
	mode_pos.x = x + INVENTORY_SIZEX * INVENTORY_SLOTSIZE + inventory_mode_item_img->w * uiscale_inventory + 2;
	mode_pos.y = y;
	mode_pos.w = 24;
	mode_pos.h = 24;
	bool mouse_in_bounds = mouseInBounds(mode_pos.x, mode_pos.x + mode_pos.w, mode_pos.y, mode_pos.y + mode_pos.h);
	if ( !mouse_in_bounds )
	{
		drawWindow(mode_pos.x, mode_pos.y, mode_pos.x + mode_pos.w, mode_pos.y + mode_pos.h);
	}
	else
	{
		drawDepressed(mode_pos.x, mode_pos.y, mode_pos.x + mode_pos.w, mode_pos.y + mode_pos.h);
	}
	ttfPrintText(ttf12, mode_pos.x, mode_pos.y + 6, "||");
	if ( mouse_in_bounds )
	{
		mode_pos.x += 2;
		mode_pos.y += 2;
		mode_pos.w -= 4;
		mode_pos.h -= 4;
		drawRect(&mode_pos, SDL_MapRGB(mainsurface->format, 192, 192, 192), 64);
		// tooltip
		SDL_Rect src;
		src.x = mousex + 16;
		src.y = mousey + 8;
		src.h = TTF12_HEIGHT + 8;
		src.w = longestline(language[2960]) * TTF12_WIDTH + 8;
		drawTooltip(&src);
		ttfPrintTextFormatted(ttf12, src.x + 4, src.y + 4, language[2960], getInputName(impulses[IN_AUTOSORT]));
		if ( mousestatus[SDL_BUTTON_LEFT] )
		{
			mousestatus[SDL_BUTTON_LEFT] = 0;
			autosortInventory();
			playSound(139, 64);
		}
	}
	// do inventory mode buttons
	mode_pos.x = x + INVENTORY_SIZEX * INVENTORY_SLOTSIZE + 1;
	mode_pos.y = y + inventory_mode_spell_img->h * uiscale_inventory;
	mode_pos.w = inventory_mode_spell_img->w * uiscale_inventory;
	mode_pos.h = inventory_mode_spell_img->h * uiscale_inventory + 1;
	mouse_in_bounds = mouseInBounds(mode_pos.x, mode_pos.x + mode_pos.w,
		mode_pos.y, mode_pos.y + mode_pos.h);
	if (mouse_in_bounds)
	{
		drawImageScaled(inventory_mode_spell_highlighted_img, NULL, &mode_pos);

		// tooltip
		SDL_Rect src;
		src.x = mousex + 16;
		src.y = mousey + 8;
		src.h = TTF12_HEIGHT + 8;
		src.w = longestline(language[342]) * TTF12_WIDTH + 8;
		drawTooltip(&src);
		ttfPrintText(ttf12, src.x + 4, src.y + 4, language[342]);

		if (mousestatus[SDL_BUTTON_LEFT])
		{
			mousestatus[SDL_BUTTON_LEFT] = 0;
			inventory_mode = INVENTORY_MODE_SPELL;
			playSound(139, 64);
		}
	}
	else
	{
		drawImageScaled(inventory_mode_spell_img, NULL, &mode_pos);
	}
	mode_pos.x = x + INVENTORY_SIZEX * INVENTORY_SLOTSIZE + 1;
	mode_pos.y = y - 1;
	mode_pos.w = inventory_mode_item_img->w * uiscale_inventory;
	mode_pos.h = inventory_mode_item_img->h * uiscale_inventory + 2;
	mouse_in_bounds = mouseInBounds(mode_pos.x, mode_pos.x + mode_pos.w,
		mode_pos.y, mode_pos.y + mode_pos.h);
	if (mouse_in_bounds)
	{
		drawImageScaled(inventory_mode_item_highlighted_img, NULL, &mode_pos);

		// tooltip
		SDL_Rect src;
		src.x = mousex + 16;
		src.y = mousey + 8;
		src.h = TTF12_HEIGHT + 8;
		src.w = longestline(language[343]) * TTF12_WIDTH + 8;
		drawTooltip(&src);
		ttfPrintText(ttf12, src.x + 4, src.y + 4, language[343]);

		if (mousestatus[SDL_BUTTON_LEFT])
		{
			mousestatus[SDL_BUTTON_LEFT] = 0;
			inventory_mode = INVENTORY_MODE_ITEM;
			playSound(139, 64);
		}
	}
	else
	{
		drawImageScaled(inventory_mode_item_img, NULL, &mode_pos);
	}

	// mouse interactions
	if ( !selectedItem )
	{
		for ( node = stats[clientnum]->inventory.first; node != NULL; node = nextnode )
		{
			nextnode = node->next;
			Item* item = (Item*)node->element;  //I don't like that there's not a check that either are null.

			if (item)
			{
				pos.x = x + item->x * INVENTORY_SLOTSIZE + 4;
				pos.y = y + item->y * INVENTORY_SLOTSIZE + 4;
				pos.w = INVENTORY_SLOTSIZE - 8;
				pos.h = INVENTORY_SLOTSIZE - 8;

				if ( omousex >= pos.x && omousey >= pos.y && omousex < pos.x + pos.w && omousey < pos.y + pos.h )
				{
					// tooltip
					if ((inventory_mode == INVENTORY_MODE_ITEM && itemCategory(item) == SPELL_CAT) || (inventory_mode == INVENTORY_MODE_SPELL && itemCategory(item) != SPELL_CAT))
					{
						continue;    //Skip over this items since the filter is blocking it (eg spell in normal inventory or vice versa).
					}
					if ( !itemMenuOpen )
					{
						SDL_Rect src;
						src.x = mousex + 16;
						src.y = mousey + 8;
						if (itemCategory(item) == SPELL_CAT)
						{
							spell_t* spell = getSpellFromItem(item);
							drawSpellTooltip(spell, item, nullptr);
						}
						else
						{
							src.w = std::max(13, longestline(item->description())) * TTF12_WIDTH + 8;
							src.h = TTF12_HEIGHT * 4 + 8;
							char spellEffectText[256] = "";
							if ( item->identified )
							{
								bool learnedSpellbook = false;
								if ( itemCategory(item) == SPELLBOOK )
								{
									learnedSpellbook = playerLearnedSpellbook(item);
									if ( !learnedSpellbook && stats[clientnum] && players[clientnum] && players[clientnum]->entity )
									{
										// spellbook tooltip shows if you have the magic requirement as well (for goblins)
										int skillLVL = stats[clientnum]->PROFICIENCIES[PRO_MAGIC] + statGetINT(stats[clientnum], players[clientnum]->entity);
										spell_t* spell = getSpellFromID(getSpellIDFromSpellbook(item->type));
										if ( spell && skillLVL >= spell->difficulty )
										{
											learnedSpellbook = true;
										}
									}
								}

								if ( itemCategory(item) == WEAPON || itemCategory(item) == ARMOR || itemCategory(item) == THROWN
									|| itemTypeIsQuiver(item->type) )
								{
									src.h += TTF12_HEIGHT;
								}
								else if ( itemCategory(item) == SCROLL && item->identified )
								{
									src.h += TTF12_HEIGHT;
									src.w = std::max((2 + longestline(language[3862]) + longestline(item->getScrollLabel())) * TTF12_WIDTH + 8, src.w);
								}
								else if ( itemCategory(item) == SPELLBOOK && learnedSpellbook )
								{
									int height = 1;
									char effectType[32] = "";
									int spellID = getSpellIDFromSpellbook(item->type);
									int damage = drawSpellTooltip(getSpellFromID(spellID), item, nullptr);
									real_t dummy = 0.f;
									getSpellEffectString(spellID, spellEffectText, effectType, damage, &height, &dummy);
									int width = longestline(spellEffectText) * TTF12_WIDTH + 8;
									if ( width > src.w )
									{
										src.w = width;
									}
									src.h += height * TTF12_HEIGHT;
								}
								else if ( item->type == TOOL_GYROBOT || item->type == TOOL_DUMMYBOT
									|| item->type == TOOL_SENTRYBOT || item->type == TOOL_SPELLBOT
									|| (item->type == ENCHANTED_FEATHER && item->identified) )
								{
									src.w += 7 * TTF12_WIDTH;
								}
							}
							int furthestX = xres;
							if ( proficienciesPage == 0 )
							{
								if ( src.y < interfaceSkillsSheet.y + interfaceSkillsSheet.h )
								{
									furthestX = xres - interfaceSkillsSheet.w;
								}
							}
							else
							{
								if ( src.y < interfacePartySheet.y + interfacePartySheet.h )
								{
									furthestX = xres - interfacePartySheet.w;
								}
							}
							if ( src.x + src.w + 16 > furthestX ) // overflow right side of screen
							{
								src.x -= (src.w + 32);
							}

							drawTooltip(&src);

							Uint32 color = 0xFFFFFFFF;
							if ( !item->identified )
							{
								color = SDL_MapRGB(mainsurface->format, 255, 255, 0);
								ttfPrintTextFormattedColor( ttf12, src.x + 4 + TTF12_WIDTH, src.y + 4 + TTF12_HEIGHT, color, language[309] );
							}
							else
							{
								if (item->beatitude < 0)
								{
									//Red if cursed
									color = SDL_MapRGB(mainsurface->format, 255, 0, 0);
									ttfPrintTextFormattedColor(ttf12, src.x + 4 + TTF12_WIDTH, src.y + 4 + TTF12_HEIGHT, color, language[310]);
								}
								else if (item->beatitude == 0)
								{
									//White if normal item.
									color = 0xFFFFFFFF;
									ttfPrintTextFormattedColor(ttf12, src.x + 4 + TTF12_WIDTH, src.y + 4 + TTF12_HEIGHT, color, language[311]);
								}
								else
								{
									//Green if blessed.
									if (colorblind)
									{
										color = SDL_MapRGB(mainsurface->format, 100, 245, 255); //Light blue if colorblind
									}
									else
									{
										color = SDL_MapRGB(mainsurface->format, 0, 255, 0);
									}

									ttfPrintTextFormattedColor(ttf12, src.x + 4 + TTF12_WIDTH, src.y + 4 + TTF12_HEIGHT, color, language[312]);
								}
							}
							if ( item->beatitude == 0 || !item->identified )
							{
								color = 0xFFFFFFFF;
							}

							if ( item->type == TOOL_GYROBOT || item->type == TOOL_DUMMYBOT
								|| item->type == TOOL_SENTRYBOT || item->type == TOOL_SPELLBOT )
							{
								int health = 100;
								if ( !item->tinkeringBotIsMaxHealth() )
								{
									health = 25 * (item->appearance % 10);
									if ( health == 0 && item->status != BROKEN )
									{
										health = 5;
									}
								}
								ttfPrintTextFormattedColor(ttf12, src.x + 4, src.y + 4, color, "%s (%d%%)", item->description(), health);
							}
							else if ( item->type == ENCHANTED_FEATHER && item->identified )
							{
								ttfPrintTextFormattedColor(ttf12, src.x + 4, src.y + 4, color, "%s (%d%%)", item->description(), item->appearance % ENCHANTED_FEATHER_MAX_DURABILITY);
							}
							else
							{
								ttfPrintTextFormattedColor( ttf12, src.x + 4, src.y + 4, color, "%s", item->description());
							}
							int itemWeight = items[item->type].weight * item->count;
							if ( itemTypeIsQuiver(item->type) )
							{
								itemWeight = std::max(1, itemWeight / 5);
							}
							ttfPrintTextFormatted( ttf12, src.x + 4 + TTF12_WIDTH, src.y + 4 + TTF12_HEIGHT * 2, language[313], itemWeight);
							ttfPrintTextFormatted( ttf12, src.x + 4 + TTF12_WIDTH, src.y + 4 + TTF12_HEIGHT * 3, language[314], item->sellValue(clientnum));
							if ( strcmp(spellEffectText, "") )
							{
								ttfPrintTextFormattedColor(ttf12, src.x + 4, src.y + 4 + TTF12_HEIGHT * 4, SDL_MapRGB(mainsurface->format, 0, 255, 255), spellEffectText);
							}

							if ( item->identified )
							{
								if ( itemCategory(item) == WEAPON || itemCategory(item) == THROWN 
									|| itemTypeIsQuiver(item->type) )
								{
									Monster tmpRace = stats[clientnum]->type;
									if ( stats[clientnum]->type == TROLL
										|| stats[clientnum]->type == RAT
										|| stats[clientnum]->type == SPIDER
										|| stats[clientnum]->type == CREATURE_IMP )
									{
										// these monsters have 0 bonus from weapons, but want the tooltip to say the normal amount.
										stats[clientnum]->type = HUMAN;
									}

									if ( item->weaponGetAttack(stats[clientnum]) >= 0 )
									{
										color = SDL_MapRGB(mainsurface->format, 0, 255, 255);
									}
									else
									{
										color = SDL_MapRGB(mainsurface->format, 255, 0, 0);
									}
									if ( stats[clientnum]->type != tmpRace )
									{
										color = SDL_MapRGB(mainsurface->format, 127, 127, 127); // grey out the text if monster doesn't benefit.
									}

									ttfPrintTextFormattedColor( ttf12, src.x + 4 + TTF12_WIDTH, src.y + 4 + TTF12_HEIGHT * 4, color, language[315], item->weaponGetAttack(stats[clientnum]));
									stats[clientnum]->type = tmpRace;
								}
								else if ( itemCategory(item) == ARMOR )
								{
									Monster tmpRace = stats[clientnum]->type;
									if ( stats[clientnum]->type == TROLL
										|| stats[clientnum]->type == RAT
										|| stats[clientnum]->type == SPIDER
										|| stats[clientnum]->type == CREATURE_IMP )
									{
										// these monsters have 0 bonus from weapons, but want the tooltip to say the normal amount.
										stats[clientnum]->type = HUMAN;
									}

									if ( item->armorGetAC(stats[clientnum]) >= 0 )
									{
										color = SDL_MapRGB(mainsurface->format, 0, 255, 255);
									}
									else
									{
										color = SDL_MapRGB(mainsurface->format, 255, 0, 0);
									}
									if ( stats[clientnum]->type != tmpRace )
									{
										color = SDL_MapRGB(mainsurface->format, 127, 127, 127); // grey out the text if monster doesn't benefit.
									}

									ttfPrintTextFormattedColor( ttf12, src.x + 4 + TTF12_WIDTH, src.y + 4 + TTF12_HEIGHT * 4, color, language[316], item->armorGetAC(stats[clientnum]));
									stats[clientnum]->type = tmpRace;
								}
								else if ( itemCategory(item) == SCROLL )
								{
									color = SDL_MapRGB(mainsurface->format, 0, 255, 255);
									ttfPrintTextFormattedColor(ttf12, src.x + 4 + TTF12_WIDTH, src.y + 4 + TTF12_HEIGHT * 4, color, "%s%s", language[3862], item->getScrollLabel());
								}
							}
						}
					}

					if ( stats[clientnum]->HP <= 0 )
					{
						break;
					}

					if ( *inputPressed(joyimpulses[INJOY_MENU_DROP_ITEM]) 
						&& !itemMenuOpen && !selectedItem && selectedChestSlot < 0 
						&& selectedShopSlot < 0 && selectedIdentifySlot < 0 && selectedRemoveCurseSlot < 0
						&& GenericGUI.selectedSlot < 0 )
					{
						*inputPressed(joyimpulses[INJOY_MENU_DROP_ITEM]) = 0;
						if ( dropItem(item, clientnum) )
						{
							item = nullptr;
						}
					}

					bool disableItemUsage = false;
					if ( item )
					{
						if ( players[clientnum] && players[clientnum]->entity && players[clientnum]->entity->effectShapeshift != NOTHING )
						{
							// shape shifted, disable some items
							if ( !item->usableWhileShapeshifted(stats[clientnum]) )
							{
								disableItemUsage = true;
							}
						}
						if ( client_classes[clientnum] == CLASS_SHAMAN )
						{
							if ( item->type == SPELL_ITEM && !(playerUnlockedShamanSpell(clientnum, item)) )
							{
								disableItemUsage = true;
							}
						}
					}

					// handle clicking
					if ( (mousestatus[SDL_BUTTON_LEFT] 
						|| (*inputPressed(joyimpulses[INJOY_MENU_LEFT_CLICK]) 
							&& selectedChestSlot < 0 && selectedShopSlot < 0 
							&& selectedIdentifySlot < 0 && selectedRemoveCurseSlot < 0
							&& GenericGUI.selectedSlot < 0)) 
						&& !selectedItem && !itemMenuOpen )
					{
						if ( !(*inputPressed(joyimpulses[INJOY_MENU_LEFT_CLICK])) && (keystatus[SDL_SCANCODE_LSHIFT] || keystatus[SDL_SCANCODE_RSHIFT]) )
						{
							if ( dropItem(item, clientnum) ) // Quick item drop
							{
								item = nullptr;
							}
						}
						else
						{
							selectedItem = item;
							//itemSelectBehavior = BEHAVIOR_MOUSE;
							playSound(139, 64); // click sound

							toggleclick = false; //Default reset. Otherwise will break mouse support after using gamepad once to trigger a context menu.

							if ( *inputPressed(joyimpulses[INJOY_MENU_LEFT_CLICK]) )
							{
								*inputPressed(joyimpulses[INJOY_MENU_LEFT_CLICK]) = 0;
								//itemSelectBehavior = BEHAVIOR_GAMEPAD;
								toggleclick = true;
								mousestatus[SDL_BUTTON_LEFT] = 0;
								//TODO: Change the mouse cursor to THE HAND.
							}
						}
					}
					else if ( (mousestatus[SDL_BUTTON_RIGHT] 
						|| (*inputPressed(joyimpulses[INJOY_MENU_USE]) 
							&& selectedChestSlot < 0 && selectedShopSlot < 0 
							&& selectedIdentifySlot < 0 && selectedRemoveCurseSlot < 0
							&& GenericGUI.selectedSlot < 0)) 
						&& !itemMenuOpen && !selectedItem )
					{
						if ( (keystatus[SDL_SCANCODE_LSHIFT] || keystatus[SDL_SCANCODE_RSHIFT]) && !(*inputPressed(joyimpulses[INJOY_MENU_USE]) && selectedChestSlot < 0) ) //TODO: selected shop slot, identify, remove curse?
						{
							// auto-appraise the item
							identifygui_active = false;
							identifygui_appraising = true;
							identifyGUIIdentify(item);
							mousestatus[SDL_BUTTON_RIGHT] = 0;

							//Cleanup identify GUI gamecontroller code here.
							selectedIdentifySlot = -1;
						}
						else if ( !disableItemUsage && (itemCategory(item) == POTION || itemCategory(item) == SPELLBOOK || item->type == FOOD_CREAMPIE) &&
							(keystatus[SDL_SCANCODE_LALT] || keystatus[SDL_SCANCODE_RALT]) 
							&& !(*inputPressed(joyimpulses[INJOY_MENU_USE])) )
						{
							mousestatus[SDL_BUTTON_RIGHT] = 0;
							// force equip potion/spellbook
							playerTryEquipItemAndUpdateServer(item);
						}
						else
						{
							// open a drop-down menu of options for "using" the item
							itemMenuOpen = true;
							itemMenuX = mousex + 8;
							itemMenuY = mousey;
							itemMenuSelected = 0;
							itemMenuItem = item->uid;

							toggleclick = false; //Default reset. Otherwise will break mouse support after using gamepad once to trigger a context menu.

							if ( *inputPressed(joyimpulses[INJOY_MENU_USE]) )
							{
								*inputPressed(joyimpulses[INJOY_MENU_USE]) = 0;
								toggleclick = true;
							}
						}
					}

					bool numkey_quick_add = hotbar_numkey_quick_add;
					if ( item && itemCategory(item) == SPELL_CAT && item->appearance >= 1000 &&
						players[clientnum] && players[clientnum]->entity && players[clientnum]->entity->effectShapeshift )
					{
						if ( canUseShapeshiftSpellInCurrentForm(*item) != 1 )
						{
							numkey_quick_add = false;
						}
					}

					if ( numkey_quick_add && !command )
					{
						if ( keystatus[SDL_SCANCODE_1] )
						{
							keystatus[SDL_SCANCODE_1] = 0;
							hotbar[0].item = item->uid;
						}
						if ( keystatus[SDL_SCANCODE_2] )
						{
							keystatus[SDL_SCANCODE_2] = 0;
							hotbar[1].item = item->uid;
						}
						if ( keystatus[SDL_SCANCODE_3] )
						{
							keystatus[SDL_SCANCODE_3] = 0;
							hotbar[2].item = item->uid;
						}
						if ( keystatus[SDL_SCANCODE_4] )
						{
							keystatus[SDL_SCANCODE_4] = 0;
							hotbar[3].item = item->uid;
						}
						if ( keystatus[SDL_SCANCODE_5] )
						{
							keystatus[SDL_SCANCODE_5] = 0;
							hotbar[4].item = item->uid;
						}
						if ( keystatus[SDL_SCANCODE_6] )
						{
							keystatus[SDL_SCANCODE_6] = 0;
							hotbar[5].item = item->uid;
						}
						if ( keystatus[SDL_SCANCODE_7] )
						{
							keystatus[SDL_SCANCODE_7] = 0;
							hotbar[6].item = item->uid;
						}
						if ( keystatus[SDL_SCANCODE_8] )
						{
							keystatus[SDL_SCANCODE_8] = 0;
							hotbar[7].item = item->uid;
						}
						if ( keystatus[SDL_SCANCODE_9] )
						{
							keystatus[SDL_SCANCODE_9] = 0;
							hotbar[8].item = item->uid;
						}
						if ( keystatus[SDL_SCANCODE_0] )
						{
							keystatus[SDL_SCANCODE_0] = 0;
							hotbar[9].item = item->uid;
						}
					}
					break;
				}
			}
		}
	}
	else if ( stats[clientnum]->HP > 0 )
	{
		// releasing items
		releaseItem(x, y);
	}

	itemContextMenu();
}

inline bool itemMenuSkipRow1ForShopsAndChests(const Item& item)
{
	if ( (openedChest[clientnum] || gui_mode == GUI_MODE_SHOP) 
		&& (itemCategory(&item) == POTION || item.type == TOOL_ALEMBIC || item.type == TOOL_TINKERING_KIT || itemCategory(&item) == SPELLBOOK) )
	{
		return true;
	}
	return false;
}

/*
 * Helper function to drawItemMenuSlots. Draws the empty window for an individual item context menu slot.
 */
inline void drawItemMenuSlot(int x, int y, int width, int height, bool selected = false)
{
	if (selected)
	{
		drawDepressed(x, y, x + width, y + height);
	}
	else
	{
		drawWindow(x, y, x + width, y + height);
	}
}

/*
 * Helper function to itemContextMenu(). Draws the context menu slots.
 */
inline void drawItemMenuSlots(const Item& item, int slot_width, int slot_height)
{
	//Draw the action select boxes. "Appraise", "Use, "Equip", etc.
	int current_x = itemMenuX;
	int current_y = itemMenuY;
	drawItemMenuSlot(current_x, current_y, slot_width, slot_height, itemMenuSelected == 0); //Option 0 => Store in chest, sell, use.
	if (itemCategory(&item) != SPELL_CAT)
	{
		if ( itemMenuSkipRow1ForShopsAndChests(item) )
		{
			current_y += slot_height;
			drawItemMenuSlot(current_x, current_y, slot_width, slot_height, itemMenuSelected == 2); //Option 1 => appraise

			current_y += slot_height;
			drawItemMenuSlot(current_x, current_y, slot_width, slot_height, itemMenuSelected == 3); //Option 2 => drop
			return; // only draw 3 lines.
		}

		current_y += slot_height;
		drawItemMenuSlot(current_x, current_y, slot_width, slot_height, itemMenuSelected == 1); //Option 1 => wield, unwield, use, appraise

		current_y += slot_height;
		drawItemMenuSlot(current_x, current_y, slot_width, slot_height, itemMenuSelected == 2); //Option 2 => appraise, drop

		if ( stats[clientnum] && stats[clientnum]->type == AUTOMATON && itemIsConsumableByAutomaton(item) 
			&& itemCategory(&item) != FOOD )
		{
			current_y += slot_height;
			drawItemMenuSlot(current_x, current_y, slot_width, slot_height, itemMenuSelected == 3); //Option 3 => drop
		}
		if (itemCategory(&item) == POTION || item.type == TOOL_ALEMBIC || item.type == TOOL_TINKERING_KIT )
		{
			current_y += slot_height;
			drawItemMenuSlot(current_x, current_y, slot_width, slot_height, itemMenuSelected == 3); //Option 3 => drop
		}
		else if ( itemCategory(&item) == SPELLBOOK )
		{
			current_y += slot_height;
			drawItemMenuSlot(current_x, current_y, slot_width, slot_height, itemMenuSelected == 3); //Option 3 => drop
		}
		else if ( item.type == FOOD_CREAMPIE )
		{
			current_y += slot_height;
			drawItemMenuSlot(current_x, current_y, slot_width, slot_height, itemMenuSelected == 3); //Option 3 => drop
		}
	}
}

/*
 * drawOptionX() - renders the specified option in the given item option menu slot.
 */
inline void drawOptionStoreInChest(int x, int y)
{
	int width = 0;
	TTF_SizeUTF8(ttf12, language[344], &width, nullptr);
	ttfPrintText(ttf12, x + 50 - width / 2, y + 4, language[344]);
}

inline void drawOptionSell(int x, int y)
{
	int width = 0;
	TTF_SizeUTF8(ttf12, language[345], &width, nullptr);
	ttfPrintText(ttf12, x + 50 - width / 2, y + 4, language[345]);
}

inline void drawOptionUse(const Item& item, int x, int y)
{
	int width = 0;
	ttfPrintTextFormatted(ttf12, x + 50 - strlen(itemUseString(&item)) * TTF12_WIDTH / 2, y + 4, "%s", itemUseString(&item));
}

inline void drawOptionUnwield(int x, int y)
{
	int width = 0;
	TTF_SizeUTF8(ttf12, language[323], &width, nullptr);
	ttfPrintText(ttf12, x + 50 - width / 2, y + 4, language[323]);
}

inline void drawOptionWield(int x, int y)
{
	int width = 0;
	TTF_SizeUTF8(ttf12, language[324], &width, nullptr);
	ttfPrintText(ttf12, x + 50 - width / 2, y + 4, language[324]);
}

inline void drawOptionAppraise(int x, int y)
{
	int width = 0;
	TTF_SizeUTF8(ttf12, language[1161], &width, nullptr);
	ttfPrintText(ttf12, x + 50 - width / 2, y + 4, language[1161]);
}

inline void drawOptionDrop(int x, int y)
{
	int width = 0;
	TTF_SizeUTF8(ttf12, language[1162], &width, nullptr);
	ttfPrintText(ttf12, x + 50 - width / 2, y + 4, language[1162]);
}

/*
 * Helper function to itemContextMenu(). Draws a spell's options.
 */
inline void drawItemMenuOptionSpell(const Item& item, int x, int y)
{
	if (itemCategory(&item) != SPELL_CAT)
	{
		return;
	}

	int width = 0;

	//Option 0.
	drawOptionUse(item, x, y);
}

/*
 * Helper function to itemContextMenu(). Draws a potion's options.
 */
inline void drawItemMenuOptionPotion(const Item& item, int x, int y, int height, bool is_potion_bad = false)
{
	if (itemCategory(&item) != POTION && item.type != TOOL_ALEMBIC && item.type != TOOL_TINKERING_KIT )
	{
		return;
	}

	int width = 0;

	//Option 0.
	if (openedChest[clientnum])
	{
		drawOptionStoreInChest(x, y);
	}
	else if (gui_mode == GUI_MODE_SHOP)
	{
		drawOptionSell(x, y);
	}
	else
	{
		if ( item.type == TOOL_TINKERING_KIT )
		{
			if ( itemIsEquipped(&item, clientnum) )
			{
				drawOptionUnwield(x, y);
			}
			else
			{
				drawOptionWield(x, y);
			}
		}
		else if (!is_potion_bad)
		{
			drawOptionUse(item, x, y);
		}
		else
		{
			if (itemIsEquipped(&item, clientnum))
			{
				drawOptionUnwield(x, y);
			}
			else
			{
				drawOptionWield(x, y);
			}
		}
	}
	y += height;

	//Option 1.
	if ( itemMenuSkipRow1ForShopsAndChests(item) )
	{
		// skip this row.
	}
	else
	{
		if (!is_potion_bad)
		{
			if ( item.type == TOOL_ALEMBIC )
			{
				TTF_SizeUTF8(ttf12, language[3341], &width, nullptr);
				ttfPrintText(ttf12, x + 50 - width / 2, y + 4, language[3341]);
			}
			else if ( item.type == TOOL_TINKERING_KIT )
			{
				TTF_SizeUTF8(ttf12, language[3670], &width, nullptr);
				ttfPrintText(ttf12, x + 50 - width / 2, y + 4, language[3670]);
			}
			else if (itemIsEquipped(&item, clientnum))
			{
				drawOptionUnwield(x, y);
			}
			else
			{
				drawOptionWield(x, y);
			}
		}
		else
		{
			drawOptionUse(item, x, y);
		}
		y += height;
	}

	//Option 1.
	drawOptionAppraise(x, y);
	y += height;

	//Option 2.
	drawOptionDrop(x, y);
}

inline void drawItemMenuOptionAutomaton(const Item& item, int x, int y, int height, bool is_potion_bad)
{
	int width = 0;

	//Option 0.
	if ( openedChest[clientnum] )
	{
		drawOptionStoreInChest(x, y);
	}
	else if ( gui_mode == GUI_MODE_SHOP )
	{
		drawOptionSell(x, y);
	}
	else
	{
		if ( !is_potion_bad )
		{
			if ( !itemIsConsumableByAutomaton(item) || (itemCategory(&item) != FOOD && item.type != TOOL_METAL_SCRAP && item.type != TOOL_MAGIC_SCRAP) )
			{
				drawOptionUse(item, x, y);
			}
			else
			{
				TTF_SizeUTF8(ttf12, language[3487], &width, nullptr);
				ttfPrintText(ttf12, x + 50 - width / 2, y + 4, language[3487]);
			}
		}
		else
		{
			if ( itemIsEquipped(&item, clientnum) )
			{
				drawOptionUnwield(x, y);
			}
			else
			{
				drawOptionWield(x, y);
			}
		}
	}
	y += height;

	//Option 1.
	if ( item.type == TOOL_METAL_SCRAP || item.type == TOOL_MAGIC_SCRAP )
	{
		TTF_SizeUTF8(ttf12, language[1881], &width, nullptr);
		ttfPrintText(ttf12, x + 50 - width / 2, y + 4, language[1881]);
		y += height;
	}
	else if ( itemCategory(&item) != FOOD )
	{
		TTF_SizeUTF8(ttf12, language[3487], &width, nullptr);
		ttfPrintText(ttf12, x + 50 - width / 2, y + 4, language[3487]);
		y += height;
	}

	//Option 1.
	drawOptionAppraise(x, y);
	y += height;

	//Option 2.
	drawOptionDrop(x, y);
}

/*
 * Helper function to itemContextMenu(). Draws all other items's options.
 */
inline void drawItemMenuOptionGeneric(const Item& item, int x, int y, int height)
{
	if (itemCategory(&item) == SPELL_CAT || itemCategory(&item) == POTION)
	{
		return;
	}

	int width = 0;

	//Option 0.
	if (openedChest[clientnum])
	{
		drawOptionStoreInChest(x, y);
	}
	else if (gui_mode == GUI_MODE_SHOP)
	{
		drawOptionSell(x, y);
	}
	else
	{
		drawOptionUse(item, x, y);
	}
	y += height;

	//Option 1
	drawOptionAppraise(x, y);
	y += height;

	//Option 2
	drawOptionDrop(x, y);
}

inline void drawItemMenuOptionSpellbook(const Item& item, int x, int y, int height, bool learnedSpell)
{
	int width = 0;

	//Option 0.
	if ( openedChest[clientnum] )
	{
		drawOptionStoreInChest(x, y);
	}
	else if ( gui_mode == GUI_MODE_SHOP )
	{
		drawOptionSell(x, y);
	}
	else
	{
		if ( learnedSpell )
		{
			if ( itemIsEquipped(&item, clientnum) )
			{
				drawOptionUnwield(x, y);
			}
			else
			{
				drawOptionWield(x, y);
			}
		}
		else
		{
			drawOptionUse(item, x, y);
		}
	}
	y += height;

	//Option 1
	if ( itemMenuSkipRow1ForShopsAndChests(item) )
	{
		// skip this row.
	}
	else
	{
		if ( learnedSpell )
		{
			drawOptionUse(item, x, y);
		}
		else
		{
			if ( itemIsEquipped(&item, clientnum) )
			{
				drawOptionUnwield(x, y);
			}
			else
			{
				drawOptionWield(x, y);
			}
		}
		y += height;
	}

	//Option 2
	drawOptionAppraise(x, y);
	y += height;

	//Option 3
	drawOptionDrop(x, y);
}

inline void drawItemMenuOptionUsableAndWieldable(const Item& item, int x, int y, int height)
{
	int width = 0;

	//Option 0.
	if ( openedChest[clientnum] )
	{
		drawOptionStoreInChest(x, y);
	}
	else if ( gui_mode == GUI_MODE_SHOP )
	{
		drawOptionSell(x, y);
	}
	else
	{
		drawOptionUse(item, x, y);
	}
	y += height;

	//Option 1
	if ( itemIsEquipped(&item, clientnum) )
	{
		drawOptionUnwield(x, y);
	}
	else
	{
		drawOptionWield(x, y);
	}
	y += height;

	//Option 2
	drawOptionAppraise(x, y);
	y += height;

	//Option 3
	drawOptionDrop(x, y);
}

/*
 * Helper function to itemContextMenu(). Changes the currently selected slot based on the mouse cursor's position.
 */
inline void selectItemMenuSlot(const Item& item, int x, int y, int slot_width, int slot_height)
{
	int current_x = itemMenuX;
	int current_y = itemMenuY;

	if (mousey < current_y - slot_height)   //Check if out of bounds above.
	{
		itemMenuSelected = -1; //For canceling out.
	}
	if (mousey >= current_y - 2 && mousey < current_y + slot_height)
	{
		itemMenuSelected = 0;
	}
	if (itemCategory(&item) != SPELL_CAT)
	{
		if ( itemMenuSkipRow1ForShopsAndChests(item) )
		{
			current_y += slot_height;
			if ( mousey >= current_y && mousey < current_y + slot_height )
			{
				itemMenuSelected = 2;
			}
			current_y += slot_height;
			if ( mousey >= current_y && mousey < current_y + slot_height )
			{
				itemMenuSelected = 3;
			}
		}
		else
		{
			current_y += slot_height;
			if (mousey >= current_y && mousey < current_y + slot_height)
			{
				itemMenuSelected = 1;
			}
			current_y += slot_height;
			if (mousey >= current_y && mousey < current_y + slot_height)
			{
				itemMenuSelected = 2;
			}
			current_y += slot_height;
			if ( itemCategory(&item) == POTION || itemCategory(&item) == SPELLBOOK || item.type == FOOD_CREAMPIE
				|| (stats[clientnum] && stats[clientnum]->type == AUTOMATON && itemIsConsumableByAutomaton(item) 
					&& itemCategory(&item) != FOOD) )
			{
				if (mousey >= current_y && mousey < current_y + slot_height)
				{
					itemMenuSelected = 3;
				}
				current_y += slot_height;
			}
		}
	}

	if (mousey >= current_y + slot_height)   //Check if out of bounds below.
	{
		itemMenuSelected = -1; //For canceling out.
	}

	if (mousex >= current_x + slot_width)   //Check if out of bounds to the right.
	{
		itemMenuSelected = -1; //For canceling out.
	}
	if ( mousex < itemMenuX - 10 || (mousex < itemMenuX && right_click_protect) )   //Check if out of bounds to the left.
	{
		itemMenuSelected = -1; //For canceling out.
	}
}

/*
 * execteItemMenuOptionX() -  Helper function to itemContextMenu(). Executes the specified menu option for the item.
 */
inline void executeItemMenuOption0(Item* item, bool is_potion_bad, bool learnedSpell)
{
	if (!item)
	{
		return;
	}

	bool disableItemUsage = false;
	if ( players[clientnum] && players[clientnum]->entity )
	{
		if ( players[clientnum]->entity->effectShapeshift != NOTHING )
		{
			// shape shifted, disable some items
			if ( !item->usableWhileShapeshifted(stats[clientnum]) )
			{
				disableItemUsage = true;
			}
		}
		else
		{
			if ( itemCategory(item) == SPELL_CAT && item->appearance >= 1000 )
			{
				if ( canUseShapeshiftSpellInCurrentForm(*item) != 1 )
				{
					disableItemUsage = true;
				}
			}
		}
	}

	if ( client_classes[clientnum] == CLASS_SHAMAN )
	{
		if ( item->type == SPELL_ITEM && !(playerUnlockedShamanSpell(clientnum, item)) )
		{
			disableItemUsage = true;
		}
	}

	if (openedChest[clientnum] && itemCategory(item) != SPELL_CAT)
	{
		//Option 0 = store in chest.
		openedChest[clientnum]->addItemToChestFromInventory(clientnum, item, false);
	}
	else if (gui_mode == GUI_MODE_SHOP && itemCategory(item) != SPELL_CAT)
	{
		//Option 0 = sell.
		sellItemToShop(item);
	}
	else
	{
		if (!is_potion_bad && !learnedSpell)
		{
			//Option 0 = use.
			if ( !(isItemEquippableInShieldSlot(item) && cast_animation.active_spellbook) )
			{
				if ( !disableItemUsage )
				{
					if ( stats[clientnum] && stats[clientnum]->type == AUTOMATON 
						&& (item->type == TOOL_METAL_SCRAP || item->type == TOOL_MAGIC_SCRAP) )
					{
						// consume item
						if ( multiplayer == CLIENT )
						{
							strcpy((char*)net_packet->data, "FODA");
							SDLNet_Write32((Uint32)item->type, &net_packet->data[4]);
							SDLNet_Write32((Uint32)item->status, &net_packet->data[8]);
							SDLNet_Write32((Uint32)item->beatitude, &net_packet->data[12]);
							SDLNet_Write32((Uint32)item->count, &net_packet->data[16]);
							SDLNet_Write32((Uint32)item->appearance, &net_packet->data[20]);
							net_packet->data[24] = item->identified;
							net_packet->data[25] = clientnum;
							net_packet->address.host = net_server.host;
							net_packet->address.port = net_server.port;
							net_packet->len = 26;
							sendPacketSafe(net_sock, -1, net_packet, 0);
						}
						item_FoodAutomaton(item, clientnum);
					}
					else
					{
						useItem(item, clientnum);
					}
				}
				else
				{
					if ( client_classes[clientnum] == CLASS_SHAMAN && item->type == SPELL_ITEM )
					{
						messagePlayer(clientnum, language[3488]); // unable to use with current level.
					}
					else
					{
						messagePlayer(clientnum, language[3432]); // unable to use in current form message.
					}
				}
			}
		}
		else
		{
			if ( !disableItemUsage )
			{
				//Option 0 = equip.
				playerTryEquipItemAndUpdateServer(item);
			}
			else
			{
				if ( client_classes[clientnum] == CLASS_SHAMAN && item->type == SPELL_ITEM )
				{
					messagePlayer(clientnum, language[3488]); // unable to use with current level.
				}
				else
				{
					messagePlayer(clientnum, language[3432]); // unable to use in current form message.
				}
			}
		}
	}
}

inline void executeItemMenuOption1(Item* item, bool is_potion_bad, bool learnedSpell)
{
	if (!item || itemCategory(item) == SPELL_CAT)
	{
		return;
	}

	bool disableItemUsage = false;
	if ( players[clientnum] && players[clientnum]->entity && players[clientnum]->entity->effectShapeshift != NOTHING )
	{
		// shape shifted, disable some items
		if ( !item->usableWhileShapeshifted(stats[clientnum]) )
		{
			disableItemUsage = true;
		}
		if ( item->type == FOOD_CREAMPIE )
		{
			disableItemUsage = true;
		}
	}

	if ( client_classes[clientnum] == CLASS_SHAMAN )
	{
		if ( item->type == SPELL_ITEM && !(playerUnlockedShamanSpell(clientnum, item)) )
		{
			disableItemUsage = true;
		}
	}

	if ( stats[clientnum] && stats[clientnum]->type == AUTOMATON && itemIsConsumableByAutomaton(*item) 
		&& itemCategory(item) != FOOD )
	{
		if ( item->type == TOOL_METAL_SCRAP || item->type == TOOL_MAGIC_SCRAP )
		{
			useItem(item, clientnum);
		}
		else
		{
			// consume item
			if ( multiplayer == CLIENT )
			{
				strcpy((char*)net_packet->data, "FODA");
				SDLNet_Write32((Uint32)item->type, &net_packet->data[4]);
				SDLNet_Write32((Uint32)item->status, &net_packet->data[8]);
				SDLNet_Write32((Uint32)item->beatitude, &net_packet->data[12]);
				SDLNet_Write32((Uint32)item->count, &net_packet->data[16]);
				SDLNet_Write32((Uint32)item->appearance, &net_packet->data[20]);
				net_packet->data[24] = item->identified;
				net_packet->data[25] = clientnum;
				net_packet->address.host = net_server.host;
				net_packet->address.port = net_server.port;
				net_packet->len = 26;
				sendPacketSafe(net_sock, -1, net_packet, 0);
			}
			item_FoodAutomaton(item, clientnum);
		}
	}
	else if ( item->type == TOOL_ALEMBIC )
	{
		// experimenting!
		if ( !disableItemUsage )
		{
			GenericGUI.openGUI(GUI_TYPE_ALCHEMY, true, item);
		}
		else
		{
			messagePlayer(clientnum, language[3432]); // unable to use in current form message.
		}
	}
	else if ( item->type == TOOL_TINKERING_KIT )
	{
		if ( !disableItemUsage )
		{
			GenericGUI.openGUI(GUI_TYPE_TINKERING, item);
		}
		else
		{
			messagePlayer(clientnum, language[3432]); // unable to use in current form message.
		}
	}
	else if (itemCategory(item) != POTION && itemCategory(item) != SPELLBOOK && item->type != FOOD_CREAMPIE)
	{
		//Option 1 = appraise.
		identifygui_active = false;
		identifygui_appraising = true;

		//Cleanup identify GUI gamecontroller code here.
		selectedIdentifySlot = -1;

		identifyGUIIdentify(item);
	}
	else
	{
		if ( !disableItemUsage )
		{
			if (!is_potion_bad && !learnedSpell)
			{
				//Option 1 = equip.
				playerTryEquipItemAndUpdateServer(item);
			}
			else
			{
				//Option 1 = drink/use/whatever.
				if ( !(isItemEquippableInShieldSlot(item) && cast_animation.active_spellbook) )
				{
					useItem(item, clientnum);
				}
			}
		}
		else
		{
			if ( client_classes[clientnum] == CLASS_SHAMAN && item->type == SPELL_ITEM )
			{
				messagePlayer(clientnum, language[3488]); // unable to use with current level.
			}
			else
			{
				messagePlayer(clientnum, language[3432]); // unable to use in current form message.
			}
		}
	}
}

inline void executeItemMenuOption2(Item* item)
{
	if (!item || itemCategory(item) == SPELL_CAT)
	{
		return;
	}

	if ( stats[clientnum] && stats[clientnum]->type == AUTOMATON && itemIsConsumableByAutomaton(*item) && itemCategory(item) != FOOD )
	{
		//Option 2 = appraise.
		identifygui_active = false;
		identifygui_appraising = true;

		//Cleanup identify GUI gamecontroller code here.
		selectedIdentifySlot = -1;

		identifyGUIIdentify(item);
	}
	else if ( itemCategory(item) != POTION && item->type != TOOL_ALEMBIC 
		&& itemCategory(item) != SPELLBOOK && item->type != TOOL_TINKERING_KIT
		&& item->type != FOOD_CREAMPIE )
	{
		//Option 2 = drop.
		dropItem(item, clientnum);
	}
	else
	{
		//Option 2 = appraise.
		identifygui_active = false;
		identifygui_appraising = true;

		//Cleanup identify GUI gamecontroller code here.
		selectedIdentifySlot = -1;

		identifyGUIIdentify(item);
	}
}

inline void executeItemMenuOption3(Item* item)
{
	if ( !item )
	{
		return;
	}
	if ( stats[clientnum] && stats[clientnum]->type == AUTOMATON && itemIsConsumableByAutomaton(*item) && itemCategory(item) != FOOD )
	{
		//Option 3 = drop (automaton has 4 options on consumable items).
		dropItem(item, clientnum);
		return;
	}
	if ( itemCategory(item) != POTION && item->type != TOOL_ALEMBIC 
		&& itemCategory(item) != SPELLBOOK && item->type != TOOL_TINKERING_KIT
		&& item->type != FOOD_CREAMPIE )
	{
		return;
	}

	//Option 3 = drop (only spellbooks/potions have option 3).
	dropItem(item, clientnum);
}

void itemContextMenu()
{
	if (!itemMenuOpen)
	{
		return;
	}

	if ( *inputPressed(joyimpulses[INJOY_MENU_CANCEL]) )
	{
		*inputPressed(joyimpulses[INJOY_MENU_CANCEL]) = 0;
		itemMenuOpen = false;
		//Warp cursor back into inventory, for gamepad convenience.
		SDL_WarpMouseInWindow(screen, INVENTORY_STARTX + (uidToItem(itemMenuItem)->x * INVENTORY_SLOTSIZE) + (INVENTORY_SLOTSIZE / 2), INVENTORY_STARTY + (uidToItem(itemMenuItem)->y * INVENTORY_SLOTSIZE) + (INVENTORY_SLOTSIZE / 2));
		return;
	}

	//Item *item = uidToItem(itemMenuItem);

	Item* current_item = uidToItem(itemMenuItem);
	if (!current_item)
	{
		itemMenuOpen = false;
		return;
	}

	bool is_potion_bad = false;
	if (current_item->identified)
	{
		is_potion_bad = isPotionBad(*current_item);
	}
	if ( current_item->type == POTION_EMPTY )
	{
		is_potion_bad = true; //So that you wield empty potions by default.
	}

	const int slot_width = 100;
	const int slot_height = 20;

	if ( game_controller->handleItemContextMenu(*current_item) )
	{
		SDL_WarpMouseInWindow(screen, itemMenuX + (slot_width / 2), itemMenuY + (itemMenuSelected * slot_height) + (slot_height / 2));
	}

	drawItemMenuSlots(*current_item, slot_width, slot_height);
	bool learnedSpell = false;

	if (itemCategory(current_item) == SPELL_CAT)
	{
		drawItemMenuOptionSpell(*current_item, itemMenuX, itemMenuY);
	}
	else
	{
		if ( stats[clientnum] && stats[clientnum]->type == AUTOMATON && itemIsConsumableByAutomaton(*current_item)
			&& current_item->type != FOOD_CREAMPIE )
		{
			drawItemMenuOptionAutomaton(*current_item, itemMenuX, itemMenuY, slot_height, is_potion_bad);
		}
		else if ( current_item->type == TOOL_ALEMBIC || current_item->type == TOOL_TINKERING_KIT )
		{
			drawItemMenuOptionPotion(*current_item, itemMenuX, itemMenuY, slot_height, false);
		}
		else if (itemCategory(current_item) == POTION || current_item->type == TOOL_ALEMBIC)
		{
			drawItemMenuOptionPotion(*current_item, itemMenuX, itemMenuY, slot_height, is_potion_bad);
		}
		else if ( itemCategory(current_item) == SPELLBOOK )
		{
			learnedSpell = playerLearnedSpellbook(current_item);
			if ( itemIsEquipped(current_item, clientnum) )
			{
				learnedSpell = true; // equipped spellbook will unequip on use.
			}
			else if ( players[clientnum] && players[clientnum]->entity )
			{
				if ( players[clientnum]->entity->effectShapeshift == CREATURE_IMP )
				{
					learnedSpell = true; // imps can't learn spells but always equip books.
				}
				else if ( stats[clientnum] && stats[clientnum]->type == GOBLIN )
				{
					learnedSpell = true; // goblinos can't learn spells but always equip books.
				}
			}

			drawItemMenuOptionSpellbook(*current_item, itemMenuX, itemMenuY, slot_height, learnedSpell);
		}
		else if ( current_item->type == FOOD_CREAMPIE )
		{
			drawItemMenuOptionUsableAndWieldable(*current_item, itemMenuX, itemMenuY, slot_height);
		}
		else
		{
			drawItemMenuOptionGeneric(*current_item, itemMenuX, itemMenuY, slot_height); //Every other item besides potions and spells.
		}
	}

	selectItemMenuSlot(*current_item, itemMenuX, itemMenuY, slot_width, slot_height);

	bool activateSelection = false;
	if (!mousestatus[SDL_BUTTON_RIGHT] && !toggleclick)
	{
		activateSelection = true;
	}
	else if ( *inputPressed(joyimpulses[INJOY_MENU_USE]) )
	{
		*inputPressed(joyimpulses[INJOY_MENU_USE]) = 0;
		activateSelection = true;
		//Warp cursor back into inventory, for gamepad convenience.
		SDL_WarpMouseInWindow(screen, INVENTORY_STARTX + (selected_inventory_slot_x * INVENTORY_SLOTSIZE) + (INVENTORY_SLOTSIZE / 2), INVENTORY_STARTY + (selected_inventory_slot_y * INVENTORY_SLOTSIZE) + (INVENTORY_SLOTSIZE / 2));
	}

	if (activateSelection)
	{
		switch (itemMenuSelected)
		{
			case 0:
				executeItemMenuOption0(current_item, is_potion_bad, learnedSpell);
				break;
			case 1:
				executeItemMenuOption1(current_item, is_potion_bad, learnedSpell);
				break;
			case 2:
				executeItemMenuOption2(current_item);
				break;
			case 3:
				executeItemMenuOption3(current_item);
				break;
			default:
				break;
		}

		//Close the menu.
		itemMenuOpen = false;
		itemMenuItem = 0;
	}
}

int numItemMenuSlots(const Item& item)
{
	int numSlots = 1; //Option 0 => store in chest, sell, use.

	if (itemCategory(&item) != SPELL_CAT)
	{
		numSlots += 2; //Option 1 => wield, unwield, use, appraise. & Option 2 => appraise, drop

		if (itemCategory(&item) == POTION)
		{
			numSlots += 1; //Option 3 => drop.
		}
		else if ( itemCategory(&item) == SPELLBOOK )
		{
			numSlots += 1; //Can read/equip spellbooks
		}
		else if ( item.type == FOOD_CREAMPIE )
		{
			numSlots += 1; //Can equip
		}
	}

	return numSlots;
}

//Used by the gamepad, primarily. Dpad buttons changes selection.
void selectItemMenuSlot(const Item& item, int entry)
{
	if (entry > numItemMenuSlots(item))
	{
		entry = 0;
	}
	if (entry < 0)
	{
		entry = numItemMenuSlots(item);
	}

	itemMenuSelected = entry;
}

// filters out items excluded by auto_hotbar_categories
bool autoAddHotbarFilter(const Item& item)
{
	Category cat = itemCategory(&item);
	for ( int i = 0; i < NUM_HOTBAR_CATEGORIES; ++i )
	{
		if ( auto_hotbar_categories[i] )
		{
			switch ( i )
			{
				case 0: // weapons
					if ( cat == WEAPON )
					{
						return true;
					}
					break;
				case 1: // armor
					if ( cat == ARMOR )
					{
						return true;
					}
					break;
				case 2: // jewelry
					if ( cat == RING || cat == AMULET )
					{
						return true;
					}
					break;
				case 3: // books/spellbooks
					if ( cat == BOOK || cat == SPELLBOOK )
					{
						return true;
					}
					break;
				case 4: // tools
					if ( cat == TOOL )
					{
						return true;
					}
					break;
				case 5: // thrown
					if ( cat == THROWN || item.type == GEM_ROCK )
					{
						return true;
					}
					break;
				case 6: // gems
					if ( cat == GEM )
					{
						return true;
					}
					break;
				case 7: // potions
					if ( cat == POTION )
					{
						return true;
					}
					break;
				case 8: // scrolls
					if ( cat == SCROLL )
					{
						return true;
					}
					break;
				case 9: // magicstaves
					if ( cat == MAGICSTAFF )
					{
						return true;
					}
					break;
				case 10: // food
					if ( cat == FOOD )
					{
						return true;
					}
					break;
				case 11: // spells
					if ( cat == SPELL_CAT )
					{
						return true;
					}
					break;
				default:
					break;
			}
		}
	}
	return false;
}

void quickStackItems()
{
	for ( node_t* node = stats[clientnum]->inventory.first; node != NULL; node = node->next )
	{
		Item* itemToStack = (Item*)node->element;
		if ( itemToStack && itemToStack->shouldItemStack(clientnum) )
		{
			for ( node_t* node = stats[clientnum]->inventory.first; node != NULL; node = node->next )
			{
				Item* item2 = (Item*)node->element;
				// if items are the same, check to see if they should stack
				if ( item2 && item2 != itemToStack && !itemCompare(itemToStack, item2, false) )
				{
					itemToStack->count += item2->count;
					if ( multiplayer == CLIENT && itemIsEquipped(itemToStack, clientnum) )
					{
						// if incrementing qty and holding item, then send "equip" for server to update their count of your held item.
						clientSendEquipUpdateToServer(EQUIP_ITEM_SLOT_WEAPON, EQUIP_ITEM_SUCCESS_UPDATE_QTY, clientnum,
							itemToStack->type, itemToStack->status,	itemToStack->beatitude, itemToStack->count, itemToStack->appearance, itemToStack->identified);
					}
					if ( item2->node )
					{
						list_RemoveNode(item2->node);
					}
					else
					{
						free(item2);
						item2 = nullptr;
					}
				}
			}
		}
	}
}

void autosortInventory()
{
	std::vector<std::pair<int, int>> autosortPairs;
	for ( int i = 0; i < NUM_AUTOSORT_CATEGORIES; ++i )
	{
		autosortPairs.push_back(std::make_pair(autosort_inventory_categories[i], i));
	}

	for ( node_t* node = stats[clientnum]->inventory.first; node != NULL; node = node->next )
	{
		Item* item = (Item*)node->element;
		if ( item && (!itemIsEquipped(item, clientnum) || autosort_inventory_categories[11] != 0) && itemCategory(item) != SPELL_CAT )
		{
			item->x = -1;
			item->y = 0;
			// move all items away.
		}
	}

	std::sort(autosortPairs.begin(), autosortPairs.end());

	// iterate and sort from highest to lowest priority, 1 to 9
	for ( std::vector<std::pair<int, int>>::reverse_iterator it = autosortPairs.rbegin(); it != autosortPairs.rend(); ++it )
	{
		std::pair<int, int> tmpPair = *it;
		if ( tmpPair.first > 0 )
		{
			//messagePlayer(0, "priority %d, category: %d", tmpPair.first, tmpPair.second);
			bool invertSortDirection = false;
			switch ( tmpPair.second )
			{
				case 0: // weapons
					sortInventoryItemsOfType(WEAPON, invertSortDirection);
					break;
				case 1: // armor
					sortInventoryItemsOfType(ARMOR, invertSortDirection);
					break;
				case 2: // jewelry
					sortInventoryItemsOfType(RING, invertSortDirection);
					sortInventoryItemsOfType(AMULET, invertSortDirection);
					break;
				case 3: // books/spellbooks
					sortInventoryItemsOfType(SPELLBOOK, invertSortDirection);
					sortInventoryItemsOfType(BOOK, invertSortDirection);
					break;
				case 4: // tools
					sortInventoryItemsOfType(TOOL, invertSortDirection);
					break;
				case 5: // thrown
					sortInventoryItemsOfType(THROWN, invertSortDirection);
					break;
				case 6: // gems
					sortInventoryItemsOfType(GEM, invertSortDirection);
					break;
				case 7: // potions
					sortInventoryItemsOfType(POTION, invertSortDirection);
					break;
				case 8: // scrolls
					sortInventoryItemsOfType(SCROLL, invertSortDirection);
					break;
				case 9: // magicstaves
					sortInventoryItemsOfType(MAGICSTAFF, invertSortDirection);
					break;
				case 10: // food
					sortInventoryItemsOfType(FOOD, invertSortDirection);
					break;
				case 11: // equipped items
					sortInventoryItemsOfType(-2, invertSortDirection);
					break;
				default:
					break;
			}
		}
	}

	// iterate and sort from lowest to highest priority, -9 to -1
	for ( std::vector<std::pair<int, int>>::iterator it = autosortPairs.begin(); it != autosortPairs.end(); ++it )
	{
		std::pair<int, int> tmpPair = *it;
		if ( tmpPair.first < 0 )
		{
			//messagePlayer(0, "priority %d, category: %d", tmpPair.first, tmpPair.second);
			bool invertSortDirection = true;
			switch ( tmpPair.second )
			{
				case 0: // weapons
					sortInventoryItemsOfType(WEAPON, invertSortDirection);
					break;
				case 1: // armor
					sortInventoryItemsOfType(ARMOR, invertSortDirection);
					break;
				case 2: // jewelry
					sortInventoryItemsOfType(RING, invertSortDirection);
					sortInventoryItemsOfType(AMULET, invertSortDirection);
					break;
				case 3: // books/spellbooks
					sortInventoryItemsOfType(SPELLBOOK, invertSortDirection);
					sortInventoryItemsOfType(BOOK, invertSortDirection);
					break;
				case 4: // tools
					sortInventoryItemsOfType(TOOL, invertSortDirection);
					break;
				case 5: // thrown
					sortInventoryItemsOfType(THROWN, invertSortDirection);
					break;
				case 6: // gems
					sortInventoryItemsOfType(GEM, invertSortDirection);
					break;
				case 7: // potions
					sortInventoryItemsOfType(POTION, invertSortDirection);
					break;
				case 8: // scrolls
					sortInventoryItemsOfType(SCROLL, invertSortDirection);
					break;
				case 9: // magicstaves
					sortInventoryItemsOfType(MAGICSTAFF, invertSortDirection);
					break;
				case 10: // food
					sortInventoryItemsOfType(FOOD, invertSortDirection);
					break;
				case 11: // equipped items
					sortInventoryItemsOfType(-2, invertSortDirection);
					break;
				default:
					break;
			}
		}
	}


	sortInventoryItemsOfType(-1, true); // clean up the rest of the items.
}

void sortInventoryItemsOfType(int categoryInt, bool sortRightToLeft)
{
	node_t* node = nullptr;
	Item* itemBeingSorted = nullptr;
	Category cat = static_cast<Category>(categoryInt);

	for ( node = stats[clientnum]->inventory.first; node != NULL; node = node->next )
	{
		itemBeingSorted = (Item*)node->element;
		if ( itemBeingSorted && itemBeingSorted->x == -1 )
		{
			if ( itemCategory(itemBeingSorted) == SPELL_CAT )
			{
				continue;
			}
			if ( categoryInt != -1 && categoryInt != -2 && itemCategory(itemBeingSorted) != cat )
			{
				if ( (itemBeingSorted->type == GEM_ROCK && categoryInt == THROWN) )
				{
					// exception for rocks as they are part of the thrown sort category...
				}
				else
				{
					// if item is not in the category specified, continue on.
					continue;
				}
			}
			if ( categoryInt == -2 && !itemIsEquipped(itemBeingSorted, clientnum) )
			{
				continue;
			}
			if ( categoryInt != -2 && itemIsEquipped(itemBeingSorted, clientnum) )
			{
				continue;
			}

			// find a place...
			int x, y;
			bool notfree = false, foundaspot = false;

			bool is_spell = false;
			int inventory_y = std::min(std::max(INVENTORY_SIZEY, 2), 3); // only sort y values of 2-3, if extra row don't auto sort into it.
			if ( itemCategory(itemBeingSorted) == SPELL_CAT )
			{
				is_spell = true;
				inventory_y = std::min(inventory_y, 3);
			}

			if ( sortRightToLeft )
			{
				x = INVENTORY_SIZEX - 1; // fill rightmost first.
			}
			else
			{
				x = 0; // fill leftmost first.
			}
			while ( 1 )
			{
				for ( y = 0; y < inventory_y; y++ )
				{
					node_t* node2 = nullptr;
					for ( node2 = stats[clientnum]->inventory.first; node2 != nullptr; node2 = node2->next )
					{
						Item* tempItem = (Item*)node2->element;
						if ( tempItem == itemBeingSorted )
						{
							continue;
						}
						if ( tempItem )
						{
							if ( tempItem->x == x && tempItem->y == y )
							{
								if ( is_spell && itemCategory(tempItem) == SPELL_CAT )
								{
									notfree = true;  //Both spells. Can't fit in the same slot.
								}
								else if ( !is_spell && itemCategory(tempItem) != SPELL_CAT )
								{
									notfree = true;  //Both not spells. Can't fit in the same slot.
								}
							}
						}
					}
					if ( notfree )
					{
						notfree = false;
						continue;
					}
					itemBeingSorted->x = x;
					itemBeingSorted->y = y;
					foundaspot = true;
					break;
				}
				if ( foundaspot )
				{
					break;
				}
				if ( sortRightToLeft )
				{
					--x; // fill rightmost first.
				}
				else
				{
					++x; // fill leftmost first.
				}
			}

			// backpack sorting, sort into here as last priority.
			if ( (x < 0 || x > INVENTORY_SIZEX - 1) && INVENTORY_SIZEY > 3 )
			{
				foundaspot = false;
				notfree = false;
				if ( sortRightToLeft )
				{
					x = INVENTORY_SIZEX - 1; // fill rightmost first.
				}
				else
				{
					x = 0; // fill leftmost first.
				}
				while ( 1 )
				{
					for ( y = 3; y < INVENTORY_SIZEY; y++ )
					{
						node_t* node2 = nullptr;
						for ( node2 = stats[clientnum]->inventory.first; node2 != nullptr; node2 = node2->next )
						{
							Item* tempItem = (Item*)node2->element;
							if ( tempItem == itemBeingSorted )
							{
								continue;
							}
							if ( tempItem )
							{
								if ( tempItem->x == x && tempItem->y == y )
								{
									if ( is_spell && itemCategory(tempItem) == SPELL_CAT )
									{
										notfree = true;  //Both spells. Can't fit in the same slot.
									}
									else if ( !is_spell && itemCategory(tempItem) != SPELL_CAT )
									{
										notfree = true;  //Both not spells. Can't fit in the same slot.
									}
								}
							}
						}
						if ( notfree )
						{
							notfree = false;
							continue;
						}
						itemBeingSorted->x = x;
						itemBeingSorted->y = y;
						foundaspot = true;
						break;
					}
					if ( foundaspot )
					{
						break;
					}
					if ( sortRightToLeft )
					{
						--x; // fill rightmost first.
					}
					else
					{
						++x; // fill leftmost first.
					}
				}
			}
		}
	}
}

bool mouseInsidePlayerInventory()
{
	SDL_Rect pos;
	pos.x = INVENTORY_STARTX;
	pos.y = INVENTORY_STARTY;
	pos.w = INVENTORY_SIZEX * INVENTORY_SLOTSIZE;
	pos.h = INVENTORY_SIZEY * INVENTORY_SLOTSIZE;
	return mouseInBounds(pos.x, pos.x + pos.w, pos.y, pos.y + pos.h);
}

bool mouseInsidePlayerHotbar()
{
	SDL_Rect pos;
	pos.x = HOTBAR_START_X;
	pos.y = STATUS_Y - hotbar_img->h * uiscale_hotbar;
	pos.w = NUM_HOTBAR_SLOTS * hotbar_img->w * uiscale_hotbar;
	pos.h = hotbar_img->h * uiscale_hotbar;
	return mouseInBounds(pos.x, pos.x + pos.w, pos.y, pos.y + pos.h);
}

bool playerLearnedSpellbook(Item* current_item)
{
	if ( !current_item )
	{
		return false;
	}
	if ( itemCategory(current_item) != SPELLBOOK )
	{
		return false;
	}
	for ( node_t* node = stats[clientnum]->inventory.first; node && current_item->identified; node = node->next )
	{
		Item* item = static_cast<Item*>(node->element);
		if ( !item )
		{
			continue;
		}
		//Search player's inventory for the special spell item.
		if ( itemCategory(item) != SPELL_CAT )
		{
			continue;
		}
		if ( item->appearance >= 1000 )
		{
			// special shaman racial spells, don't count this as being learnt
			continue;
		}
		spell_t *spell = getSpellFromItem(item); //Do not free or delete this.
		if ( !spell )
		{
			continue;
		}
		if ( current_item->type == getSpellbookFromSpellID(spell->ID) )
		{
			// learned spell, default option is now equip spellbook.
			return true;
		}
	}
	return false;
}