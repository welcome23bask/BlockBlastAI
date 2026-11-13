#ifndef BL_SCREEN_SCAN_H
#define BL_SCREEN_SCAN_H
#import <UIKit/UIKit.h>
NS_ASSUME_NONNULL_BEGIN
typedef void (^BLScreenCaptureCompletion)(UIImage * _Nullable image, NSError * _Nullable error);
void BLCaptureScreen(BLScreenCaptureCompletion completion);
// Board calibration positions are centers of top-left and bottom-right cells in UI points.
// Detector is heuristic; the user must visually verify its result before solving.
BOOL BLDetectBoard(UIImage *image, CGPoint firstCenter, CGPoint lastCenter,
                   CGSize screenSize, BOOL occupied[64], NSString ** _Nullable detail);
NS_ASSUME_NONNULL_END
#endif
