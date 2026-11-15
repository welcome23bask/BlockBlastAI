#import "BLTrainingCollector.h"
#import "BLTrainingZIP.h"
#import <CoreGraphics/CoreGraphics.h>
#include <math.h>
#include <stdint.h>

NSString * const BLTrainingCollectorDidChangeNotification=@"BLTrainingCollectorDidChange";
static const NSUInteger kTrainingMaximum=200;
static const NSTimeInterval kTrainingMinimumGap=1.5;
// Images + JSON always stay inside the app's Documents sandbox, not Photos,
// iCloud, GitHub, or any remote service.
static NSString *BLTrainingFolder(void) {
    NSString *docs=NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,
                                                        NSUserDomainMask,YES).firstObject;
    return [[docs stringByAppendingPathComponent:@"BlockBlastAI"]
                 stringByAppendingPathComponent:@"VisionTraining"];
}
static uint64_t hashValue(uint64_t hash,uint64_t n) {
    for(int k=0;k<8;++k) {
        hash^=(n>>(k*8))&255U;
        hash*=UINT64_C(1099511628211);
    }
    return hash;
}
static uint32_t sampleSkin(CGImageRef cg,CGRect pixelBoard) {
    size_t iw=CGImageGetWidth(cg),ih=CGImageGetHeight(cg);
    if(!iw||!ih)return 0;
    int x=(int)floor(pixelBoard.origin.x+pixelBoard.size.width*.02);
    int y=(int)floor(pixelBoard.origin.y+pixelBoard.size.height*.02);
    x=MAX(0,MIN((int)iw-1,x));
    y=MAX(0,MIN((int)ih-1,y));
    CGImageRef tiny=CGImageCreateWithImageInRect(cg,CGRectMake(x,y,1,1));
    if(!tiny)return 0;
    uint8_t px[4]={0};
    CGColorSpaceRef cs=CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx=CGBitmapContextCreate(px,1,1,8,4,cs,
                           kCGImageAlphaPremultipliedLast|kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(cs);
    if(ctx){CGContextDrawImage(ctx,CGRectMake(0,0,1,1),tiny);CGContextRelease(ctx);}
    CGImageRelease(tiny);
    return (uint32_t)(px[0]/32)<<10 |
           (uint32_t)(px[1]/32)<<5 |
           (uint32_t)(px[2]/32);
}
static NSArray *BLCapturedPieceLabels(const BLPiece pieces[3]) {
    NSMutableArray *result=[NSMutableArray arrayWithCapacity:3];
    for(int p=0;p<3;++p) {
        NSMutableArray *cells=[NSMutableArray array];
        for(int j=0;j<pieces[p].count&&j<25;++j)
            [cells addObject:@[@(pieces[p].x[j]),@(pieces[p].y[j])]];
        [result addObject:@{@"slot":@(p+1),@"cells":cells}];
    }
    return result;
}
@interface BLTrainingCollector ()
@end
@implementation BLTrainingCollector {
    BOOL _enabled;
    NSUInteger _savedCount, _pendingCount, _generation;
    NSTimeInterval _lastSubmitted;
    NSMutableSet<NSString *> *_seenKeys;
    dispatch_queue_t _ioQueue;
    NSString *_status;
}
+ (instancetype)shared {
    static BLTrainingCollector *instance;
    static dispatch_once_t once;
    dispatch_once(&once, ^{instance=[BLTrainingCollector new];});
    return instance;
}
- (instancetype)init {
    self=[super init];
    if(!self)return nil;
    _enabled=NO; // never quietly begin recording a new session
    _ioQueue=dispatch_queue_create("com.blockblastai.visiontrainer",DISPATCH_QUEUE_SERIAL);
    _seenKeys=[NSMutableSet new];
    _status=@"OFF — screenshots are not being saved.";
    NSString *folder=BLTrainingFolder();
    [[NSFileManager defaultManager] createDirectoryAtPath:folder
                              withIntermediateDirectories:YES attributes:nil error:NULL];
    NSArray<NSString *> *names=[[NSFileManager defaultManager]
                                  contentsOfDirectoryAtPath:folder error:NULL];
    for(NSString *name in names) {
        if(![name.pathExtension.lowercaseString isEqualToString:@"json"])continue;
        NSString *full=[folder stringByAppendingPathComponent:name];
        NSData *data=[NSData dataWithContentsOfFile:full];
        NSDictionary *json=data?[NSJSONSerialization JSONObjectWithData:data
                                                                options:0 error:NULL]:nil;
        if(![json isKindOfClass:NSDictionary.class])continue;
        NSString *key=json[@"dedupeKey"];
        if([key isKindOfClass:NSString.class])[_seenKeys addObject:key];
        NSString *jpg=[[name stringByDeletingPathExtension] stringByAppendingPathExtension:@"jpg"];
        if([[NSFileManager defaultManager] fileExistsAtPath:
                        [folder stringByAppendingPathComponent:jpg]])_savedCount++;
    }
    return self;
}
- (BOOL)isEnabled {@synchronized(self){return _enabled;}}
- (NSUInteger)savedCount {@synchronized(self){return _savedCount;}}
- (NSUInteger)limit {return kTrainingMaximum;}
- (NSString *)status {@synchronized(self){return _status;}}
- (void)notify {
    NSAssert(NSThread.isMainThread,@"Notifications delivered on main thread");
    [[NSNotificationCenter defaultCenter]
        postNotificationName:BLTrainingCollectorDidChangeNotification object:self];
}
- (void)setEnabled:(BOOL)enabled {
    if(!NSThread.isMainThread) {
        dispatch_async(dispatch_get_main_queue(),^{[self setEnabled:enabled];});
        return;
    }
    @synchronized(self) {
        if(_enabled==enabled)return;
        _enabled=enabled;
        ++_generation;
        _lastSubmitted=0;
        _status=enabled?@"ON — collecting stable board/tray crops locally."
                        :@"OFF — screenshots are not being saved.";
        if(_savedCount>=kTrainingMaximum&&enabled){
            _enabled=NO;
            _status=@"200/200 saved. Export ZIP, then clear to collect more.";
        }
    }
    [self notify];
}
- (void)observeStableFrame:(UIImage *)image
                    board:(uint64_t)board
                   pieces:(const BLPiece [3])pieces
                    first:(CGPoint)first
                     last:(CGPoint)last
               screenSize:(CGSize)screenSize {
    if(!image.CGImage || !pieces || screenSize.width<=0 || screenSize.height<=0)return;
    // Caller must only supply a settled, scanner-validated gameplay frame.
    double pitchX=(last.x-first.x)/7.0,pitchY=(last.y-first.y)/7.0;
    double pitch=(pitchX+pitchY)*.5;
    if(pitch<15||fabs(pitchX-pitchY)>pitch*.14)return;
    int total=0;
    for(int p=0;p<3;++p) {
        if(pieces[p].count<0||pieces[p].count>25)return;
        total+=pieces[p].count;
    }
    if(total<1)return;
    CGRect boardRect=CGRectMake(first.x-pitch*.5,first.y-pitch*.5,
                                pitch*8,pitch*8);
    if(boardRect.origin.y<0||CGRectGetMaxY(boardRect)>screenSize.height||
       boardRect.origin.x<0||CGRectGetMaxX(boardRect)>screenSize.width)return;
    // Save ONLY gameplay board + tray. No score, user notifications, header
    // or unrelated screen contents. Capture ends below the 3-piece tray.
    CGFloat cropY=MAX(0,boardRect.origin.y-pitch*.12);
    CGFloat cropEnd=MIN(screenSize.height,CGRectGetMaxY(boardRect)+pitch*8*.70);
    CGFloat yScale=CGImageGetHeight(image.CGImage)/screenSize.height;
    CGFloat xScale=CGImageGetWidth(image.CGImage)/screenSize.width;
    CGRect pixelCrop=CGRectIntegral(CGRectMake(0,cropY*yScale,
                        CGImageGetWidth(image.CGImage),(cropEnd-cropY)*yScale));
    pixelCrop=CGRectIntersection(pixelCrop,CGRectMake(0,0,CGImageGetWidth(image.CGImage),
                                                      CGImageGetHeight(image.CGImage)));
    CGRect pixelBoard=CGRectMake(boardRect.origin.x*xScale,boardRect.origin.y*yScale,
                                boardRect.size.width*xScale,boardRect.size.height*yScale);
    if(CGRectIsNull(pixelCrop)||pixelCrop.size.width<200||pixelCrop.size.height<200)return;
    uint32_t skin=sampleSkin(image.CGImage,pixelBoard);
    uint64_t digest=hashValue(UINT64_C(1469598103934665603),board);
    digest=hashValue(digest,skin);
    for(int p=0;p<3;++p) {
        digest=hashValue(digest,pieces[p].count);
        for(int k=0;k<pieces[p].count;++k)
            digest=hashValue(hashValue(digest,pieces[p].x[k]),pieces[p].y[k]);
    }
    NSString *key=[NSString stringWithFormat:@"%016llx",(unsigned long long)digest];
    NSTimeInterval now=[NSDate date].timeIntervalSince1970;
    NSUInteger generation=0;
    @synchronized(self) {
        if(!_enabled||_savedCount+_pendingCount>=kTrainingMaximum||
           now-_lastSubmitted<kTrainingMinimumGap||[_seenKeys containsObject:key])return;
        _lastSubmitted=now;
        [_seenKeys addObject:key];++_pendingCount;
        generation=_generation;
    }
    BLTraySnapshot unused; (void)unused;
    NSMutableArray *rows=[NSMutableArray arrayWithCapacity:8];
    for(int r=0;r<8;++r) {
        NSMutableArray *cells=[NSMutableArray arrayWithCapacity:8];
        for(int c=0;c<8;++c)[cells addObject:@((board>>(r*8+c))&1)];
        [rows addObject:cells];
    }
    NSArray *tray=BLCapturedPieceLabels(pieces);
    NSString *uid=NSUUID.UUID.UUIDString;
    NSDate *date=[NSDate date];
    NSString *iso=[NSISO8601DateFormatter.new stringFromDate:date];
    NSDictionary *label=@{
        @"schemaVersion":@1,
        @"id":uid,
        @"timestampUTC":iso?:@"",
        @"dedupeKey":key,
        @"source":@"nativeScanner_unverified",
        @"reviewed":@NO,
        @"trainable":@NO,
        @"board":rows,
        @"pieces":tray,
        @"boardRectInCrop":@{
            @"x":@(boardRect.origin.x*xScale-pixelCrop.origin.x),
            @"y":@(boardRect.origin.y*yScale-pixelCrop.origin.y),
            @"width":@(boardRect.size.width*xScale),
            @"height":@(boardRect.size.height*yScale)
        },
        @"cropPixels":@{@"width":@(pixelCrop.size.width),
                        @"height":@(pixelCrop.size.height)},
        @"skinFingerprint":@(skin),
        @"note":@"PROVISIONAL detection labels. Review/correct before training."
    };
    UIImage *frame=image;
    dispatch_async(_ioQueue, ^{
        BOOL ok=NO;
        @autoreleasepool {
            BOOL stillEnabled=NO;
            @synchronized(self){stillEnabled=_enabled&&_generation==generation;}
            if(stillEnabled) {
                CGImageRef crop=CGImageCreateWithImageInRect(frame.CGImage,pixelCrop);
                if(crop) {
                    UIImage *cropped=[UIImage imageWithCGImage:crop];
                    NSData *jpg=UIImageJPEGRepresentation(cropped,.85);
                    CGImageRelease(crop);
                    NSData *json=[NSJSONSerialization dataWithJSONObject:label
                                                                 options:NSJSONWritingPrettyPrinted error:NULL];
                    NSString *base=[BLTrainingFolder() stringByAppendingPathComponent:
                                   [@"capture-" stringByAppendingString:uid]];
                    NSString *jpgFile=[base stringByAppendingPathExtension:@"jpg"];
                    NSString *jsonFile=[base stringByAppendingPathExtension:@"json"];
                    if(jpg.length>0&&json.length>0&&
                       [jpg writeToFile:jpgFile atomically:YES]&&
                       [json writeToFile:jsonFile atomically:YES])ok=YES;
                    else {
                        [[NSFileManager defaultManager] removeItemAtPath:jpgFile error:NULL];
                        [[NSFileManager defaultManager] removeItemAtPath:jsonFile error:NULL];
                    }
                }
            }
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            @synchronized(self){
                if(_pendingCount>0)--_pendingCount;
                if(ok) {
                    ++_savedCount;
                    _status=[NSString stringWithFormat:@"Saved %lu/%lu local samples. Labels need review.",
                             (unsigned long)_savedCount,(unsigned long)kTrainingMaximum];
                    if(_savedCount>=kTrainingMaximum){
                        _enabled=NO;++_generation;
                        _status=@"200/200 saved. Export ZIP, then clear to collect more.";
                    }
                }else{
                    [_seenKeys removeObject:key];
                    _status=@"Capture skipped or failed. Check available device storage.";
                }
            }
            [self notify];
        });
    });
}
- (void)createArchive:(void (^)(NSURL * _Nullable, NSError * _Nullable))completion {
    if(!completion)return;
    dispatch_async(_ioQueue, ^{
        NSString *folder=BLTrainingFolder();
        NSFileManager *fm=NSFileManager.defaultManager;
        NSArray<NSString *> *files=[fm contentsOfDirectoryAtPath:folder error:NULL];
        NSMutableArray<NSString *> *names=[NSMutableArray array];
        for(NSString *name in files) {
            NSString *ext=name.pathExtension.lowercaseString;
            if([ext isEqualToString:@"jpg"]||[ext isEqualToString:@"json"])
                [names addObject:name];
        }
        [names sortUsingSelector:@selector(compare:)];
        NSString *readme=@"BlockBlastAI Vision Training Dataset\n"
                          "Screenshots cropped to board and tray; no automatic upload.\n"
                          "Labels are UNREVIEWED scanner predictions. Confirm/correct all labels before training.\n"
                          "Board is 8x8 (0 empty, 1 filled). Tray coordinates are local zero-based cells.\n"
                          "To gather additional screenshots, export and explicitly clear the local collection.\n";
        [readme writeToFile:[folder stringByAppendingPathComponent:@"README.txt"]
                atomically:YES encoding:NSUTF8StringEncoding error:NULL];
        [names addObject:@"README.txt"];
        NSString *zip=[NSTemporaryDirectory() stringByAppendingPathComponent:
                        [NSString stringWithFormat:@"BlockBlastAI-Vision-%@.zip",NSUUID.UUID.UUIDString]];
        NSMutableArray<NSValue *> *refs=[NSMutableArray array];
        for(NSString *name in names)[refs addObject:[NSValue valueWithPointer:name.UTF8String]];
        // UTF8String pointers may be invalid after autorelease pools; keep
        // an owned C copy until the synchronous ZIP writer has completed.
        const char **paths=calloc(names.count,sizeof(char *));
        for(NSUInteger i=0;i<names.count;++i)
            paths[i]=strdup(names[i].UTF8String);
        int success=paths!=NULL &&
                    BLTrainingWriteZIP(zip.fileSystemRepresentation,
                                       folder.fileSystemRepresentation,
                                       paths,names.count);
        if(paths) {
            for(NSUInteger i=0;i<names.count;++i)free((void *)paths[i]);
            free(paths);
        }
        NSError *err=success?nil:[NSError errorWithDomain:@"BlockBlastAI"
            code:91 userInfo:@{NSLocalizedDescriptionKey:
                        @"Could not create training ZIP. Check device storage."}];
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(success?[NSURL fileURLWithPath:zip]:nil,err);
        });
    });
}
- (void)deleteAllWithCompletion:(void (^)(NSError * _Nullable))completion {
    [self setEnabled:NO];
    dispatch_async(_ioQueue, ^{
        NSString *folder=BLTrainingFolder();
        NSFileManager *fm=NSFileManager.defaultManager;
        NSError *failure=nil;
        for(NSString *name in [fm contentsOfDirectoryAtPath:folder error:NULL]) {
            if(![@[@"jpg",@"json",@"txt"] containsObject:name.pathExtension.lowercaseString])continue;
            NSError *err=nil;
            if(![fm removeItemAtPath:[folder stringByAppendingPathComponent:name] error:&err])
                failure=err;
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            @synchronized(self) {
                _savedCount=0;_pendingCount=0;[_seenKeys removeAllObjects];
                _status=@"Local training screenshots cleared. Training OFF.";
            }
            [self notify];
            if(completion)completion(failure);
        });
    });
}
@end
