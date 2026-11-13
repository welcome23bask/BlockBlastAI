#import "BLScreenScan.h"
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

static double BLScore(const uint8_t *data, size_t w, size_t h, CGFloat x, CGFloat y) {
    int cx=(int)lround(x), cy=(int)lround(y);
    double sum=0; int n=0;
    for (int dy=-3;dy<=3;dy+=3) for (int dx=-3;dx<=3;dx+=3) {
        int px=cx+dx, py=cy+dy;
        if(px<0 || py<0 || px>=(int)w || py>=(int)h) continue;
        const uint8_t *p=data+4*((size_t)py*w+(size_t)px);
        double lo=fmin(p[0],fmin(p[1],p[2]));
        double hi=fmax(p[0],fmax(p[1],p[2]));
        sum+=0.22*p[0]+0.44*p[1]+0.34*p[2]+0.14*(hi-lo);
        ++n;
    }
    return n?sum/n:0;
}

BOOL BLDetectBoard(UIImage *image, CGPoint a, CGPoint b, CGSize screenSize,
                   BOOL occupied[64], NSString **detail) {
    if (!image.CGImage || screenSize.width<=0 || screenSize.height<=0 ||
        (b.x-a.x)<70 || (b.y-a.y)<70) {
        if(detail) *detail=@"Calibrate the first and last cell centers before scanning.";
        return NO;
    }
    // Decode a small, upright bitmap in UIKit point coordinates to simplify orientation.
    size_t w=(size_t)lround(screenSize.width),h=(size_t)lround(screenSize.height);
    if (w<100 || h<100 || w>3000 || h>4000) return NO;
    UIGraphicsBeginImageContextWithOptions(CGSizeMake(w,h),YES,1.0);
    [image drawInRect:CGRectMake(0,0,w,h)];
    UIImage *upright=UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    if(!upright.CGImage) return NO;
    uint8_t *pixels=calloc(w*h,4);
    if(!pixels) return NO;
    CGColorSpaceRef cs=CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx=CGBitmapContextCreate(pixels,w,h,8,w*4,cs,
                                           kCGImageAlphaPremultipliedLast|kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(cs);
    if(!ctx) { free(pixels);return NO; }
    CGContextTranslateCTM(ctx,0,h);
    CGContextScaleCTM(ctx,1,-1);
    CGContextDrawImage(ctx,CGRectMake(0,0,w,h),upright.CGImage);
    CGContextRelease(ctx);
    double scores[64],low=1e9,high=-1e9;
    for(int r=0;r<8;++r) for(int c=0;c<8;++c) {
        CGFloat x=a.x+(b.x-a.x)*c/7.0,y=a.y+(b.y-a.y)*r/7.0;
        if(x<3 || x>w-4 || y<3 || y>h-4) {
            free(pixels);
            if(detail) *detail=@"Board calibration falls outside the screenshot.";
            return NO;
        }
        double value=BLScore(pixels,w,h,x,y);
        scores[r*8+c]=value;
        low=fmin(low,value);high=fmax(high,value);
    }
    free(pixels);
    // Two-cluster brightness separation. Empty boards have no meaningful bright cluster.
    if(high-low<27) {
        if(detail) *detail=@"Grid cells look too similar to classify confidently. Board left unchanged.";
        return NO;
    }
    double c0=low,c1=high;
    for(int it=0;it<12;++it) {
        double s0=0,s1=0;int n0=0,n1=0;
        for(int i=0;i<64;++i) {
            if(fabs(scores[i]-c0)<=fabs(scores[i]-c1)){s0+=scores[i];++n0;}
            else{s1+=scores[i];++n1;}
        }
        if(!n0||!n1) break;
        c0=s0/n0;c1=s1/n1;
    }
    double threshold=(c0+c1)/2;
    if(fabs(c1-c0)<24) {
        if(detail) *detail=@"Could not distinguish occupied and empty cells; try rescanning.";
        return NO;
    }
    int count=0;
    for(int i=0;i<64;++i){ occupied[i]=scores[i]>threshold; count+=occupied[i]; }
    if(detail) *detail=[NSString stringWithFormat:@"Detected %d occupied cells (beta). Verify the grid and correct any mistakes.",count];
    return YES;
}
