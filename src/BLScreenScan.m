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
// All state transitions run on main; video callbacks claim pending requests
// under @synchronized(recorder), avoiding double completion and start/stop races.
static BLScreenCaptureCompletion sPendingFrame;
static BOOL sStreamStarted=NO, sStreamStarting=NO, sStreamStopping=NO;
static BOOL sShouldRun=NO, sRestartAfterStart=NO, sWatchdogArmed=NO;
static unsigned long sGeneration=0, sFrameTicket=0;
static CFAbsoluteTime sLastFrameAt=0;
static CIContext *sSharedCIContext;
// Frame identity: every decoded frame gets a monotonically increasing counter
// and a capture timestamp; restarts bump sRestartCount so callers can drop
// any cached detections.
static unsigned long sFrameCounter=0, sRestartCount=0;
static CFAbsoluteTime sPendingSince=0;

static NSError *BLError(NSInteger code, NSString *reason) {
    return [NSError errorWithDomain:@"BlockBlastAI" code:code
        userInfo:@{NSLocalizedDescriptionKey:reason}];
}
static BOOL BLDeliverPending(UIImage *image, NSError *error) {
    RPScreenRecorder *rec=[RPScreenRecorder sharedRecorder];
    BLScreenCaptureCompletion callback=nil;
    @synchronized(rec) { callback=sPendingFrame; sPendingFrame=nil; }
    if (callback) dispatch_async(dispatch_get_main_queue(), ^{ callback(image,error); });
    return callback!=nil;
}
static void BLStartStream(void);
static void BLRecycleStream(void);

static void BLWatchdog(void) {
    if(!sShouldRun) {sWatchdogArmed=NO;return;}
    RPScreenRecorder *rec=[RPScreenRecorder sharedRecorder];
    CFAbsoluteTime last; BOOL pending;
    @synchronized(rec) {last=MAX(sLastFrameAt,sPendingSince); pending=(sPendingFrame!=nil);}
    // ReplayKit only emits frames when pixels change, so a quiet screen is NOT
    // a stall. Treat it as stalled only while a caller is waiting on a frame
    // (the overlay forces a pixel change before each request).
    if(!sStreamStopping && pending && (sStreamStarted || sStreamStarting) &&
       last>0 && CFAbsoluteTimeGetCurrent()-last>2.5) {
        BLDeliverPending(nil,BLError(7,@"ReplayKit stopped sending video frames. Restarting."));
        BLRecycleStream();
    }
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(1*NSEC_PER_SEC)),
                   dispatch_get_main_queue(), ^{BLWatchdog();});
}
static void BLArmWatchdog(void) {
    if(sWatchdogArmed)return;
    sWatchdogArmed=YES;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(2*NSEC_PER_SEC)),
                   dispatch_get_main_queue(), ^{BLWatchdog();});
}
static void BLRecycleStream(void) {
    RPScreenRecorder *rec=[RPScreenRecorder sharedRecorder];
    if(sStreamStopping)return;
    if(sStreamStarting) {sRestartAfterStart=YES;return;}
    if(!sStreamStarted && !rec.isRecording) {
        sStreamStarted=NO;
        @synchronized(rec) {++sRestartCount;}
        if(sShouldRun) BLStartStream();
        return;
    }
    sStreamStopping=YES;
    @synchronized(rec) {++sGeneration; ++sRestartCount;} // ignore old video callbacks
    [rec stopCaptureWithHandler:^(NSError *error) {
        dispatch_async(dispatch_get_main_queue(), ^{
            sStreamStarted=NO;
            sStreamStarting=NO;
            sStreamStopping=NO;
            sRestartAfterStart=NO;
            if(sShouldRun) BLStartStream();
        });
    }];
}
static void BLStartStream(void) {
    if(!sShouldRun || sStreamStarted || sStreamStarting || sStreamStopping) return;
    RPScreenRecorder *rec=[RPScreenRecorder sharedRecorder];
    if(!rec.available) {
        BLDeliverPending(nil,BLError(1,@"ReplayKit recording unavailable."));
        return;
    }
    static dispatch_once_t ciOnce;
    dispatch_once(&ciOnce, ^{
        sSharedCIContext=[CIContext contextWithOptions:@{kCIContextUseSoftwareRenderer:@NO}];
    });
    sStreamStarting=YES;
    unsigned long generation;
    @synchronized(rec) {
        generation=++sGeneration;
        sLastFrameAt=CFAbsoluteTimeGetCurrent();
    }
    [rec startCaptureWithHandler:^(CMSampleBufferRef buffer, RPSampleBufferType type, NSError *error) {
        if(type!=RPSampleBufferTypeVideo) return;
        BLScreenCaptureCompletion callback=nil;
        @synchronized(rec) {
            if(generation==sGeneration) {
                sLastFrameAt=CFAbsoluteTimeGetCurrent();
                ++sFrameCounter;
                callback=sPendingFrame;
                sPendingFrame=nil;
            }
        }
        if(!callback)return; // do not decode every 60fps video frame
        UIImage *image=nil;
        if(!error && CMSampleBufferIsValid(buffer)) {
            CVImageBufferRef pixelBuffer=CMSampleBufferGetImageBuffer(buffer);
            if(pixelBuffer) {
                CIImage *ci=[CIImage imageWithCVPixelBuffer:pixelBuffer];
                CGImageRef cg=[sSharedCIContext createCGImage:ci fromRect:ci.extent];
                if(cg) { image=[UIImage imageWithCGImage:cg]; CGImageRelease(cg); }
            }
        }
        UIImage *result=image;
        dispatch_async(dispatch_get_main_queue(), ^{
            callback(result,error ?: (result ? nil : BLError(2,@"Video frame was empty.")));
        });
    } completionHandler:^(NSError *error) {
        dispatch_async(dispatch_get_main_queue(), ^{
            if(generation!=sGeneration)return;
            sStreamStarting=NO;
            sStreamStarted=(error==nil);
            if(error) BLDeliverPending(nil,error);
            if(sRestartAfterStart || !sShouldRun) {
                sRestartAfterStart=NO;
                if(sStreamStarted) BLRecycleStream();
                else if(sShouldRun) {
                    // A watchdog restart requested during startup must also
                    // recover if ReplayKit rejects the initial start.
                    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(3*NSEC_PER_SEC)),
                                   dispatch_get_main_queue(),^{BLStartStream();});
                }
            } else if(error && sShouldRun) {
                // Back off to avoid hammering ReplayKit permission errors.
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(3*NSEC_PER_SEC)),
                               dispatch_get_main_queue(),^{BLStartStream();});
            }
        });
    }];
    BLArmWatchdog();
}

unsigned long BLCaptureFrameCounter(void) {
    RPScreenRecorder *rec=[RPScreenRecorder sharedRecorder];
    unsigned long v; @synchronized(rec){v=sFrameCounter;} return v;
}
unsigned long BLCaptureRestartCount(void) {
    RPScreenRecorder *rec=[RPScreenRecorder sharedRecorder];
    unsigned long v; @synchronized(rec){v=sRestartCount;} return v;
}
CFAbsoluteTime BLCaptureLastFrameTime(void) {
    RPScreenRecorder *rec=[RPScreenRecorder sharedRecorder];
    CFAbsoluteTime v; @synchronized(rec){v=sLastFrameAt;} return v;
}

void BLCaptureScreen(BLScreenCaptureCompletion completion) {
    if(![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(),^{BLCaptureScreen(completion);});
        return;
    }
    if(!completion)return;
    RPScreenRecorder *rec=[RPScreenRecorder sharedRecorder];
    BOOL busy=NO;
    @synchronized(rec) {
        if(sPendingFrame)busy=YES;
        else {sPendingFrame=[completion copy]; sPendingSince=CFAbsoluteTimeGetCurrent();}
    }
    if(busy) {
        completion(nil,BLError(5,@"Previous frame request still pending."));
        return;
    }
    sShouldRun=YES;
    unsigned long ticket=++sFrameTicket;
    BLStartStream();
    BLArmWatchdog();
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(3*NSEC_PER_SEC)),
                   dispatch_get_main_queue(), ^{
        if(ticket!=sFrameTicket) return;
        if(BLDeliverPending(nil,BLError(3,@"No video frame received within 3 seconds.")))
            BLRecycleStream();
    });
}
void BLStopScreenCapture(void) {
    if(![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(),^{BLStopScreenCapture();});
        return;
    }
    sShouldRun=NO;
    ++sFrameTicket;
    BLDeliverPending(nil,BLError(6,@"Screen capture paused."));
    BLRecycleStream();
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
    // ReplayKit supplies an already-upright CGImage. Skip the redundant
    // UIKit redraw and go straight to a single scaled pixel decode.
    // Retain the slower normalization fallback for non-upright images.
    UIImage *upright=image;
    if(image.imageOrientation!=UIImageOrientationUp) {
        UIGraphicsBeginImageContextWithOptions(CGSizeMake(w,h),YES,1.0);
        [image drawInRect:CGRectMake(0,0,w,h)];
        upright=UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        if(!upright.CGImage) return NO;
    }
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