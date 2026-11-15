// MIT-licensed AutoTouch API adapted for BlockBlastAI.
#import <UIKit/UIKit.h>
NS_ASSUME_NONNULL_BEGIN
@interface BLInProcessTouch : NSObject
// Capability tested against this iOS runtime; cannot guarantee game behavior.
+ (BOOL)isAvailable;
+ (NSInteger)touchAt:(CGPoint)point phase:(UITouchPhase)phase slot:(NSInteger)slot;
+ (BOOL)tap:(CGPoint)point;
@end
NS_ASSUME_NONNULL_END
