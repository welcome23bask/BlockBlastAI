#ifndef BL_TOUCH_ROUTE_H
#define BL_TOUCH_ROUTE_H
// Pure decision policy. A route being available is NOT proof that a game
// accepts the generated touch. On-device post-drag verification is mandatory.
typedef enum {
    BLTouchRouteNone=0,
    BLTouchRouteProvider=1,
    BLTouchRouteHIDEvent=2,
    BLTouchRouteDirectGameView=3
} BLTouchRoute;
BLTouchRoute BLSelectTouchRoute(int hasLoadedProvider,int fullHIDRoute,
                               int hasGameViewFallback,int hasUIEventAddTouch);
#endif
