#include "BLTouchRoute.h"
BLTouchRoute BLSelectTouchRoute(int provider,int hid,
                               int direct,int hasAddTouch){
    if(provider)return BLTouchRouteProvider;
    if(hid)return BLTouchRouteHIDEvent;
    // An absent UIEvent add-touch selector is specifically the failure
    // reported by the user's iPhone. Try responder delivery only when
    // its own UIKit/HID and visible-game-window checks have passed.
    if(!hasAddTouch && direct)return BLTouchRouteDirectGameView;
    return BLTouchRouteNone;
}
