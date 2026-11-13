#ifndef BL_SCREEN_SCAN_H
#define BL_SCREEN_SCAN_H
#import <UIKit/UIKit.h>
#import "BlockBlastSolver.h"
NS_ASSUME_NONNULL_BEGIN
typedef void (^BLScreenCaptureCompletion)(UIImage * _Nullable image, NSError * _Nullable error);
void BLCaptureScreen(BLScreenCaptureCompletion completion);
void BLStopScreenCapture(void);
// Board calibration positions are centers of top-left and bottom-right cells in UI points.
// Detector is heuristic; the user must visually verify its result before solving.
BOOL BLDetectBoard(UIImage *image, CGPoint firstCenter, CGPoint lastCenter,
                   CGSize screenSize, BOOL occupied[64], NSString ** _Nullable detail);
// Automatic board-location, board-occupancy and tray-piece recognition.
// Currently tuned to the user's blue-background Block Blast screen layout.
// Returns NO if geometry is uncertain. Shapes are oriented as displayed.
BOOL BLAutoDetect(UIImage *image, CGSize screenSize, BOOL occupied[64],
                  BLPiece pieces[3], CGPoint *firstCenter, CGPoint *lastCenter,
                  NSString ** _Nullable detail);
NS_ASSUME_NONNULL_END
#endif
