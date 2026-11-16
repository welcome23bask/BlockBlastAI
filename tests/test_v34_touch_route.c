#include "../src/BLTouchRoute.h"
#include <assert.h>
#include <stdio.h>
int main(void){
    // User's reported iOS failure: missing _addTouch but valid game view.
    assert(BLSelectTouchRoute(0,0,1,0)==BLTouchRouteDirectGameView);
    // Fallback can never override a working injected provider.
    assert(BLSelectTouchRoute(1,0,1,0)==BLTouchRouteProvider);
    assert(BLSelectTouchRoute(0,1,1,1)==BLTouchRouteHIDEvent);
    // No private prerequisites or game view = remain disabled.
    assert(BLSelectTouchRoute(0,0,0,0)==BLTouchRouteNone);
    // Never bypass a DIFFERENT HID-method error by assuming direct works.
    assert(BLSelectTouchRoute(0,0,1,1)==BLTouchRouteNone);
    puts("PASS missing UIEvent add-touch triggers guarded direct-to-game fallback; all other failures closed");
    return 0;
}
