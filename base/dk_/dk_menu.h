//header file for the daikatana menu interface
//#pragma once

#ifndef _DK_MENU_H_
#define _DK_MENU_H_


void DKM_Enter(dk_main_button force_menu_button = DKMB_FIRST_MENU_IN);  // default to first menu
void DKM_Leave();
void DKM_MouseMove(int mx, int my);
void DKM_MousePos(int screen_x, int screen_y);
void DKM_MouseDown();
void DKM_MouseUp();
void DKM_Draw();      //draws the 2-d portion of the interface.
void DKM_Key(int key, qboolean down);
bool DKM_InMenu();    //returns true if the menu is active, false otherwise.
void DKM_ToggleEnterSound(bool toggle); 


#endif _DK_MENU_H_