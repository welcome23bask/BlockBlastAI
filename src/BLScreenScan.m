#import "BLScreenScan.h"
#import "BLVisionCore.h"
#import <ReplayKit/ReplayKit.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreImage/CoreImage.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

void BLCaptureScreen(BLScreenCaptureCompletion completion) {
    RPScreenRecorder *rec=[RPScreenRecorder sharedRecorder];
    if (!rec.isAvailable) {
        if (completion) completion(nil,[NSError errorWithDomain:@"BlockBlastAI" code:1
           userInfo:@{NSLocalizedDescriptionKey:@"ReplayKit capture unavailable; enter the board manually."}]);
        return;
    }
    __block BOOL finished=NO;
    [rec startCaptureWithHandler:^(CMSampleBufferRef buffer, RPSampleBufferType type, NSError *error) {
        if(type!=RPSampleBufferTypeVideo) return;
        @synchronized(rec) { if(finished) return; finished=YES; }
        UIImage *image=nil;
        if(!error) {
            CVImageBufferRef pixelBuffer=CMSampleBufferGetImageBuffer(buffer);
            if(pixelBuffer) {
                CIImage *ci=[CIImage imageWithCVPixelBuffer:pixelBuffer];
                CIContext *context=[CIContext contextWithOptions:nil];
                CGImageRef cg=[context createCGImage:ci fromRect:ci.extent];
                if(cg) { image=[UIImage imageWithCGImage:cg]; CGImageRelease(cg); }
            }
        }
        UIImage *result=image;
        dispatch_async(dispatch_get_main_queue(), ^{
            [rec stopCaptureWithHandler:^(NSError * _Nullable stopError) {}];
            if(completion) completion(result,error ?: (result?nil:
               [NSError errorWithDomain:@"BlockBlastAI" code:2
                  userInfo:@{NSLocalizedDescriptionKey:@"Screen capture frame was empty."}]));
        });
    } completionHandler:^(NSError *error) {
        if (!error) return;
        @synchronized(rec) { if(finished) return; finished=YES; }
        dispatch_async(dispatch_get_main_queue(), ^{
            if(completion) completion(nil,error);
        });
    }];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(7*NSEC_PER_SEC)),dispatch_get_main_queue(), ^{
        @synchronized(rec) { if(finished) return; finished=YES; }
        [rec stopCaptureWithHandler:^(NSError * _Nullable stopError) {}];
        if(completion) completion(nil,[NSError errorWithDomain:@"BlockBlastAI" code:3
               userInfo:@{NSLocalizedDescriptionKey:@"Capture timed out. Enter board manually."}]);
    });
}


BOOL BLDetectBoard(UIImage *image, CGPoint firstCenter, CGPoint lastCenter,
                   CGSize screenSize, BOOL occupied[64], NSString **detail) {
    // Manual button uses the same orientation-corrected detector as automatic
    // mode; old brightness clustering was vertically mirrored on this device.
    BLPiece shapes[3]={0};
    CGPoint first=CGPointZero,last=CGPointZero;
    BOOL success=BLAutoDetect(image,screenSize,occupied,shapes,&first,&last,detail);
    (void)firstCenter;(void)lastCenter;
    return success;
}

static BOOL BLDecodeImage(UIImage *image, CGSize screen, uint8_t **out,
                          size_t *outW, size_t *outH) {
    if (!image.CGImage || screen.width<240 || screen.height<350) return NO;
    size_t w=(size_t)lround(screen.width),h=(size_t)lround(screen.height);
    if(w>1600 || h>3000 || w*h>4500000) return NO;
    UIGraphicsBeginImageContextWithOptions(CGSizeMake(w,h),YES,1.0);
    [image drawInRect:CGRectMake(0,0,w,h)];
    UIImage *upright=UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    if(!upright.CGImage) return NO;
    uint8_t *px=calloc(w*h,4);
    if(!px) return NO;
    CGColorSpaceRef cs=CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx=CGBitmapContextCreate(px,w,h,8,w*4,cs,
                                          kCGImageAlphaPremultipliedLast|kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(cs);
    if(!ctx) {free(px);return NO;}
    CGContextTranslateCTM(ctx,0,h);
    CGContextScaleCTM(ctx,1,-1);
    CGContextDrawImage(ctx,CGRectMake(0,0,w,h),upright.CGImage);
    CGContextRelease(ctx);
    *out=px;*outW=w;*outH=h;
    return YES;
}

BOOL BLAutoDetect(UIImage *image, CGSize screenSize, BOOL occupied[64],
                  BLPiece pieces[3], CGPoint *firstCenter, CGPoint *lastCenter,
                  NSString **detail) {
    uint8_t *pixels=NULL;
    size_t w=0,h=0;
    if(!BLDecodeImage(image,screenSize,&pixels,&w,&h)) {
        if(detail)*detail=@"Screen frame could not be decoded.";
        return NO;
    }
    BLVisionResult found={0};
    int okay=BLVisionAnalyzeRGBA(pixels,(int)w,(int)h,&found);
    free(pixels);
    if(!okay) {
        if(detail)*detail=@"Cannot safely identify the whole board and tray. Highlights cleared; wait for gameplay.";
        return NO;
    }
    for(int i=0;i<64;++i)occupied[i]=(found.board & (UINT64_C(1)<<i))!=0;
    memcpy(pieces,found.pieces,sizeof(found.pieces));
    if(firstCenter)*firstCenter=CGPointMake(found.firstX,found.firstY);
    if(lastCenter)*lastCenter=CGPointMake(found.lastX,found.lastY);
    int squares=__builtin_popcountll(found.board);
    if(detail)*detail=[NSString stringWithFormat:@"Detected %d filled squares, %d tray piece(s); orientation %@.",
         squares,found.detectedPieces,found.flippedInput?@"corrected":@"upright"];
    return YES;
}
