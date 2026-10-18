#include "dk_shared.h"

#include "dk_misc.h"
#include "dk_buffer.h"
#include "dk_array.h"

#include "dk_menup.h"
#include "dk_menu_controls.h"

#include "keys.h"
#include "client.h"

///////////////////////////////////////////////////////////////////////////////////
//
//  Advanced Menu
//
///////////////////////////////////////////////////////////////////////////////////

CMenuSubAdvanced::CMenuSubAdvanced() {

}

void CMenuSubAdvanced::DrawForeground() {

}

void CMenuSubAdvanced::DrawBackground() {
    
}

bool CMenuSubAdvanced::Keydown(int32 key) {
    switch (key) {
        case K_ESCAPE:
            //leave the menu.
            DKM_EnterMain();
            return true;
    }

    return false;
}

bool CMenuSubAdvanced::MousePos(int32 norm_x, int32 norm_y) {
    return false;
}

bool CMenuSubAdvanced::MouseDown(int32 norm_x, int32 norm_y) {
    return false;
}

bool CMenuSubAdvanced::MouseUp(int32 norm_x, int32 norm_y) {
    return false;
}

