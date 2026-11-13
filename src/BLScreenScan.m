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
    // Avoid mistaking the bright blue sky/background outside the game grid for pieces.
    // In the blue/cyan Block Blast theme, the empty grid is much darker than blocks.
    double threshold=fmax(125.0,(c0+c1)/2);
    if(fabs(c1-c0)<24) {
        if(detail) *detail=@"Could not distinguish occupied and empty cells; try rescanning.";
        return NO;
    }
    int count=0;
    for(int i=0;i<64;++i){ occupied[i]=scores[i]>threshold; count+=occupied[i]; }
    if(detail) *detail=[NSString stringWithFormat:@"Detected %d occupied cells (v0.3 color guard). Check board alignment before using suggestions.",count];
    return YES;
}


#pragma mark - Automatic board and tray recognition (screen-layout v0.4)

// Pixel buffer is RGBA. Classification deliberately samples CELL CENTERS,
// keeping highlights at cell edges from corrupting successive reads.
static inline const uint8_t *BLPixel(const uint8_t *pixels, size_t width, int x, int y) {
    return pixels + ((size_t)y * width + (size_t)x)*4;
}
static inline BOOL BLIsDarkGameCell(const uint8_t *p) {
    return p[0] < 75 && p[1] < 103 && p[2] < 122;
}
static inline BOOL BLIsBrightGameBlock(const uint8_t *p) {
    int hi = MAX(p[0],MAX(p[1],p[2]));
    return hi > 138;
}
static inline BOOL BLIsBrightTrayBlock(const uint8_t *p) {
    int hi = MAX(p[0],MAX(p[1],p[2]));
    int lo = MIN(p[0],MIN(p[1],p[2]));
    return hi > 155 && hi-lo > 35;
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
static BOOL BLLocateBoard(const uint8_t *px,size_t w,size_t h,
                          int *left,int *top,int *right,int *bottom) {
    // The board is a large, near-square dark grid. We look for the continuous
    // band of dark cells, ignoring bright (occupied) cells between dark rows.
    int yFirst=-1,yLast=-1,accepted=0;
    int from=(int)(h*.12),to=(int)(h*.78);
    int x0=(int)(w*.08),x1=(int)(w*.92);
    for(int y=from;y<to;y+=2) {
        int hits=0,n=0;
        for(int x=x0;x<x1;x+=5) {hits+=BLIsDarkGameCell(BLPixel(px,w,x,y));n++;}
        if(n && (double)hits/n>.46) {
            if(yFirst<0)yFirst=y;
            yLast=y;accepted++;
        }
    }
    if(yFirst<0 || accepted<18) return NO;
    int ySize=yLast-yFirst;
    if(ySize<(int)(w*.68) || ySize>(int)(w*1.15)) return NO;
    int xFirst=-1,xLast=-1;
    for(int x=0;x<(int)w;x+=2) {
        int hits=0,n=0;
        for(int y=yFirst;y<yLast;y+=5) {
            hits+=BLIsDarkGameCell(BLPixel(px,w,x,y));++n;
        }
        if(n && (double)hits/n>.45) {
            if(xFirst<0)xFirst=x;
            xLast=x;
        }
    }
    if(xFirst<0) return NO;
    int xSize=xLast-xFirst;
    if(xSize<(int)(w*.67) || xSize>(int)(w*.985)) return NO;
    if(fabs((double)xSize-ySize)>MAX(xSize,ySize)*.13) return NO;
    *left=xFirst;*right=xLast;*top=yFirst;*bottom=yLast;
    return YES;
}
BOOL BLAutoDetect(UIImage *image, CGSize screenSize, BOOL occupied[64],
                  BLPiece pieces[3], CGPoint *firstCenter, CGPoint *lastCenter,
                  NSString **detail) {
    uint8_t *px=NULL;size_t w=0,h=0;
    if(!BLDecodeImage(image,screenSize,&px,&w,&h)) {
        if(detail)*detail=@"Could not decode screen frame.";
        return NO;
    }
    int left=0,top=0,right=0,bottom=0;
    if(!BLLocateBoard(px,w,h,&left,&top,&right,&bottom)) {
        free(px);
        if(detail)*detail=@"Automatic board location uncertain. Wait for a clear game screen.";
        return NO;
    }
    double boardWidth=(double)(right-left),boardHeight=(double)(bottom-top);
    double pitchX=boardWidth/8.0,pitchY=boardHeight/8.0;
    BOOL detectedBoard[64]={NO};
    for(int r=0;r<8;++r) for(int c=0;c<8;++c) {
        int x=(int)lround(left+(c+.5)*pitchX);
        int y=(int)lround(top+(r+.5)*pitchY);
        if(x<3||x>=(int)w-3||y<3||y>=(int)h-3) {free(px);return NO;}
        // Vote across center pixels to ignore one sparkling animation pixel.
        int bright=0;
        for(int dy=-2;dy<=2;dy+=2) for(int dx=-2;dx<=2;dx+=2)
            bright+=BLIsBrightGameBlock(BLPixel(px,w,x+dx,y+dy));
        detectedBoard[r*8+c]=(bright>=5);
    }
    BLPiece detectedPieces[3]={0};
    double piecePitch=pitchX*.44; // Confirmed from full-screen iPhone reference.
    if(piecePitch<9 || piecePitch>80) {free(px);return NO;}
    int trayTop=MIN((int)h-1, bottom+(int)lround(boardWidth*.10));
    int trayBottom=MIN((int)(h*.95), bottom+(int)lround(boardWidth*.70));
    if(trayBottom<=trayTop+20) {free(px);return NO;}
    for(int p=0;p<3;++p) {
        int regionLeft=(int)((double)p*w/3.0);
        int regionRight=(int)((double)(p+1)*w/3.0);
        int xmin=(int)w,xmax=-1,ymin=(int)h,ymax=-1,brightCount=0;
        for(int y=trayTop;y<trayBottom;++y) for(int x=regionLeft;x<regionRight;++x) {
            if(BLIsBrightTrayBlock(BLPixel(px,w,x,y))) {
                xmin=MIN(xmin,x);xmax=MAX(xmax,x);
                ymin=MIN(ymin,y);ymax=MAX(ymax,y);++brightCount;
            }
        }
        if(brightCount<30) continue; // This tray slot is currently empty.
        int cols=(int)lround((xmax-xmin+1)/piecePitch);
        int rows=(int)lround((ymax-ymin+1)/piecePitch);
        if(cols<1||cols>5||rows<1||rows>5) {
            free(px);
            if(detail)*detail=@"Tray piece dimensions are uncertain. Recheck the game layout.";
            return NO;
        }
        BLPiece shape={0};
        for(int r=0;r<rows;++r) for(int c=0;c<cols;++c) {
            int x=(int)lround(xmin+(c+.5)*piecePitch);
            int y=(int)lround(ymin+(r+.5)*piecePitch);
            x=MAX(2,MIN((int)w-3,x));y=MAX(2,MIN((int)h-3,y));
            int bright=0;
            for(int dy=-2;dy<=2;dy+=2)for(int dx=-2;dx<=2;dx+=2)
                bright+=BLIsBrightTrayBlock(BLPixel(px,w,x+dx,y+dy));
            if(bright>=5) {
                int idx=shape.count++;
                shape.x[idx]=c;
                shape.y[idx]=r;
            }
        }
        detectedPieces[p]=shape;
    }
    free(px);
    memcpy(occupied,detectedBoard,sizeof(detectedBoard));
    memcpy(pieces,detectedPieces,sizeof(detectedPieces));
    if(firstCenter)*firstCenter=CGPointMake(left+.5*pitchX,top+.5*pitchY);
    if(lastCenter)*lastCenter=CGPointMake(left+7.5*pitchX,top+7.5*pitchY);
    int blocks=0,slots=0;
    for(int i=0;i<64;++i)blocks+=detectedBoard[i];
    for(int p=0;p<3;++p)slots+=(detectedPieces[p].count>0);
    if(detail)*detail=[NSString stringWithFormat:@"AUTO: %d filled squares, %d tray pieces.",blocks,slots];
    return YES;
}
