// BlockBlastAI v1.0 Fast Vision prototype — experimental in-app move assistant.
// Public UIKit/ReplayKit APIs only. Reads the screen to suggest moves; never touches or places pieces.
// v0.8: stale-frame protection. Every detection must come from a fresh captured frame; capture
// stalls/restarts clear the board, tray, solution, highlight and confirmation buffer.
// Designed for injection into a compatible, properly signed iPhone app by its owner.
#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#import "BlockBlastSolver.h"
#import "BLScreenScan.h"
#import "BLTransition.h"
#include <math.h>
#include <stdint.h>

static UIColor *BLColor(CGFloat r, CGFloat g, CGFloat b) {
    return [UIColor colorWithRed:r/255.0 green:g/255.0 blue:b/255.0 alpha:1.0];
}
static const CGFloat kBoardPitch=30.0;
static const CGFloat kPiecePitch=19.0;
static const CFTimeInterval kStaleAfter=3.0;   // no fresh frame for this long => clear everything
static const CFTimeInterval kAutoScanInterval=0.24; // faster; ReplayKit and frame processing still bound latency
static const CFTimeInterval kCaptureWarmup=0.04;
static const NSInteger kConfirmFrames=2;
static const NSInteger kUncertainTransitionFrames=3;
// C Blocks cannot capture raw local arrays. A value-type wrapper copies all
// three pieces safely onto the background solver work item.
typedef struct { BLPiece values[3]; } BLTraySnapshot;         // identical hashes despite a forced pixel change
@interface BLPassThroughWindow : UIWindow
@end
@implementation BLPassThroughWindow
- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event {
    UIView *hit=[super hitTest:point withEvent:event];
    return hit==self.rootViewController.view ? nil : hit;
}
@end

@interface BLController : UIViewController
@end

@implementation BLController {
    UIButton *_bubble;
    UIView *_panel;
    UIScrollView *_scroll;
    UIView *_contentView;
    UIView *_dragStrip;
    UILabel *_resizeGrip;
    UIView *_guideLayer;
    UIView *_calibrationLayer;
    UILabel *_calibrationLabel;
    CGPoint _gridFirst, _gridLast, _calibrationFirst;
    BOOL _calibrated, _showGuides, _showCalibrationGrid;
    int _calibrationStage;
    UILabel *_status;
    UILabel *_moveLabel;
    UILabel *_info;
    UIButton *_boardButtons[64];
    UIButton *_pieceButtons[3][25];
    BOOL _board[64];
    BOOL _pieces[3][25];
    BLPiece _normalizedPieces[3];
    BLSolution _solution;
    int _previewIndex;
    BOOL _previewing;
    CGFloat _panelWidth;
    BOOL _autoEnabled, _autoBusy;
    NSInteger _autoErrors, _stableAutoFrames;
    CFTimeInterval _autoRetryAfter;
    uint64_t _observedAutoHash, _solvedAutoHash;
    uint64_t _solveGeneration; // discard background solver results after any newer observation
    NSTimer *_autoTimer;
    UIButton *_autoButton;
    UILabel *_autoLabel;
    UIView *_nudgeStrip;      // 2pt strip flashed for one frame so a quiet screen still yields a new frame
    BOOL _nudgeOn;
    unsigned long _lastFrameCounter, _lastRestartCount;
    CFAbsoluteTime _lastFreshAt;
    NSInteger _frozenRepeats;
}

- (UIButton *)roundedButton:(NSString *)title frame:(CGRect)frame action:(SEL)selector {
    UIButton *button=[UIButton buttonWithType:UIButtonTypeSystem];
    button.frame=frame;
    button.layer.cornerRadius=8;
    button.backgroundColor=BLColor(37,58,82);
    [button setTitle:title forState:UIControlStateNormal];
    [button setTitleColor:UIColor.whiteColor forState:UIControlStateNormal];
    button.titleLabel.font=[UIFont boldSystemFontOfSize:12];
    [button addTarget:self action:selector forControlEvents:UIControlEventTouchUpInside];
    return button;
}
- (UILabel *)addLabel:(NSString *)title y:(CGFloat)y height:(CGFloat)height font:(CGFloat)font {
    UILabel *label=[[UILabel alloc] initWithFrame:CGRectMake(12,y,_panelWidth-24,height)];
    label.text=title;
    label.textColor=BLColor(229,238,245);
    label.font=[UIFont systemFontOfSize:font];
    label.adjustsFontSizeToFitWidth=YES;
    label.minimumScaleFactor=0.65;
    label.numberOfLines=2;
    [_contentView addSubview:label];
    return label;
}
- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor=UIColor.clearColor;
    CGFloat width=self.view.bounds.size.width;
    CGFloat height=self.view.bounds.size.height;
    if (width<100) width=390;
    if (height<100) height=844;
    _panelWidth=356;

    _bubble=[self roundedButton:@"🧩 AI" frame:CGRectMake(width-72,110,66,46) action:@selector(togglePanel)];
    _bubble.layer.cornerRadius=22;
    _bubble.layer.borderColor=BLColor(52,211,153).CGColor;
    _bubble.layer.borderWidth=2;
    _bubble.backgroundColor=BLColor(13,23,40);
    _bubble.titleLabel.font=[UIFont boldSystemFontOfSize:15];
    [self.view addSubview:_bubble];
    UIPanGestureRecognizer *pan=[[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(dragBubble:)];
    [_bubble addGestureRecognizer:pan];

    _panel=[[UIView alloc] initWithFrame:CGRectMake(12,65,MIN(356,width-24),MIN(690,height-88))];
    NSString *saved=[[NSUserDefaults standardUserDefaults] stringForKey:@"BLPanelFrameV2"];
    if(saved.length) {
        CGRect f=CGRectFromString(saved);
        if(isfinite(f.origin.x)&&isfinite(f.origin.y)&&isfinite(f.size.width)&&isfinite(f.size.height)) {
            f.size.width=MAX(305,MIN(f.size.width,width-12));
            f.size.height=MAX(245,MIN(f.size.height,height-64));
            f.origin.x=MAX(0,MIN(f.origin.x,width-f.size.width));
            f.origin.y=MAX(30,MIN(f.origin.y,height-f.size.height-8));
            _panel.frame=f;
        }
    }
    _panel.layer.cornerRadius=16;
    _panel.layer.borderWidth=1;
    _panel.layer.borderColor=BLColor(63,84,106).CGColor;
    _panel.clipsToBounds=YES;
    _panel.backgroundColor=BLColor(18,30,48);
    [self.view addSubview:_panel];
    _panel.hidden=YES;

    _scroll=[[UIScrollView alloc] initWithFrame:CGRectZero];
    _scroll.alwaysBounceVertical=YES;
    [_panel addSubview:_scroll];
    _contentView=[[UIView alloc] initWithFrame:CGRectMake(0,0,356,868)];
    _contentView.layer.anchorPoint=CGPointZero;
    _contentView.layer.position=CGPointZero;
    [_scroll addSubview:_contentView];
    UILabel *title=[self addLabel:@"BLOCK BLAST AI · MOVE ASSISTANT" y:13 height:24 font:16];
    title.font=[UIFont boldSystemFontOfSize:16];
    UIButton *close=[self roundedButton:@"×" frame:CGRectMake(_panelWidth-41,10,29,28) action:@selector(togglePanel)];
    [_contentView addSubview:close];
    _autoLabel=[self addLabel:@"AUTO: Board + pieces detected automatically (manual fallback below)" y:42 height:22 font:12];

    CGFloat x0=(_panelWidth-8*kBoardPitch)/2;
    for (int r=0;r<8;++r) for (int c=0;c<8;++c) {
        int idx=r*8+c;
        UIButton *tile=[self roundedButton:@"" frame:CGRectMake(x0+c*kBoardPitch,69+r*kBoardPitch,28,28) action:@selector(toggleBoardTile:)];
        tile.tag=idx;
        tile.layer.cornerRadius=4;
        [_contentView addSubview:tile];
        _boardButtons[idx]=tile;
    }
    UIButton *clearBoard=[self roundedButton:@"Clear board" frame:CGRectMake(12,315,105,30) action:@selector(clearBoard)];
    [_contentView addSubview:clearBoard];
    UIButton *edit=[self roundedButton:@"Edit inputs" frame:CGRectMake(125,315,102,30) action:@selector(editInputs)];
    [_contentView addSubview:edit];
    UIButton *clearAll=[self roundedButton:@"Reset all" frame:CGRectMake(235,315,_panelWidth-247,30) action:@selector(clearAll)];
    [_contentView addSubview:clearAll];
    [self addLabel:@"Automatically recognized tray pieces (editable if paused):" y:357 height:19 font:12];

    CGFloat pieceGridWidth=5*kPiecePitch;
    CGFloat pieceGap=(_panelWidth-3*pieceGridWidth)/4;
    for (int p=0;p<3;++p) {
        CGFloat pieceX=pieceGap+(pieceGridWidth+pieceGap)*p;
        UILabel *pieceTitle=[[UILabel alloc] initWithFrame:CGRectMake(pieceX,381,pieceGridWidth,20)];
        pieceTitle.text=[NSString stringWithFormat:@"PIECE %d",p+1];
        pieceTitle.font=[UIFont boldSystemFontOfSize:11];
        pieceTitle.textAlignment=NSTextAlignmentCenter;
        pieceTitle.textColor=BLColor(164,190,213);
        [_contentView addSubview:pieceTitle];
        for (int r=0;r<5;++r) for (int c=0;c<5;++c) {
            int index=r*5+c;
            UIButton *tile=[self roundedButton:@"" frame:CGRectMake(pieceX+c*kPiecePitch,403+r*kPiecePitch,17,17) action:@selector(togglePieceTile:)];
            tile.tag=p*25+index;
            tile.layer.cornerRadius=2;
            [_contentView addSubview:tile];
            _pieceButtons[p][index]=tile;
        }
    }
    UIButton *solve=[self roundedButton:@"✨ SOLVE CURRENT THREE PIECES" frame:CGRectMake(12,510,_panelWidth-24,39) action:@selector(solveBoard)];
    solve.backgroundColor=BLColor(14,118,110);
    solve.titleLabel.font=[UIFont boldSystemFontOfSize:14];
    [_contentView addSubview:solve];
    UIButton *calibrate=[self roundedButton:@"📍 Calibrate" frame:CGRectMake(12,555,106,33) action:@selector(startCalibration)];
    UIButton *scan=[self roundedButton:@"📷 Scan board" frame:CGRectMake(125,555,106,33) action:@selector(scanBoard)];
    UIButton *guide=[self roundedButton:@"🎯 On game" frame:CGRectMake(238,555,_panelWidth-250,33) action:@selector(showOnGame)];
    [_contentView addSubview:calibrate]; [_contentView addSubview:scan]; [_contentView addSubview:guide];
    _autoButton=[self roundedButton:@"⚡ AUTO SCAN: ON (tap to pause)" frame:CGRectMake(12,596,_panelWidth-24,34) action:@selector(toggleAutomatic)];
    _autoButton.backgroundColor=BLColor(14,118,110);
    [_contentView addSubview:_autoButton];
    _status=[self addLabel:@"v1.1 THEME FAST: Detecting your board colors…" y:638 height:48 font:12];
    _moveLabel=[self addLabel:@"" y:696 height:50 font:13];
    _moveLabel.font=[UIFont boldSystemFontOfSize:13];
    UIButton *prev=[self roundedButton:@"← Previous" frame:CGRectMake(12,756,130,37) action:@selector(previousMove)];
    UIButton *next=[self roundedButton:@"Next move →" frame:CGRectMake(_panelWidth-142,756,130,37) action:@selector(nextMove)];
    [_contentView addSubview:prev]; [_contentView addSubview:next];
    _info=[self addLabel:@"FAST THEME AUTO: wood, blue and other colored boards. Confirm highlights before each move." y:805 height:47 font:10];
    _info.textColor=BLColor(146,168,191);
    _dragStrip=[[UIView alloc] initWithFrame:CGRectZero];
    _dragStrip.backgroundColor=BLColor(38,60,84);
    UILabel *dragText=[[UILabel alloc] initWithFrame:CGRectMake(12,0,270,29)];
    dragText.text=@"⠿ Drag panel     •     Resize from ↘";
    dragText.textColor=UIColor.whiteColor;
    dragText.font=[UIFont boldSystemFontOfSize:12];
    [_dragStrip addSubview:dragText];
    [_panel addSubview:_dragStrip];
    [_dragStrip addGestureRecognizer:[[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(dragPanel:)]];
    _resizeGrip=[[UILabel alloc] initWithFrame:CGRectZero];
    _resizeGrip.text=@"↘";
    _resizeGrip.textAlignment=NSTextAlignmentCenter;
    _resizeGrip.font=[UIFont boldSystemFontOfSize:27];
    _resizeGrip.textColor=BLColor(84,225,193);
    _resizeGrip.backgroundColor=BLColor(40,66,91);
    _resizeGrip.layer.cornerRadius=8;
    _resizeGrip.clipsToBounds=YES;
    _resizeGrip.userInteractionEnabled=YES;
    [_panel addSubview:_resizeGrip];
    [_resizeGrip addGestureRecognizer:[[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(resizePanel:)]];
    _guideLayer=[[UIView alloc] initWithFrame:self.view.bounds];
    _guideLayer.userInteractionEnabled=NO;
    _guideLayer.backgroundColor=UIColor.clearColor;
    [self.view insertSubview:_guideLayer belowSubview:_bubble];
    _nudgeStrip=[[UIView alloc] initWithFrame:CGRectMake(0,0,width,2)];
    _nudgeStrip.userInteractionEnabled=NO;
    _nudgeStrip.hidden=YES;
    _nudgeStrip.autoresizingMask=UIViewAutoresizingFlexibleWidth;
    [self.view addSubview:_nudgeStrip];
    _calibrationLayer=[[UIView alloc] initWithFrame:self.view.bounds];
    _calibrationLayer.backgroundColor=[UIColor colorWithWhite:0 alpha:.06];
    _calibrationLayer.hidden=YES;
    _calibrationLabel=[[UILabel alloc] initWithFrame:CGRectMake(16,53,MAX(100,width-32),70)];
    _calibrationLabel.backgroundColor=BLColor(17,31,51);
    _calibrationLabel.layer.cornerRadius=12;
    _calibrationLabel.clipsToBounds=YES;
    _calibrationLabel.textColor=UIColor.whiteColor;
    _calibrationLabel.font=[UIFont boldSystemFontOfSize:14];
    _calibrationLabel.numberOfLines=3;
    _calibrationLabel.textAlignment=NSTextAlignmentCenter;
    [_calibrationLayer addSubview:_calibrationLabel];
    [self.view addSubview:_calibrationLayer];
    [_calibrationLayer addGestureRecognizer:[[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(tapCalibration:)]];
    NSUserDefaults *prefs=[NSUserDefaults standardUserDefaults];
    if([prefs boolForKey:@"BLCalibratedV2"]) {
        _gridFirst=CGPointMake([prefs doubleForKey:@"BLFirstX"],[prefs doubleForKey:@"BLFirstY"]);
        _gridLast=CGPointMake([prefs doubleForKey:@"BLLastX"],[prefs doubleForKey:@"BLLastY"]);
        _calibrated=YES;
    }
    [self layoutPanel];
    [self refreshTiles];
    _autoEnabled=YES;
    _showGuides=NO;
    // Auto starts without taps after the host game settles; permission may be
    // requested by iOS on first ReplayKit use.
    [self performSelector:@selector(startAutomatic) withObject:nil afterDelay:2.0];
}

#pragma mark - Resizable, movable panel
- (void)layoutPanel {
    CGFloat w=_panel.bounds.size.width,h=_panel.bounds.size.height;
    _dragStrip.frame=CGRectMake(0,0,w,30);
    _resizeGrip.frame=CGRectMake(w-40,h-40,40,40);
    _scroll.frame=CGRectMake(0,30,w,h-30);
    CGFloat ratio=w/_panelWidth;
    _contentView.transform=CGAffineTransformMakeScale(ratio,ratio);
    _scroll.contentSize=CGSizeMake(w,868*ratio);
}
- (void)savePanelFrame {
    [[NSUserDefaults standardUserDefaults] setObject:NSStringFromCGRect(_panel.frame) forKey:@"BLPanelFrameV2"];
}
- (void)dragPanel:(UIPanGestureRecognizer *)pan {
    CGPoint d=[pan translationInView:self.view];
    CGRect f=_panel.frame;
    f.origin.x=MAX(0,MIN(self.view.bounds.size.width-f.size.width,f.origin.x+d.x));
    f.origin.y=MAX(30,MIN(self.view.bounds.size.height-f.size.height-8,f.origin.y+d.y));
    _panel.frame=f;
    [pan setTranslation:CGPointZero inView:self.view];
    if(pan.state==UIGestureRecognizerStateEnded) [self savePanelFrame];
}
- (void)resizePanel:(UIPanGestureRecognizer *)pan {
    CGPoint d=[pan translationInView:self.view];
    CGRect f=_panel.frame;
    CGFloat maxW=MAX(305,self.view.bounds.size.width-f.origin.x-8);
    CGFloat maxH=MAX(245,self.view.bounds.size.height-f.origin.y-8);
    f.size.width=MAX(305,MIN(maxW,f.size.width+d.x));
    f.size.height=MAX(245,MIN(maxH,f.size.height+d.y));
    _panel.frame=f;
    [pan setTranslation:CGPointZero inView:self.view];
    [self layoutPanel];
    if(pan.state==UIGestureRecognizerStateEnded) [self savePanelFrame];
}
#pragma mark - One-time board geometry calibration
- (void)startCalibration {
    _autoEnabled=NO;
    [_autoButton setTitle:@"⚡ AUTO SCAN: PAUSED (tap to resume)" forState:UIControlStateNormal];
    _showCalibrationGrid=NO;
    _panel.hidden=YES;
    _bubble.hidden=YES;
    _guideLayer.hidden=YES;
    _calibrationStage=0;
    _calibrationLayer.hidden=NO;
    _calibrationLabel.text=@"Tap the CENTER of the TOP-LEFT board cell.\n(1 of 2)";
}
- (void)tapCalibration:(UITapGestureRecognizer *)tap {
    CGPoint point=[tap locationInView:self.view];
    if(_calibrationStage==0) {
        _calibrationFirst=point;
        _calibrationStage=1;
        _calibrationLabel.text=@"Now tap CENTER of BOTTOM-RIGHT board cell.\n(2 of 2)";
        return;
    }
    _calibrationLayer.hidden=YES;
    _bubble.hidden=NO;
    if(point.x-_calibrationFirst.x<70||point.y-_calibrationFirst.y<70) {
        _panel.hidden=NO;
        _status.text=@"Calibration failed. Tap top-left, then bottom-right.";
        return;
    }
    _gridFirst=_calibrationFirst;
    _gridLast=point;
    _calibrated=YES;
    NSUserDefaults *prefs=[NSUserDefaults standardUserDefaults];
    [prefs setBool:YES forKey:@"BLCalibratedV2"];
    [prefs setDouble:_gridFirst.x forKey:@"BLFirstX"];
    [prefs setDouble:_gridFirst.y forKey:@"BLFirstY"];
    [prefs setDouble:_gridLast.x forKey:@"BLLastX"];
    [prefs setDouble:_gridLast.y forKey:@"BLLastY"];
    _status.text=@"Calibration saved. Check grid overlay, then Scan board.";
    _showCalibrationGrid=YES;
    _panel.hidden=YES;
    [self refreshGuides];
}
- (void)checkCalibration {
    if(!_calibrated) { _status.text=@"Calibrate the top-left and bottom-right square centers first."; return; }
    _showCalibrationGrid=YES;
    _showGuides=NO;
    [self refreshGuides];
    _panel.hidden=YES;
    _bubble.hidden=NO;
}
- (void)scanBoard {
    if(!_calibrated) {
        _status.text=@"Tap Calibrate to mark the board before scanning.";
        return;
    }
    _status.text=@"Scanning game screen…";
    _showCalibrationGrid=NO;
    _panel.hidden=YES;
    _bubble.hidden=YES;
    _guideLayer.hidden=YES;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(0.35*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
        BLCaptureScreen(^(UIImage *image,NSError *error) {
            self->_bubble.hidden=NO;
            self->_panel.hidden=NO;
            if(!image||error) {
                self->_status.text=[NSString stringWithFormat:@"Screen capture unavailable: %@",
                                     error.localizedDescription?:@"Enter board manually"];
                [self refreshGuides];
                return;
            }
            BOOL scanned[64]={0};
            NSString *detail=nil;
            if(BLDetectBoard(image,self->_gridFirst,self->_gridLast,
                             self.view.bounds.size,scanned,&detail)) {
                memcpy(self->_board,scanned,sizeof(scanned));
                self->_previewing=NO;
                self->_showGuides=NO;
                self->_moveLabel.text=@"";
                [self refreshTiles];
            }
            self->_status.text=detail?:@"Could not read screen. Enter board manually.";
            [self refreshGuides];
        });
    });
}
#pragma mark - Pass-through highlights aligned to board
- (void)showOnGame {
    _showCalibrationGrid=NO;
    if(!_calibrated) { _status.text=@"Calibrate first to align the on-game highlights."; return; }
    if(!_previewing) { _status.text=@"Enter 3 pieces and Solve before showing highlights."; return; }
    _showGuides=YES;
    [self refreshGuides];
    _panel.hidden=YES;
}
- (void)refreshGuides {
    for(UIView *v in [_guideLayer.subviews copy]) [v removeFromSuperview];
    _guideLayer.frame=self.view.bounds;
    BOOL showGrid=(_calibrated&&_showCalibrationGrid);
    BOOL showMove=(_calibrated&&_showGuides&&_previewing&&_solution.movesFound>0);
    _guideLayer.hidden=!(showGrid||showMove);
    if(_guideLayer.hidden) return;
    CGFloat dx=(_gridLast.x-_gridFirst.x)/7.0;
    CGFloat dy=(_gridLast.y-_gridFirst.y)/7.0;
    if(dx<=2||dy<=2) { _guideLayer.hidden=YES;return; }
    if(showGrid) {
        for(int row=0;row<8;++row) for(int col=0;col<8;++col) {
            CGFloat x=_gridFirst.x+dx*col;
            CGFloat y=_gridFirst.y+dy*row;
            UIView *tile=[[UIView alloc] initWithFrame:CGRectMake(x-dx*.47,y-dy*.47,dx*.94,dy*.94)];
            tile.backgroundColor=[UIColor colorWithRed:0 green:1 blue:.8 alpha:.06];
            tile.layer.borderWidth=1.3;
            tile.layer.borderColor=[UIColor colorWithRed:0 green:1 blue:.8 alpha:.88].CGColor;
            tile.userInteractionEnabled=NO;
            [_guideLayer addSubview:tile];
            if((row==0&&col==0)||(row==7&&col==7)) {
                UILabel *label=[[UILabel alloc] initWithFrame:tile.bounds];
                label.text=(row==0?@"1":@"64");
                label.textAlignment=NSTextAlignmentCenter;
                label.font=[UIFont boldSystemFontOfSize:16];
                label.textColor=UIColor.yellowColor;
                label.userInteractionEnabled=NO;
                [tile addSubview:label];
            }
        }
        return;
    }
    BLMove move=_solution.moves[_previewIndex];
    BLPiece piece=_normalizedPieces[move.pieceIndex];
    for(int i=0;i<piece.count;++i) {
        CGFloat x=_gridFirst.x+dx*(move.col+piece.x[i]);
        CGFloat y=_gridFirst.y+dy*(move.row+piece.y[i]);
        UIView *tile=[[UIView alloc] initWithFrame:CGRectMake(x-dx*.44,y-dy*.44,dx*.88,dy*.88)];
        // Border only: leave sampled cell centers untouched for automatic recognition.
        tile.backgroundColor=UIColor.clearColor;
        tile.layer.borderWidth=2.2;
        tile.layer.borderColor=UIColor.yellowColor.CGColor;
        tile.layer.cornerRadius=4;
        tile.userInteractionEnabled=NO;
        [_guideLayer addSubview:tile];
    }
}


#pragma mark - Fully automatic recognition & live placement overlay
// The same recommendation should not recreate all outlined UIKit tile views
// at every scan: just unhide the existing pass-through guide layer.
- (void)restoreGuidesAfterCapture {
    if(_calibrated && _showGuides && _previewing &&
       !_showCalibrationGrid && _solution.movesFound>0 &&
       _guideLayer.subviews.count>0) {
        _guideLayer.hidden=NO;
    } else [self refreshGuides];
}
- (void)startAutomatic {
    if(_autoTimer) return;
    _autoEnabled=YES;
    _lastRestartCount=BLCaptureRestartCount();
    _lastFrameCounter=BLCaptureFrameCounter();
    _autoTimer=[NSTimer scheduledTimerWithTimeInterval:kAutoScanInterval
        target:self selector:@selector(autoTick) userInfo:nil repeats:YES];
    [self autoTick];
}
// Drop EVERYTHING derived from earlier frames: board, tray, solution, highlight,
// confirmation buffer and the "last solved" cache. Called on any error, restart,
// uncertain vision result or stale frame so old answers are never repainted.
- (void)invalidateDetection:(NSString *)reason bubble:(NSString *)bubbleTitle {
    _previewing=NO;
    _showGuides=NO;
    _previewIndex=0;
    memset(&_solution,0,sizeof(_solution));
    memset(_normalizedPieces,0,sizeof(_normalizedPieces));
    memset(_pieces,0,sizeof(_pieces));
    memset(_board,0,sizeof(_board));
    _stableAutoFrames=0;
    _observedAutoHash=0;
    _solvedAutoHash=0;
    ++_solveGeneration;
    _moveLabel.text=@"";
    if(reason) _status.text=reason;
    if(bubbleTitle) [_bubble setTitle:bubbleTitle forState:UIControlStateNormal];
    [self refreshTiles];
    [self refreshGuides];
}
- (void)toggleAutomatic {
    _autoEnabled=!_autoEnabled;
    _stableAutoFrames=0;
    if(!_autoEnabled) BLStopScreenCapture();
    else { _autoRetryAfter=0; [self invalidateDetection:nil bubble:@"AI …"]; }
    _autoErrors=0;
    _frozenRepeats=0;
    NSString *title=_autoEnabled?@"⚡ AUTO SCAN: ON (tap to pause)":@"⚡ AUTO SCAN: PAUSED (tap to resume)";
    [_autoButton setTitle:title forState:UIControlStateNormal];
    _autoButton.backgroundColor=_autoEnabled?BLColor(14,118,110):BLColor(91,81,70);
    _status.text=_autoEnabled?@"Auto scanning resumes when you close this panel.":@"Auto paused. Manual grid editing is available.";
    if(!_autoEnabled) {
        _showGuides=NO;
        [self refreshGuides];
    }
}
- (void)autoTick {
    if(!_autoEnabled || _autoBusy || !_panel.hidden || !_calibrationLayer.hidden) return;
    if(_autoRetryAfter>0 && CFAbsoluteTimeGetCurrent()<_autoRetryAfter) return;
    if([UIApplication sharedApplication].applicationState!=UIApplicationStateActive) return;
    // 1. Capture was restarted since the last frame we trusted: forget everything.
    unsigned long restarts=BLCaptureRestartCount();
    if(restarts!=_lastRestartCount) {
        _lastRestartCount=restarts;
        _frozenRepeats=0;
        [self invalidateDetection:@"Reconnecting… capture restarted. Waiting for fresh frame." bubble:@"AI …"];
    }
    // 2. Nothing fresh for too long (app was backgrounded, panel was open, etc.).
    if((_previewing||_showGuides) && _lastFreshAt>0 &&
       CFAbsoluteTimeGetCurrent()-_lastFreshAt>kStaleAfter) {
        [self invalidateDetection:@"AI paused — no fresh frame. Waiting for fresh frame." bubble:@"AI …"];
    }
    _autoBusy=YES;
    // Prevent captured AI text, buttons, and colored outlines from being
    // misidentified as Block Blast pieces. Restore after one video frame.
    _guideLayer.hidden=YES;
    _bubble.hidden=YES;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(kCaptureWarmup*NSEC_PER_SEC)),dispatch_get_main_queue(),^{
      if(!self->_autoEnabled || !self->_panel.hidden) {
          self->_autoBusy=NO;
          self->_bubble.hidden=NO;
          [self restoreGuidesAfterCapture];
          return;
      }
      // Register the frame request FIRST. Only then change the strip to
      // generate a new screen frame; do not nudge if registration fails.
      __block BOOL callbackCompleted=NO;
      BLCaptureScreen(^(UIImage *image,NSError *error) {
        callbackCompleted=YES;
        self->_nudgeStrip.hidden=YES;
        self->_autoBusy=NO;
        self->_bubble.hidden=NO;
        if(!self->_autoEnabled || !self->_panel.hidden) {
            [self refreshGuides];
            return;
        }
        if(!image || error) {
            self->_autoErrors++;
            [self invalidateDetection:[NSString stringWithFormat:@"AI paused — %@ Reconnecting…",
                                       error.localizedDescription?:@"no fresh frame."]
                               bubble:@"AI …"];
            if(self->_autoErrors>=3) {
                self->_autoErrors=0;
                self->_autoRetryAfter=CFAbsoluteTimeGetCurrent()+3.0;
                BLStopScreenCapture(); // full stop; next request starts a clean stream
                self->_status.text=[NSString stringWithFormat:@"Capture interrupted: %@. Restarting capture.",
                                   error.localizedDescription?:@"ReplayKit interrupted"];
            }
            return;
        }
        // 3. Frame identity: this must be a NEW frame from a stream that was not restarted.
        unsigned long counter=BLCaptureFrameCounter();
        unsigned long restartsNow=BLCaptureRestartCount();
        if(restartsNow!=self->_lastRestartCount) {
            self->_lastRestartCount=restartsNow;
            [self invalidateDetection:@"Reconnecting… capture restarted. Waiting for fresh frame." bubble:@"AI …"];
            return;
        }
        if(counter==self->_lastFrameCounter) {
            [self invalidateDetection:@"AI paused — waiting for fresh frame." bubble:@"AI …"];
            return;
        }
        self->_lastFrameCounter=counter;
        // A stationary or empty board is allowed to be identical between
        // independent ReplayKit frames. The monotonic frame counter and capture
        // timeout already reject stale delivery; hashing a still game screen
        // incorrectly triggered restarts and stopped the AI on quiet boards.
        self->_frozenRepeats=0;
        self->_lastFreshAt=CFAbsoluteTimeGetCurrent();
        BOOL board[64]={0};
        BLPiece tray[3]={0};
        CGPoint first=CGPointZero,last=CGPointZero;
        NSString *info=nil;
        if(!BLAutoDetect(image,self.view.bounds.size,board,tray,&first,&last,&info)) {
            // Vision is uncertain: pause the highlight and DROP the old board.
            [self invalidateDetection:info?:@"AI paused — auto detection needs a clear screen." bubble:@"AI …"];
            return;
        }
        self->_autoErrors=0;
        self->_autoRetryAfter=0;
        // Reject anomalous geometry and sudden tray/board contradictions.
        CGFloat pitchX=(last.x-first.x)/7.0, pitchY=(last.y-first.y)/7.0;
        BOOL goodGeom=(pitchX>20 && pitchY>20 &&
            fabs(pitchX-pitchY)<MAX(pitchX,pitchY)*.14 &&
            first.x>0 && first.y>0 &&
            last.x<self.view.bounds.size.width && last.y<self.view.bounds.size.height);
        BOOL sameBoard=YES, sameTray=YES;
        if(self->_solvedAutoHash!=0) {
            for(int i=0;i<64;++i) if(board[i]!=self->_board[i]) {sameBoard=NO;break;}
            for(int p=0;p<3 && sameTray;++p) {
                if(tray[p].count!=self->_normalizedPieces[p].count) {sameTray=NO;break;}
                for(int i=0;i<tray[p].count;++i)
                    if(tray[p].x[i]!=self->_normalizedPieces[p].x[i] ||
                       tray[p].y[i]!=self->_normalizedPieces[p].y[i]) {
                        sameTray=NO;break;
                    }
            }
            if(fabs(first.x-self->_gridFirst.x)>pitchX*.35 ||
               fabs(first.y-self->_gridFirst.y)>pitchY*.35 ||
               fabs(last.x-self->_gridLast.x)>pitchX*.35 ||
               fabs(last.y-self->_gridLast.y)>pitchY*.35) goodGeom=NO;
        }
        BOOL transitional=(self->_solvedAutoHash!=0 && (sameTray != sameBoard));
        uint64_t scannedBoard=0;
        for(int i=0;i<64;++i)
            if(board[i]) scannedBoard |= UINT64_C(1)<<i;
        // Instant-path only for the exact board + tray resulting from the FIRST
        // recommended placement. Any ambiguity still needs confirmation.
        BOOL expectedMove=(self->_previewing && self->_solvedAutoHash!=0 &&
            BLIsExpectedTransition([self boardMask],self->_normalizedPieces,
                                   &self->_solution,scannedBoard,tray));
        if(!goodGeom) {
            [self invalidateDetection:@"AI paused — board coordinates changed unexpectedly. Waiting for a stable capture." bubble:@"AI …"];
            return;
        }
        // Smooth minor detector jitter; large jumps were already rejected.
        // Never smooth a new grid after reconnecting or a board transition.
        if(self->_solvedAutoHash!=0 && sameBoard && sameTray &&
           fabs(first.x-self->_gridFirst.x)<pitchX*.12 &&
           fabs(first.y-self->_gridFirst.y)<pitchY*.12 &&
           fabs(last.x-self->_gridLast.x)<pitchX*.12 &&
           fabs(last.y-self->_gridLast.y)<pitchY*.12) {
            first.x=self->_gridFirst.x*.65+first.x*.35;
            first.y=self->_gridFirst.y*.65+first.y*.35;
            last.x=self->_gridLast.x*.65+last.x*.35;
            last.y=self->_gridLast.y*.65+last.y*.35;
        }
        self->_calibrated=YES;
        self->_gridFirst=first;
        self->_gridLast=last;
        self->_showCalibrationGrid=NO;
        uint64_t signature=UINT64_C(1469598103934665603);
        for(int i=0;i<64;++i) signature=(signature^(uint64_t)board[i])*UINT64_C(1099511628211);
        for(int p=0;p<3;++p) {
            signature=(signature^(uint64_t)tray[p].count)*UINT64_C(1099511628211);
            for(int i=0;i<tray[p].count;++i) {
                signature=(signature^(uint64_t)tray[p].x[i])*UINT64_C(1099511628211);
                signature=(signature^(uint64_t)tray[p].y[i])*UINT64_C(1099511628211);
            }
        }
        // Two independent matching fresh observations for regular changes.
        // Exact expected moves can advance on their first verified frame.
        if(signature==self->_observedAutoHash) self->_stableAutoFrames++;
        else {
            self->_observedAutoHash=signature;
            self->_stableAutoFrames=1;
            self->_showGuides=NO;
        }
        NSInteger required=expectedMove?1:(transitional?kUncertainTransitionFrames:kConfirmFrames);
        if(self->_stableAutoFrames<required) {
            self->_showGuides=NO;
            [self refreshGuides];
            return;
        }
        if(signature!=self->_solvedAutoHash) {
            self->_solvedAutoHash=signature;
            memcpy(self->_board,board,sizeof(board));
            memset(self->_pieces,0,sizeof(self->_pieces));
            memcpy(self->_normalizedPieces,tray,sizeof(tray));
            int visible=0;
            for(int p=0;p<3;++p) {
                visible+=tray[p].count>0;
                for(int i=0;i<tray[p].count;++i) {
                    int x=tray[p].x[i],y=tray[p].y[i];
                    if(x>=0&&x<5&&y>=0&&y<5) self->_pieces[p][y*5+x]=YES;
                }
            }
            self->_previewIndex=0;
            self->_previewing=NO;
            self->_showGuides=NO;
            self->_moveLabel.text=@"⚡ Calculating the safest available move…";
            [self->_bubble setTitle:@"AI ⚡" forState:UIControlStateNormal];
            [self refreshGuides]; // remove stale highlights immediately
            [self refreshTiles];
            // Move the beam solver off UIKit's main thread; never block taps,
            // frame processing, or overlay rendering while calculating.
            uint64_t generation=++self->_solveGeneration;
            uint64_t boardSnapshot=scannedBoard;
            BLTraySnapshot traySnapshot; memcpy(traySnapshot.values,tray,sizeof(traySnapshot.values));
            NSString *scanDetail=[info copy];
            __weak BLController *weakSelf=self;
            dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED,0), ^{
                BLSolution answer=BLSolve(boardSnapshot,traySnapshot.values);
                dispatch_async(dispatch_get_main_queue(), ^{
                    BLController *strongSelf=weakSelf;
                    if(!strongSelf || generation!=strongSelf->_solveGeneration ||
                       signature!=strongSelf->_solvedAutoHash ||
                       !strongSelf->_autoEnabled || !strongSelf->_panel.hidden)return;
                    strongSelf->_solution=answer;
                    strongSelf->_previewing=(answer.movesFound>0);
                    strongSelf->_showGuides=strongSelf->_previewing;
                    strongSelf->_status.text=[NSString stringWithFormat:@"FAST: %@ %d move(s) recommended.",
                                              scanDetail?:@"Board detected.",answer.movesFound];
                    if(strongSelf->_previewing) {
                        [strongSelf showMove];
                        BLMove move=answer.moves[0];
                        [strongSelf->_bubble setTitle:[NSString stringWithFormat:@"P%d ↓",move.pieceIndex+1]
                                              forState:UIControlStateNormal];
                    } else {
                        strongSelf->_moveLabel.text=visible?@"No legal placement found.":@"Waiting for tray pieces…";
                        [strongSelf->_bubble setTitle:@"🧩 AI" forState:UIControlStateNormal];
                        [strongSelf refreshTiles];
                        [strongSelf refreshGuides];
                    }
                });
            });
        } else {
            // Same confirmed state seen again on a fresh frame. Any failure in between
            // would have cleared _previewing, so this can no longer repaint an old answer.
            self->_showGuides=self->_previewing;
            if(self->_previewing && self->_solution.movesFound>0) {
                BLMove move=self->_solution.moves[self->_previewIndex];
                [self->_bubble setTitle:[NSString stringWithFormat:@"P%d ↓",move.pieceIndex+1]
                               forState:UIControlStateNormal];
            }
            [self restoreGuidesAfterCapture];
        }
      });
      // If the frame request was synchronously rejected (e.g. busy),
      // its callback has already run and we must NOT leave the strip visible.
      if(!callbackCompleted && self->_autoEnabled && self->_panel.hidden) {
          self->_nudgeOn=!self->_nudgeOn;
          self->_nudgeStrip.backgroundColor=self->_nudgeOn?
              [UIColor colorWithWhite:0 alpha:.5]:
              [UIColor colorWithWhite:1 alpha:.5];
          self->_nudgeStrip.hidden=NO;
      }
    });
}

- (void)togglePanel {
    _panel.hidden=!_panel.hidden;
    if(!_panel.hidden && _showCalibrationGrid) {
        _showCalibrationGrid=NO; [self refreshGuides];
    }
    if(!_panel.hidden && _autoEnabled) {
        // Auto scan cannot run while the panel covers the tray, so whatever is drawn
        // here may already be out of date (e.g. after you played the suggested move).
        _showGuides=NO; [self refreshGuides];
        _autoLabel.text=@"⏸ AUTO paused while panel is open — board below may be OUT OF DATE";
    } else if(_panel.hidden) {
        _autoLabel.text=@"AUTO: Board + pieces detected automatically (manual fallback below)";
        if(_autoEnabled) {
            // Resuming: never reuse what was shown before; require fresh confirmed frames.
            _autoRetryAfter=0;
            [self invalidateDetection:@"Waiting for fresh frame…" bubble:@"AI …"];
        }
    }
}
- (void)dragBubble:(UIPanGestureRecognizer *)recognizer {
    CGPoint delta=[recognizer translationInView:self.view];
    CGPoint center=_bubble.center;
    center.x=MAX(34,MIN(self.view.bounds.size.width-34,center.x+delta.x));
    center.y=MAX(56,MIN(self.view.bounds.size.height-48,center.y+delta.y));
    _bubble.center=center;
    [recognizer setTranslation:CGPointZero inView:self.view];
}
- (void)toggleBoardTile:(UIButton *)sender {
    if (_previewing) return;
    _board[sender.tag]=!_board[sender.tag];
    [self refreshTiles];
}
- (void)togglePieceTile:(UIButton *)sender {
    if (_previewing) return;
    int p=(int)sender.tag/25, idx=(int)sender.tag%25;
    _pieces[p][idx]=!_pieces[p][idx];
    [self refreshTiles];
}
- (void)clearBoard {
    _previewing=NO; memset(_board,0,sizeof(_board));
    _showGuides=NO;
    [self refreshGuides];
    _status.text=@"Board cleared. Pieces are unchanged.";
    _moveLabel.text=@"";
    [self refreshTiles];
}
- (void)clearAll {
    _previewing=NO;
    memset(_board,0,sizeof(_board));
    memset(_pieces,0,sizeof(_pieces));
    _showGuides=NO;
    [self refreshGuides];
    _status.text=@"Everything cleared. Enter the new board and pieces.";
    _moveLabel.text=@"";
    [self refreshTiles];
}
- (void)editInputs {
    _previewing=NO;
    _showGuides=NO;
    [self refreshGuides];
    _status.text=@"Editing inputs. Tap Solve when you're ready.";
    _moveLabel.text=@"";
    [self refreshTiles];
}
- (uint64_t)boardMask {
    uint64_t mask=0;
    for (int i=0;i<64;++i) if (_board[i]) mask |= UINT64_C(1)<<i;
    return mask;
}
- (BOOL)readPieces {
    for (int p=0;p<3;++p) {
        BLPiece result={0};
        int minX=5, minY=5;
        for (int r=0;r<5;++r) for (int c=0;c<5;++c) {
            if (!_pieces[p][r*5+c]) continue;
            if (c<minX) minX=c;
            if (r<minY) minY=r;
        }
        if (minX==5 || minY==5) {
            _status.text=[NSString stringWithFormat:@"Piece %d is empty. Draw all three pieces before solving.",p+1];
            return NO;
        }
        for (int r=0;r<5;++r) for (int c=0;c<5;++c) {
            if (!_pieces[p][r*5+c]) continue;
            int i=result.count++;
            result.x[i]=c-minX;
            result.y[i]=r-minY;
        }
        _normalizedPieces[p]=result;
    }
    return YES;
}
- (void)solveBoard {
    _previewing=NO;
    _showGuides=NO;
    [self refreshGuides];
    if (![self readPieces]) return;
    _solution=BLSolve([self boardMask],_normalizedPieces);
    if (_solution.movesFound==0) {
        _status.text=@"No legal placement found. Check the board and piece shapes.";
        _moveLabel.text=@"";
        [self refreshTiles];
        return;
    }
    _previewing=YES;
    _previewIndex=0;
    _status.text=[NSString stringWithFormat:@"Suggested sequence: %d/3 pieces, %d line(s) cleared. Preview below.",_solution.movesFound,_solution.totalLines];
    [self showMove];
}
- (void)showMove {
    if (!_previewing || _solution.movesFound==0) return;
    BLMove m=_solution.moves[_previewIndex];
    _moveLabel.text=[NSString stringWithFormat:@"Step %d/%d: PIECE %d → ROW %d, COLUMN %d\n(%d line(s) clear after this move)",_previewIndex+1,_solution.movesFound,m.pieceIndex+1,m.row+1,m.col+1,m.linesCleared];
    [self refreshTiles];
    [self refreshGuides];
}
- (void)nextMove {
    if (!_previewing) return;
    if (_previewIndex+1 < _solution.movesFound) ++_previewIndex;
    [self showMove];
}
- (void)previousMove {
    if (!_previewing) return;
    if (_previewIndex>0) --_previewIndex;
    [self showMove];
}
- (void)refreshTiles {
    uint64_t occupied=0, suggestion=0;
    if (_previewing && _solution.movesFound>0) {
        BLMove m=_solution.moves[_previewIndex];
        occupied=m.before;
        BLPiece piece=_normalizedPieces[m.pieceIndex];
        for(int i=0;i<piece.count;++i) suggestion |= UINT64_C(1) << ((m.row+piece.y[i])*8+(m.col+piece.x[i]));
    }
    for (int i=0;i<64;++i) {
        BOOL filled=_previewing ? !!(occupied & (UINT64_C(1)<<i)) : _board[i];
        BOOL highlighted=_previewing && !!(suggestion & (UINT64_C(1)<<i));
        _boardButtons[i].backgroundColor=highlighted ? BLColor(248,181,57) : (filled ? BLColor(43,200,178) : BLColor(43,58,78));
        _boardButtons[i].enabled=!_previewing;
    }
    for (int p=0;p<3;++p) for (int idx=0;idx<25;++idx) {
        _pieceButtons[p][idx].backgroundColor=_pieces[p][idx] ? BLColor(95,172,255) : BLColor(43,58,78);
        _pieceButtons[p][idx].enabled=!_previewing;
    }
}
@end

static BLPassThroughWindow *sOverlayWindow;
static void BLInstallOverlay(void) {
    if (sOverlayWindow) return;
    UIApplication *app=[UIApplication sharedApplication];
    UIWindowScene *activeScene=nil;
    for (UIScene *scene in app.connectedScenes) {
        if (![scene isKindOfClass:[UIWindowScene class]]) continue;
        if (scene.activationState==UISceneActivationStateForegroundActive) {
            activeScene=(UIWindowScene *)scene;
            break;
        }
        if (!activeScene) activeScene=(UIWindowScene *)scene;
    }
    if (!activeScene) return;
    sOverlayWindow=[[BLPassThroughWindow alloc] initWithWindowScene:activeScene];
    sOverlayWindow.frame=activeScene.coordinateSpace.bounds;
    sOverlayWindow.backgroundColor=UIColor.clearColor;
    sOverlayWindow.windowLevel=UIWindowLevelAlert+5.0;
    sOverlayWindow.rootViewController=[BLController new];
    sOverlayWindow.hidden=NO;
}
__attribute__((constructor)) static void BLStart(void) {
    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSNotificationCenter defaultCenter] addObserverForName:UISceneDidActivateNotification object:nil queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification *note) {
            BLInstallOverlay();
        }];
        [[NSNotificationCenter defaultCenter] addObserverForName:UIApplicationDidBecomeActiveNotification object:nil queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification *note) {
            BLInstallOverlay();
        }];
        // Wait until Block Blast has created its UIWindowScene.
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(2*NSEC_PER_SEC)),dispatch_get_main_queue(),^{ BLInstallOverlay(); });
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(5*NSEC_PER_SEC)),dispatch_get_main_queue(),^{ BLInstallOverlay(); });
    });
}