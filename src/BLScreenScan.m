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

// ReplayKit capture is deliberately long-lived. Repeated start/stop every 1.8s
// was vulnerable to asynchronous stop/start races after a minute of gameplay.
static BLScreenCaptureCompletion sPendingFrame;
static BOOL sStreamStarted=NO;
static BOOL sStreamStarting=NO;
static BOOL sStreamStopping=NO;
static NSUInteger sFrameTicket=0;

static NSError *BLError(NSInteger code, NSString *reason) {
    return [NSError errorWithDomain:@"BlockBlastAI" code:code
        userInfo:@{NSLocalizedDescriptionKey:reason}];
}
static void BLDeliverPending(UIImage *image, NSError *error) {
    RPScreenRecorder *rec=[RPScreenRecorder sharedRecorder];
    BLScreenCaptureCompletion handler=nil;
    @synchronized(rec) {
        handler=sPendingFrame;
        sPendingFrame=nil;
    }
    if(handler) dispatch_async(dispatch_get_main_queue(), ^{handler(image,error);});
}
static void BLStartStream(void) {
    if(sStreamStarting||sStreamStarted||sStreamStopping) return;
    RPScreenRecorder *rec=[RPScreenRecorder sharedRecorder];
    if(!rec.available) {
        BLDeliverPending(nil,BLError(1,@"ReplayKit video is unavailable."));
        return;
    }
    sStreamStarting=YES;
    [rec startCaptureWithHandler:^(CMSampleBufferRef buffer, RPSampleBufferType type, NSError *error) {
        if(type!=RPSampleBufferTypeVideo) return;
        BLScreenCaptureCompletion pending=nil;
        @synchronized(rec) {
            // Claim exactly one requested frame; never convert all 30/60 fps.
            pending=sPendingFrame;
            sPendingFrame=nil;
        }
        if(!pending) return;
        if(error) {
            dispatch_async(dispatch_get_main_queue(), ^{pending(nil,error);});
            return;
        }
        UIImage *image=nil;
        CVImageBufferRef pixelBuffer=CMSampleBufferGetImageBuffer(buffer);
        if(pixelBuffer) {
            CIImage *ci=[CIImage imageWithCVPixelBuffer:pixelBuffer];
            CIContext *context=[CIContext contextWithOptions:nil];
            CGImageRef cg=[context createCGImage:ci fromRect:ci.extent];
            if(cg) {
                image=[UIImage imageWithCGImage:cg];
                CGImageRelease(cg);
            }
        }
        UIImage *result=image;
        dispatch_async(dispatch_get_main_queue(), ^{
            pending(result,result ? nil : BLError(2,@"Video frame was empty."));
        });
    } completionHandler:^(NSError *error) {
        dispatch_async(dispatch_get_main_queue(), ^{
            sStreamStarting=NO;
            sStreamStarted=(error==nil);
            if(error) BLDeliverPending(nil,error);
        });
    }];
}
void BLCaptureScreen(BLScreenCaptureCompletion completion) {
    if(![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), ^{BLCaptureScreen(completion);});
        return;
    }
    if(!completion)return;
    RPScreenRecorder *rec=[RPScreenRecorder sharedRecorder];
    if(sStreamStopping) {
        completion(nil,BLError(4,@"Screen capture is restarting."));
        return;
    }
    BOOL busy=NO;
    @synchronized(rec) {
        if(sPendingFrame) busy=YES;
        else sPendingFrame=[completion copy];
    }
    if(busy) {
        completion(nil,BLError(5,@"Previous video request still in progress."));
        return;
    }
    NSUInteger ticket=++sFrameTicket;
    BLStartStream();
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(6*NSEC_PER_SEC)),
                   dispatch_get_main_queue(),^{
        if(ticket!=sFrameTicket)return;
        BLDeliverPending(nil,BLError(3,@"No video frame received within 6 seconds."));
        // Keep the stream alive for the next request. Do NOT repeatedly
        // start and stop it on every one-frame timeout.
    });
}
void BLStopScreenCapture(void) {
    if(![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), ^{BLStopScreenCapture();});
        return;
    }
    RPScreenRecorder *rec=[RPScreenRecorder sharedRecorder];
    ++sFrameTicket;
    BLDeliverPending(nil,BLError(6,@"Capture paused."));
    if(sStreamStopping)return;
    if(!sStreamStarted&&!sStreamStarting&&!rec.isRecording)return;
    sStreamStopping=YES;
    [rec stopCaptureWithHandler:^(NSError *error) {
        dispatch_async(dispatch_get_main_queue(), ^{
            sStreamStarted=NO;
            sStreamStarting=NO;
            sStreamStopping=NO;
        });
    }];
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
