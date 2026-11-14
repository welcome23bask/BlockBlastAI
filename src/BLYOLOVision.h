#ifndef BL_YOLO_VISION_H
#define BL_YOLO_VISION_H
#import <UIKit/UIKit.h>
#import "BlockBlastSolver.h"
NS_ASSUME_NONNULL_BEGIN
// Optional, custom-trained YOLOv8n object detector exported through Core ML.
// It DOES NOT use COCO weights or claim to contain the website's private YOLO.
// The expected model is a compiled "BlockBlastYOLOv8n.mlmodelc" with object
// classes "board", "occupied_cell", "tray_cell", and an NMS detection head.
// Model must be in the host app bundle or alongside BlockBlastAI.dylib.
// No image is uploaded; returns NO whenever geometry or confidence is unsafe.
BOOL BLYOLOModelAvailable(void);
BOOL BLYOLOTryDetect(UIImage *image, CGSize screenSize, BOOL occupied[64],
                     BLPiece pieces[3], CGPoint * _Nullable firstCenter,
                     CGPoint * _Nullable lastCenter, NSString * _Nullable * _Nullable detail);
NS_ASSUME_NONNULL_END
#endif
