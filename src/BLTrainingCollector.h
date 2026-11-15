#ifndef BL_TRAINING_COLLECTOR_H
#define BL_TRAINING_COLLECTOR_H
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "BlockBlastSolver.h"

NS_ASSUME_NONNULL_BEGIN
extern NSString * const BLTrainingCollectorDidChangeNotification;
@interface BLTrainingCollector : NSObject
@property (nonatomic, readonly, getter=isEnabled) BOOL enabled;
@property (nonatomic, readonly) NSUInteger savedCount;
@property (nonatomic, readonly) NSUInteger limit;
@property (nonatomic, readonly, copy) NSString *status;
+ (instancetype)shared;
// Explicit per-session opt-in; starts OFF at each launch, never uploads.
- (void)setEnabled:(BOOL)enabled;
// Call only after >=3 independently delivered, matching valid scanner frames,
// with overlays hidden and no modal/panel onscreen.
- (void)observeStableFrame:(UIImage *)image
                    board:(uint64_t)board
                   pieces:(const BLPiece *)pieces
                    first:(CGPoint)first
                     last:(CGPoint)last
               screenSize:(CGSize)screenSize;
// ZIP includes only local board+tray crops and UNREVIEWED proposed JSON labels.
- (void)createArchive:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion;
- (void)deleteAllWithCompletion:(void (^)(NSError * _Nullable error))completion;
@end
NS_ASSUME_NONNULL_END
#endif
