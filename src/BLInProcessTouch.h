// MIT-licensed AutoTouch API adapted for BlockBlastAI.
#import <UIKit/UIKit.h>
NS_ASSUME_NONNULL_BEGIN
@interface BLInProcessTouch : NSObject
// Capability tested against this iOS runtime; cannot guarantee game behavior.
+ (BOOL)isAvailable;
// Describes the first unsupported requirement without attempting a touch.
// Reports iOS ABI support; real drag acceptance must still be confirmed.
+ (NSString *)capabilityReport;
+ (NSInteger)touchAt:(CGPoint)point phase:(UITouchPhase)phase slot:(NSInteger)slot;
+ (BOOL)tap:(CGPoint)point;
// Emergency release of ALL touches started by BlockBlastAI, including
// a best-effort cancel of a touch that was already sent an Ended phase.
// Call on every stop, error, panel opening and application background.
+ (void)forceCancelAllTouches;
@end
NS_ASSUME_NONNULL_END
