// Third-party MIT-licensed BLInProcessTouch implementation from
// https://github.com/Aethereux/AutoTouch (README states MIT).
// Adapted with capability detection and runtime IOKit lookup; never
// synthesize a touch if the required private symbols/selectors are absent.
#import "BLInProcessTouch.h"
#import "BLTouchRoute.h"
#import <dlfcn.h>
#import <objc/message.h>
#import <mach/mach_time.h>
#import <math.h>

// IOHIDEventRef has no public header on iOS. Older SDKs leaked it through
// UIKit; recent ones don't. Forward-declare as an opaque CF type so the
// private setters below compile.
typedef struct __IOHIDEvent *IOHIDEventRef;


// Private UIKit setters. These have been stable since iOS 9 — if Apple
// removes them, every synthetic-touch lib in existence breaks at once.
@interface UITouch ()
- (void)setWindow:(UIWindow *)w;
- (void)setView:(UIView *)v;
- (void)setPhase:(UITouchPhase)p;
- (void)setTimestamp:(NSTimeInterval)t;
- (void)setGestureView:(UIView *)v;
- (void)setIsTap:(BOOL)b;
- (void)_setIsTapToClick:(BOOL)b;
- (void)_setIsFirstTouchForView:(BOOL)b;
- (void)_setLocationInWindow:(CGPoint)p resetPrevious:(BOOL)r;
- (void)_setHidEvent:(IOHIDEventRef)e;
@end

@interface UIEvent ()
- (void)_clearTouches;
- (void)_setHIDEvent:(IOHIDEventRef)e;
- (void)_addTouch:(UITouch *)t forDelayedDelivery:(BOOL)d;
@end

@interface UIApplication ()
- (UIEvent *)_touchesEvent;
@end


// IOKit's digitiser SPI. Field IDs encode (eventType << 16) | offset; the
// digitiser eventType is 11. The one field we actually need is
// IsDisplayIntegrated — without it, gesture recognisers treat the touch as
// coming from a stylus and finger-only handlers skip it.
typedef UInt32   IOOptionBits;
typedef uint32_t IOHIDDigitizerTransducerType;
typedef uint32_t IOHIDEventField;
#ifdef __LP64__
typedef double IOHIDFloat;
#else
typedef float  IOHIDFloat;
#endif

#define kIOHIDEventFieldDigitizerIsDisplayIntegrated ((11 << 16) | 25)

enum { kIOHIDDigitizerTransducerTypeHand = 3 };
enum {
    kIOHIDDigitizerEventRange    = 0x00000001,
    kIOHIDDigitizerEventTouch    = 0x00000002,
    kIOHIDDigitizerEventPosition = 0x00000004,
};


typedef IOHIDEventRef (*BLHandFn)(CFAllocatorRef,AbsoluteTime,
    IOHIDDigitizerTransducerType,uint32_t,uint32_t,uint32_t,uint32_t,
    IOHIDFloat,IOHIDFloat,IOHIDFloat,IOHIDFloat,IOHIDFloat,
    Boolean,Boolean,IOOptionBits);
typedef IOHIDEventRef (*BLFingerFn)(CFAllocatorRef,AbsoluteTime,
    uint32_t,uint32_t,uint32_t,IOHIDFloat,IOHIDFloat,IOHIDFloat,
    IOHIDFloat,IOHIDFloat,IOHIDFloat,IOHIDFloat,IOHIDFloat,
    IOHIDFloat,IOHIDFloat,Boolean,Boolean,IOOptionBits);
typedef void (*BLAppendFn)(IOHIDEventRef,IOHIDEventRef);
typedef void (*BLSetIntFn)(IOHIDEventRef,IOHIDEventField,int);
static BLHandFn blHand;
static BLFingerFn blFinger;
static BLAppendFn blAppend;
static BLSetIntFn blSetInt;
static BOOL blTried=NO;
static BOOL BLResolveHID(void) {
    if(!blTried) {
        blTried=YES;
        void *handle=dlopen("/System/Library/Frameworks/IOKit.framework/IOKit",
                            RTLD_NOW|RTLD_LOCAL);
        if(!handle)handle=dlopen(
            "/System/Library/PrivateFrameworks/IOKit.framework/IOKit",
            RTLD_NOW|RTLD_LOCAL);
        if(!handle)handle=RTLD_DEFAULT;
        blHand=(BLHandFn)dlsym(handle,"IOHIDEventCreateDigitizerEvent");
        blFinger=(BLFingerFn)dlsym(handle,
            "IOHIDEventCreateDigitizerFingerEventWithQuality");
        blAppend=(BLAppendFn)dlsym(handle,"IOHIDEventAppendEvent");
        blSetInt=(BLSetIntFn)dlsym(handle,"IOHIDEventSetIntegerValue");
    }
    return blHand && blFinger && blAppend && blSetInt;
}
static IOHIDEventRef IOHIDEventCreateDigitizerEvent(
    CFAllocatorRef a,AbsoluteTime ts,IOHIDDigitizerTransducerType type,
    uint32_t index,uint32_t identity,uint32_t eventMask,uint32_t buttonMask,
    IOHIDFloat x,IOHIDFloat y,IOHIDFloat z,IOHIDFloat tip,
    IOHIDFloat barrel,Boolean range,Boolean touch,IOOptionBits options){
    return BLResolveHID()?blHand(a,ts,type,index,identity,eventMask,buttonMask,
                                x,y,z,tip,barrel,range,touch,options):NULL;
}
static IOHIDEventRef IOHIDEventCreateDigitizerFingerEventWithQuality(
    CFAllocatorRef a,AbsoluteTime ts,uint32_t index,uint32_t identity,
    uint32_t mask,IOHIDFloat x,IOHIDFloat y,IOHIDFloat z,IOHIDFloat tip,
    IOHIDFloat twist,IOHIDFloat minorR,IOHIDFloat majorR,
    IOHIDFloat quality,IOHIDFloat density,IOHIDFloat irregularity,
    Boolean range,Boolean touch,IOOptionBits options){
    return BLResolveHID()?blFinger(a,ts,index,identity,mask,x,y,z,tip,twist,
        minorR,majorR,quality,density,irregularity,range,touch,options):NULL;
}
static void IOHIDEventAppendEvent(IOHIDEventRef a,IOHIDEventRef b){
    if(blAppend&&a&&b)blAppend(a,b);
}
static void IOHIDEventSetIntegerValue(IOHIDEventRef a,IOHIDEventField f,int v){
    if(blSetInt&&a)blSetInt(a,f,v);
}



// 8 slots. The per-flush scan is 8 int compares, so don't grow it casually.
// Taps use one slot, drags hold one across multiple flushes; serious
// multi-touch rarely needs more than four. Ended touches stay in their slot
// for one extra flush so the liftoff actually dispatches, then get recycled.
enum { kSlotCount = 8 };
static UITouch * _Nullable g_slots[kSlotCount];
// Own ONLY the synthetic finger created by this library. Never cancel a
// human tap or a touch generated by another overlay.
static NSInteger g_providerSlot=0;
static CGPoint g_providerLastPoint={0,0};
static BOOL g_providerActive=NO;


// -keyWindow is deprecated on iOS 13+ and returns nil during scene
// transitions. Prefer the scene API; fall back to -keyWindow when there's
// no foreground scene yet (very early in launch).
static UIWindow * _Nullable ResolveTouchWindow(void) {
    UIApplication *app = UIApplication.sharedApplication;
    if (@available(iOS 13.0, *)) {
        for(UIScene *scene in app.connectedScenes){
            if(scene.activationState!=UISceneActivationStateForegroundActive ||
               ![scene isKindOfClass:[UIWindowScene class]])continue;
            UIWindowScene *ws=(UIWindowScene *)scene;
            UIWindow *candidate=nil;
            for(UIWindow *w in ws.windows){
                // Never target BlockBlastAI/iGameGod's high-level overlay:
                // dragging on a transparent overlay can silently miss Unity.
                if(w.hidden || w.alpha<0.05 ||
                   w.windowLevel>=UIWindowLevelAlert)continue;
                if(!candidate || w.isKeyWindow)candidate=w;
            }
            if(candidate)return candidate;
        }
    }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    UIWindow *key=app.keyWindow;
#pragma clang diagnostic pop
    return key && key.windowLevel<UIWindowLevelAlert ? key : nil;
}


// Builds one "hand" event with a "finger" child for each touch. Caller
// owns the result and must CFRelease it.
static IOHIDEventRef BuildHIDEvent(NSArray<UITouch *> *touches) {
    uint64_t mt = mach_absolute_time();
    AbsoluteTime ts; ts.hi = (UInt32)(mt >> 32); ts.lo = (UInt32)mt;

    IOHIDEventRef hand = IOHIDEventCreateDigitizerEvent(
        kCFAllocatorDefault, ts,
        kIOHIDDigitizerTransducerTypeHand,
        /*index*/ 0, /*identity*/ 0,
        kIOHIDDigitizerEventTouch,
        /*buttonMask*/ 0,
        /*x*/ 0, /*y*/ 0, /*z*/ 0,
        /*tip*/ 0, /*barrel*/ 0,
        /*range*/ false, /*touch*/ true,
        /*options*/ 0);
    IOHIDEventSetIntegerValue(hand, kIOHIDEventFieldDigitizerIsDisplayIntegrated, 1);

    uint32_t idx = 1;
    for (UITouch *t in touches) {
        UITouchPhase phase = t.phase;
        // Began/Ended carry Range+Touch (finger landed or lifted). Moved
        // carries Position only.
        uint32_t mask = (phase == UITouchPhaseMoved)
                        ? kIOHIDDigitizerEventPosition
                        : (kIOHIDDigitizerEventRange | kIOHIDDigitizerEventTouch);
        BOOL touching = (phase != UITouchPhaseEnded && phase != UITouchPhaseCancelled);
        CGPoint p = [t locationInView:t.window];

        IOHIDEventRef finger = IOHIDEventCreateDigitizerFingerEventWithQuality(
            kCFAllocatorDefault, ts,
            /*index*/    idx,
            /*identity*/ 2,
            /*mask*/     mask,
            /*x*/        (IOHIDFloat)p.x,
            /*y*/        (IOHIDFloat)p.y,
            /*z*/        0.0,
            /*tip*/      0,
            /*twist*/    0,
            /*minorR*/   5.0,
            /*majorR*/   5.0,
            /*quality*/  1.0,
            /*density*/  1.0,
            /*irreg*/    1.0,
            /*range*/    (IOHIDFloat)touching,
            /*touch*/    (IOHIDFloat)touching,
            /*options*/  0);
        IOHIDEventSetIntegerValue(finger, kIOHIDEventFieldDigitizerIsDisplayIntegrated, 1);
        IOHIDEventAppendEvent(hand, finger);
        CFRelease(finger);
        idx++;
    }
    return hand;
}


static NSInteger FindFreeSlot(void) {
    for (int i = 0; i < kSlotCount; i++) {
        UITouch *t = g_slots[i];
        if (!t) return i;
        UITouchPhase p = t.phase;
        if (p == UITouchPhaseEnded || p == UITouchPhaseCancelled) return i;
    }
    return -1;
}


// Same shape as KIF's -initAtPoint:inWindow:. The keyboard-window branch
// PTFakeMetaTouch had is gone — it read an uninitialised stack variable
// and would crash if you ever tapped near a UIRemoteKeyboardWindow.
static UITouch *MakeBeganTouch(CGPoint point, UIWindow *win) {
    UITouch *t = [[UITouch alloc] init];
    [t setWindow:win];                              // first — wipes internal state
    [t _setLocationInWindow:point resetPrevious:YES];
    UIView *hit = [win hitTest:point withEvent:nil];
    [t setView:hit];
    [t setPhase:UITouchPhaseBegan];
    [t setTimestamp:NSProcessInfo.processInfo.systemUptime];
    if (@available(iOS 14.0, *)) {
        if([t respondsToSelector:@selector(_setIsTapToClick:)])
            [t _setIsTapToClick:NO];
    } else {
        if([t respondsToSelector:@selector(_setIsFirstTouchForView:)])
            [t _setIsFirstTouchForView:YES];
        if([t respondsToSelector:@selector(setIsTap:)])
            [t setIsTap:NO];
    }
    if ([t respondsToSelector:@selector(setGestureView:)]) {
        [t setGestureView:hit];
    }
    // iOS 9+ requires every UITouch to carry a backing HID event before
    // gesture recognisers will accept it. Attach one here; Flush attaches
    // a separate one to the wrapping UIEvent.
    IOHIDEventRef hid = BuildHIDEvent(@[t]);
    [t _setHidEvent:hid];
    CFRelease(hid);
    return t;
}


// Collect everything alive (and the just-changed slot, which might be
// Ended), wrap it in UIKit's recycled _touchesEvent, send. Then transition
// Began/Moved to Stationary so the next flush carries them as continuing
// fingers — not as re-fired Begans.
static void Flush(NSInteger changedSlot) {
    NSMutableArray<UITouch *> *touches = [NSMutableArray arrayWithCapacity:kSlotCount];
    for (int i = 0; i < kSlotCount; i++) {
        UITouch *t = g_slots[i];
        if (!t) continue;
        UITouchPhase p = t.phase;
        BOOL alive = (p != UITouchPhaseEnded && p != UITouchPhaseCancelled);
        if (alive || i == changedSlot) {
            [touches addObject:t];
        }
    }
    if (touches.count == 0) return;

    UIApplication *app = UIApplication.sharedApplication;
    UIEvent *event = [app _touchesEvent];
    if (!event) return;

    [event _clearTouches];
    IOHIDEventRef hid = BuildHIDEvent(touches);
    [event _setHIDEvent:hid];
    CFRelease(hid);
    for (UITouch *t in touches) {
        [event _addTouch:t forDelayedDelivery:NO];
    }

    [app sendEvent:event];

    NSTimeInterval now = NSProcessInfo.processInfo.systemUptime;
    for (UITouch *t in touches) {
        UITouchPhase p = t.phase;
        if (p == UITouchPhaseBegan || p == UITouchPhaseMoved) {
            [t setPhase:UITouchPhaseStationary];
            [t setTimestamp:now];
        }
        // Ended/Cancelled stay put. FindFreeSlot will reclaim them next time.
    }
}


// Some iGameGod/tweak installations already contain PTFakeMetaTouch.
// Use its documented API only when its class is actually loaded; never
// assume it is present just because iGameGod has a Touch Recorder UI.
static Class BLExistingFakeTouchProvider(void) {
    for(NSString *name in @[@"PTFakeMetaTouch",@"PTFakeTouch"]) {
        Class cls=NSClassFromString(name);
        if(cls &&
           [cls respondsToSelector:NSSelectorFromString(@"getAvailablePointId")] &&
           [cls respondsToSelector:
             NSSelectorFromString(@"fakeTouchId:AtPoint:withTouchPhase:")])
            return cls;
    }
    return Nil;
}
static NSInteger BLProviderTouch(Class cls,NSInteger slot,CGPoint point,
                                 UITouchPhase phase) {
    if(!cls)return 0;
    SEL sel=NSSelectorFromString(@"fakeTouchId:AtPoint:withTouchPhase:");
    NSInteger (*invoke)(id,SEL,NSInteger,CGPoint,UITouchPhase)=
         (NSInteger (*)(id,SEL,NSInteger,CGPoint,UITouchPhase))objc_msgSend;
    return invoke(cls,sel,slot,point,phase);
}
static NSInteger BLProviderFreeSlot(Class cls) {
    NSInteger (*invoke)(id,SEL)=
        (NSInteger (*)(id,SEL))objc_msgSend;
    return cls?invoke(cls,NSSelectorFromString(@"getAvailablePointId")):0;
}
// Compatibility path for newer UIKit builds without the recycled
// UIEvent _addTouch:forDelayedDelivery: selector. Dispatch directly to the
// hit-tested GAME view, not BlockBlastAI's overlay. This is a best-effort,
// in-process UIResponder delivery, not a system-wide injected HID event.
// The overlay MUST verify the actual board changed after any attempt.
static BOOL BLCanDeliverDirectToGame(void) {
    if(!ResolveTouchWindow() || !BLResolveHID())return NO;
    // This build's MakeBeganTouch still attaches a real backing HID event;
    // require that private setter even though delivery itself is direct.
    if(![UITouch instancesRespondToSelector:@selector(_setHidEvent:)])
        return NO;
    Class touch=[UITouch class];
    return [touch instancesRespondToSelector:@selector(setWindow:)] &&
           [touch instancesRespondToSelector:@selector(setView:)] &&
           [touch instancesRespondToSelector:@selector(setPhase:)] &&
           [touch instancesRespondToSelector:@selector(setTimestamp:)] &&
           [touch instancesRespondToSelector:
              @selector(_setLocationInWindow:resetPrevious:)];
}
static BOOL BLDispatchDirectGameView(NSInteger idx) {
    if(idx<0||idx>=kSlotCount)return NO;
    UITouch *touch=g_slots[idx];
    if(!touch)return NO;
    UIWindow *game=ResolveTouchWindow();
    UIView *view=touch.view;
    // Fail closed: no detached views, overlay controls, or different windows.
    if(!game||!view||touch.window!=game||view.window!=game ||
       [view isKindOfClass:[UIControl class]])return NO;
    UITouchPhase phase=touch.phase;
    NSSet<UITouch *> *one=[NSSet setWithObject:touch];
    // No synthetic UIEvent is available. Some game engines accept the
    // responder calls; others require UIKit's native event pipeline.
    // Calls do NOT establish that a game processed the drag.
    @try {
        switch(phase) {
            case UITouchPhaseBegan:
                [view touchesBegan:one withEvent:nil];
                break;
            case UITouchPhaseMoved:
                [view touchesMoved:one withEvent:nil];
                break;
            case UITouchPhaseEnded:
                [view touchesEnded:one withEvent:nil];
                break;
            case UITouchPhaseCancelled:
                [view touchesCancelled:one withEvent:nil];
                break;
            default:return NO;
        }
        if(phase==UITouchPhaseBegan || phase==UITouchPhaseMoved){
            [touch setPhase:UITouchPhaseStationary];
            [touch setTimestamp:NSProcessInfo.processInfo.systemUptime];
        }
        return YES;
    } @catch(__unused NSException *e) {
        return NO;
    }
}
static NSString *BLHIDMissingPart(void){
    if(!BLResolveHID())return @"HID framework/symbols unavailable";
    if(![UITouch instancesRespondToSelector:@selector(_setLocationInWindow:resetPrevious:)])
        return @"UITouch location setter unavailable";
    if(![UITouch instancesRespondToSelector:@selector(_setHidEvent:)])
        return @"UITouch HID setter unavailable";
    if(![UIEvent instancesRespondToSelector:@selector(_setHIDEvent:)])
        return @"UIEvent HID setter unavailable";
    if(![UIEvent instancesRespondToSelector:@selector(_addTouch:forDelayedDelivery:)])
        return @"UIEvent add-touch selector unavailable";
    if(![[UIApplication sharedApplication] respondsToSelector:@selector(_touchesEvent)])
        return @"UIApplication touches-event selector unavailable";
    if(!ResolveTouchWindow())return @"no foreground game window";
    return nil;
}
static BLTouchRoute BLRuntimeTouchRoute(void) {
    return BLSelectTouchRoute(BLExistingFakeTouchProvider()!=Nil,
                             BLHIDMissingPart()==nil,
                             BLCanDeliverDirectToGame(),
                             [UIEvent instancesRespondToSelector:
                               @selector(_addTouch:forDelayedDelivery:)]);
}
@implementation BLInProcessTouch

+ (BOOL)isAvailable {
    return BLRuntimeTouchRoute()!=BLTouchRouteNone;
}
+ (NSString *)capabilityReport {
    Class provider=BLExistingFakeTouchProvider();
    if(provider)return [NSString stringWithFormat:
        @"Touch provider detected: %@ (game drag still unverified)",
        NSStringFromClass(provider)];
    NSString *missing=BLHIDMissingPart();
    if(!missing)
        return @"HID touch method available; actual game acceptance unverified.";
    if(BLCanDeliverDirectToGame() &&
       ![UIEvent instancesRespondToSelector:@selector(_addTouch:forDelayedDelivery:)])
        return [NSString stringWithFormat:
            @"UIKit method missing (%@). Testing direct game-view touches; first real drag unverified.",
            missing];
    return [NSString stringWithFormat:
        @"Automatic unavailable: %@; direct game-view fallback unsupported.",
        missing];
}

+ (NSInteger)touchAt:(CGPoint)point
               phase:(UITouchPhase)phase
                slot:(NSInteger)slot {
    if(!isfinite(point.x)||!isfinite(point.y))return 0;
    Class provider=BLExistingFakeTouchProvider();
    if(provider) {
        NSInteger providerSlot=slot;
        if(phase==UITouchPhaseBegan) {
            if(g_providerActive)return 0; // no overlapping automation
            providerSlot=BLProviderFreeSlot(provider);
        }
        if(providerSlot<=0)return 0;
        if(phase!=UITouchPhaseBegan &&
           (!g_providerActive || providerSlot!=g_providerSlot))return 0;
        NSInteger result=BLProviderTouch(provider,providerSlot,point,phase);
        if(result>0) {
            g_providerLastPoint=point;
            if(phase==UITouchPhaseBegan){
                g_providerSlot=providerSlot;g_providerActive=YES;
            } else if(phase==UITouchPhaseEnded ||
                      phase==UITouchPhaseCancelled){
                g_providerSlot=0;g_providerActive=NO;
            }
        }
        return result;
    }
    BLTouchRoute route=BLRuntimeTouchRoute();
    if(route!=BLTouchRouteDirectGameView &&
       route!=BLTouchRouteHIDEvent)return 0;
    BOOL direct=route==BLTouchRouteDirectGameView;
    UIWindow *win = ResolveTouchWindow();
    if(!win)return 0;
    // Scanner coordinates live in UIScreen space. UIKit UITouch's
    // _locationInWindow and -hitTest: require WINDOW-local coordinates.
    CGPoint local=[win convertPoint:point
                 fromCoordinateSpace:UIScreen.mainScreen.coordinateSpace];
    if(!isfinite(local.x)||!isfinite(local.y) ||
       !CGRectContainsPoint(CGRectInset(win.bounds,8,8),local))return 0;

    NSInteger idx = -1;
    if (phase == UITouchPhaseBegan) {
        // Began always allocates fresh — caller's slot is ignored.
        idx = FindFreeSlot();
        if (idx < 0) return 0;
        g_slots[idx] = MakeBeganTouch(local, win);
        if(!g_slots[idx] || !g_slots[idx].view ||
           g_slots[idx].view.window!=win) {
            g_slots[idx]=nil;
            return 0;
        }
    } else {
        if (slot < 1 || slot > kSlotCount) return 0;
        idx = slot - 1;
        UITouch *t = g_slots[idx];
        if (!t) return 0;
        // Send each move/end in the ORIGINAL window's coordinate space.
        // If the scene/window changed mid-gesture, cancel rather than drag
        // unpredictably into the edge of the screen.
        if(t.window!=win)return 0;
        [t _setLocationInWindow:local resetPrevious:NO];
        [t setTimestamp:NSProcessInfo.processInfo.systemUptime];
        [t setPhase:phase];
    }

    if(direct) {
        if(!BLDispatchDirectGameView(idx)) {
            // Some engines reject a move phase without a UIEvent.
            // Never abandon a Began touch without a best-effort Cancelled.
            UITouch *touch=g_slots[idx];
            if(touch && touch.view) {
                [touch setPhase:UITouchPhaseCancelled];
                [touch setTimestamp:NSProcessInfo.processInfo.systemUptime];
                @try {
                    [touch.view touchesCancelled:
                        [NSSet setWithObject:touch] withEvent:nil];
                } @catch(__unused NSException *e) {}
            }
            g_slots[idx]=nil;
            return 0;
        }
    }else{
        Flush(idx);
    }
    return idx + 1;
}

+ (BOOL)tap:(CGPoint)point {
    NSInteger s = [self touchAt:point phase:UITouchPhaseBegan slot:0];
    if(s==0)return NO;
    BOOL delivered=[self touchAt:point phase:UITouchPhaseEnded slot:s]>0;
    if(!delivered)[self forceCancelAllTouches];
    return delivered;
}

+ (void)forceCancelAllTouches {
    if(![NSThread isMainThread]){
        dispatch_async(dispatch_get_main_queue(), ^{
            [BLInProcessTouch forceCancelAllTouches];
        });
        return;
    }
    Class provider=BLExistingFakeTouchProvider();
    if(provider && g_providerActive && g_providerSlot>0){
        (void)BLProviderTouch(provider,g_providerSlot,
                              g_providerLastPoint,UITouchPhaseCancelled);
        g_providerActive=NO;g_providerSlot=0;
    }
    BLTouchRoute route=BLRuntimeTouchRoute();
    for(NSInteger i=0;i<kSlotCount;i++) {
        UITouch *touch=g_slots[i];
        if(!touch)continue;
        // Even after a nominal Ended phase, explicitly cancel the same
        // synthetic identity once when the user reports a stuck finger.
        [touch setPhase:UITouchPhaseCancelled];
        [touch setTimestamp:NSProcessInfo.processInfo.systemUptime];
        if(route==BLTouchRouteDirectGameView){
            (void)BLDispatchDirectGameView(i);
        }else if(route==BLTouchRouteHIDEvent){
            Flush(i);
        }else if(touch.view){
            @try {
                [touch.view touchesCancelled:[NSSet setWithObject:touch]
                                    withEvent:nil];
            } @catch(__unused NSException *e) {}
        }
        g_slots[i]=nil;
    }
}

@end
