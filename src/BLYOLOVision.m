// Real, optional YOLOv8n Core ML object detector. There is no trained model
// embedded in this repository. No model means immediate native-scan fallback.
// Expected custom class labels: board, occupied_cell, tray_cell.
// Unlike the website's custom weights, these weights must be trained by the
// user and compiled to "BlockBlastYOLOv8n.mlmodelc" before use.
#import "BLYOLOVision.h"
#import <CoreML/CoreML.h>
#import <Vision/Vision.h>
#import <dlfcn.h>
#import <math.h>
#import <string.h>

typedef struct { CGRect r; float confidence; } BLBox;
static VNCoreMLModel *sVisionModel=nil;
static BOOL sLookedForModel=NO;

static VNCoreMLModel *BLLoadCustomModel(void) {
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        sLookedForModel=YES;
        NSURL *url=[[NSBundle mainBundle] URLForResource:@"BlockBlastYOLOv8n"
                                           withExtension:@"mlmodelc"];
        if(!url) {
            Dl_info info={0};
            if(dladdr((const void *)&BLYOLOModelAvailable,&info) &&
               info.dli_fname) {
                NSString *lib=[NSString stringWithUTF8String:info.dli_fname];
                NSString *path=[[lib stringByDeletingLastPathComponent]
                                  stringByAppendingPathComponent:@"BlockBlastYOLOv8n.mlmodelc"];
                if([[NSFileManager defaultManager] fileExistsAtPath:path])
                    url=[NSURL fileURLWithPath:path isDirectory:YES];
            }
        }
        if(!url)return;
        NSError *error=nil;
        MLModelConfiguration *config=[MLModelConfiguration new];
        config.computeUnits=MLComputeUnitsAll;
        MLModel *model=[MLModel modelWithContentsOfURL:url
                                       configuration:config error:&error];
        if(!model||error)return;
        sVisionModel=[VNCoreMLModel modelForMLModel:model error:&error];
        if(error)sVisionModel=nil;
    });
    return sVisionModel;
}
BOOL BLYOLOModelAvailable(void) {
    (void)sLookedForModel;
    return BLLoadCustomModel()!=nil;
}
// Vision bounding boxes use bottom-origin normalized coordinates.
static CGRect image_box(VNRecognizedObjectObservation *obs,CGFloat w,CGFloat h) {
    CGRect b=obs.boundingBox;
    return CGRectMake(b.origin.x*w,
                      (1.0-b.origin.y-b.size.height)*h,
                      b.size.width*w,b.size.height*h);
}
static CGFloat center_x(CGRect r) {return CGRectGetMidX(r);}
static CGFloat center_y(CGRect r) {return CGRectGetMidY(r);}
static BOOL place_and_verify_board(CGRect rect,CGSize screen) {
    CGFloat side=rect.size.width,vertical=rect.size.height;
    if(side<screen.width*0.72 || side>screen.width*1.03)return NO;
    if(fabs(side-vertical)/fmax(1.0,side)>0.085)return NO;
    if(fabs(center_x(rect)-screen.width*0.5)>screen.width*0.10)return NO;
    if(fabs(center_y(rect)/screen.height-0.44)>0.095)return NO;
    if(rect.origin.x<0||rect.origin.y<0||CGRectGetMaxX(rect)>screen.width||
       CGRectGetMaxY(rect)>screen.height)return NO;
    return YES;
}
BOOL BLYOLOTryDetect(UIImage *image,CGSize screen,BOOL occupied[64],
                     BLPiece pieces[3],CGPoint *first,CGPoint *last,
                     NSString **detail) {
    VNCoreMLModel *model=BLLoadCustomModel();
    if(!model||!image.CGImage || image.imageOrientation!=UIImageOrientationUp ||
       screen.width<240||screen.height<350||!occupied||!pieces)return NO;
    VNCoreMLRequest *request=[[VNCoreMLRequest alloc] initWithModel:model];
    // Model is trained on the full portrait screenshot. No crop that would
    // move the board/tray out of the coordinate system.
    request.imageCropAndScaleOption=VNImageCropAndScaleOptionScaleFill;
    VNImageRequestHandler *handler=[[VNImageRequestHandler alloc]
                                    initWithCGImage:image.CGImage options:@{}];
    NSError *error=nil;
    if(![handler performRequests:@[request] error:&error] || error)return NO;
    CGRect boardRect=CGRectZero;
    float boardConf=0;
    BLBox occupiedBoxes[64]={0},trayBoxes[3][25]={0};
    int occCount=0,trayCount[3]={0,0,0};
    CGFloat imageWidth=screen.width,imageHeight=screen.height;
    for(VNObservation *observation in request.results) {
        if(![observation isKindOfClass:[VNRecognizedObjectObservation class]])continue;
        VNRecognizedObjectObservation *object=(VNRecognizedObjectObservation *)observation;
        VNClassificationObservation *label=object.labels.firstObject;
        if(!label)continue;
        CGRect box=image_box(object,imageWidth,imageHeight);
        CGFloat cx=center_x(box),cy=center_y(box);
        if([label.identifier isEqualToString:@"board"]) {
            if(label.confidence>=0.82 && label.confidence>boardConf) {
                boardConf=label.confidence;
                boardRect=box;
            }
        } else if([label.identifier isEqualToString:@"occupied_cell"]) {
            if(label.confidence>=0.68 && occCount<64)
                occupiedBoxes[occCount++]=(BLBox){box,label.confidence};
        } else if([label.identifier isEqualToString:@"tray_cell"]) {
            if(label.confidence<0.70 || cy<imageHeight*0.58 ||
               cy>imageHeight*0.95)continue;
            int slot=(int)floor(cx/(imageWidth/3.0));
            if(slot<0||slot>2)continue;
            if(trayCount[slot]<25) {
                trayBoxes[slot][trayCount[slot]++]=(BLBox){box,label.confidence};
            }
        }
    }
    if(boardConf<0.82||!place_and_verify_board(boardRect,screen))return NO;
    uint64_t board=0;
    for(int j=0;j<occCount;++j) {
        CGRect rect=occupiedBoxes[j].r;
        int col=(int)floor((center_x(rect)-boardRect.origin.x)*8/boardRect.size.width);
        int row=(int)floor((center_y(rect)-boardRect.origin.y)*8/boardRect.size.height);
        if(row<0||row>7||col<0||col>7)return NO;
        CGFloat cellW=boardRect.size.width/8.0;
        CGFloat cellH=boardRect.size.height/8.0;
        CGFloat idealX=boardRect.origin.x+(col+0.5)*cellW;
        CGFloat idealY=boardRect.origin.y+(row+0.5)*cellH;
        if(fabs(center_x(rect)-idealX)>cellW*0.30 ||
           fabs(center_y(rect)-idealY)>cellH*0.30)return NO;
        uint64_t bit=UINT64_C(1)<<(row*8+col);
        if(board&bit)return NO; // contradictory overlapping model boxes
        board|=bit;
    }
    int detected=0;
    BLPiece result[3]={0};
    for(int slot=0;slot<3;++slot) {
        int n=trayCount[slot];
        if(!n)continue;
        CGFloat minX=CGFLOAT_MAX,minY=CGFLOAT_MAX,sumW=0,sumH=0;
        for(int j=0;j<n;++j) {
            CGRect b=trayBoxes[slot][j].r;
            if(center_y(b)<=CGRectGetMaxY(boardRect)+boardRect.size.height*0.06)
                return NO; // board cells mistaken for tray pieces
            minX=fmin(minX,center_x(b));minY=fmin(minY,center_y(b));
            sumW+=b.size.width;sumH+=b.size.height;
        }
        CGFloat pitch=(sumW+sumH)/(2*n)*1.065;
        if(pitch<imageWidth*0.020||pitch>imageWidth*0.090)return NO;
        BOOL used[25]={NO};
        for(int j=0;j<n;++j) {
            CGFloat dx=(center_x(trayBoxes[slot][j].r)-minX)/pitch;
            CGFloat dy=(center_y(trayBoxes[slot][j].r)-minY)/pitch;
            int x=(int)lround(dx),y=(int)lround(dy);
            if(x<0||x>=5||y<0||y>=5||
               fabs(dx-x)>0.34||fabs(dy-y)>0.34||used[y*5+x])
                return NO;
            used[y*5+x]=YES;
            result[slot].x[result[slot].count]=x;
            result[slot].y[result[slot].count]=y;
            ++result[slot].count;
        }
        // All detected tray squares must form one connected tetromino-like
        // piece. Missing isolated detections should be rejected, not guessed.
        BOOL visited[25]={NO};
        int queue[25]={0},head=0,tail=0;
        for(int k=0;k<25;++k)if(used[k]) {visited[k]=YES;queue[tail++]=k;break;}
        while(head<tail) {
            int pos=queue[head++],x=pos%5,y=pos/5;
            const int ox[]={-1,1,0,0},oy[]={0,0,-1,1};
            for(int d=0;d<4;++d) {
                int nx=x+ox[d],ny=y+oy[d];
                if(nx<0||nx>=5||ny<0||ny>=5)continue;
                int k=ny*5+nx;
                if(used[k]&&!visited[k]){visited[k]=YES;queue[tail++]=k;}
            }
        }
        if(tail!=n)return NO;
        ++detected;
    }
    if(!detected)return NO;
    for(int i=0;i<64;++i)
        occupied[i]=(board&(UINT64_C(1)<<i))!=0;
    memcpy(pieces,result,sizeof(result));
    CGFloat cell=boardRect.size.width/8.0;
    if(first)*first=CGPointMake(boardRect.origin.x+cell/2.0,
                               boardRect.origin.y+boardRect.size.height/16.0);
    if(last)*last=CGPointMake(boardRect.origin.x+cell*7.5,
                             boardRect.origin.y+boardRect.size.height*15.0/16.0);
    if(detail)*detail=[NSString stringWithFormat:
         @"Custom YOLOv8n/Core ML: %d filled cells and %d pieces (validated).",
         occCount,detected];
    return YES;
}
