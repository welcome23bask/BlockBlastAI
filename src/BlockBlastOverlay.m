// BlockBlastAI v0.1 — an experimental, in-app, manually entered Block Blast solver overlay.
// Built only with public UIKit APIs. No automated screen reading, touch automation, or game hooks.
// Designed for injection into a compatible, properly signed iPhone app by its owner.
#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#import "BlockBlastSolver.h"
#include <stdint.h>

static UIColor *BLColor(CGFloat r, CGFloat g, CGFloat b) {
    return [UIColor colorWithRed:r/255.0 green:g/255.0 blue:b/255.0 alpha:1.0];
}
static const CGFloat kBoardPitch=30.0;
static const CGFloat kPiecePitch=19.0;

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
    [_scroll addSubview:label];
    return label;
}
- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor=UIColor.clearColor;
    CGFloat width=self.view.bounds.size.width;
    CGFloat height=self.view.bounds.size.height;
    if (width<100) width=390;
    if (height<100) height=844;
    _panelWidth=MIN(356, width-24);

    _bubble=[self roundedButton:@"🧩 AI" frame:CGRectMake(width-72,110,66,46) action:@selector(togglePanel)];
    _bubble.layer.cornerRadius=22;
    _bubble.layer.borderColor=BLColor(52,211,153).CGColor;
    _bubble.layer.borderWidth=2;
    _bubble.backgroundColor=BLColor(13,23,40);
    _bubble.titleLabel.font=[UIFont boldSystemFontOfSize:15];
    [self.view addSubview:_bubble];
    UIPanGestureRecognizer *pan=[[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(dragBubble:)];
    [_bubble addGestureRecognizer:pan];

    _panel=[[UIView alloc] initWithFrame:CGRectMake(12,65,_panelWidth,MIN(690,height-88))];
    _panel.layer.cornerRadius=16;
    _panel.layer.borderWidth=1;
    _panel.layer.borderColor=BLColor(63,84,106).CGColor;
    _panel.clipsToBounds=YES;
    _panel.backgroundColor=BLColor(18,30,48);
    [self.view addSubview:_panel];
    _panel.hidden=YES;

    _scroll=[[UIScrollView alloc] initWithFrame:_panel.bounds];
    _scroll.contentSize=CGSizeMake(_panelWidth,745);
    _scroll.alwaysBounceVertical=YES;
    [_panel addSubview:_scroll];
    UILabel *title=[self addLabel:@"BLOCK BLAST AI · MOVE ASSISTANT" y:13 height:24 font:16];
    title.font=[UIFont boldSystemFontOfSize:16];
    UIButton *close=[self roundedButton:@"×" frame:CGRectMake(_panelWidth-41,10,29,28) action:@selector(togglePanel)];
    [_scroll addSubview:close];
    [self addLabel:@"Tap each occupied square on your 8×8 board" y:42 height:22 font:12];

    CGFloat x0=(_panelWidth-8*kBoardPitch)/2;
    for (int r=0;r<8;++r) for (int c=0;c<8;++c) {
        int idx=r*8+c;
        UIButton *tile=[self roundedButton:@"" frame:CGRectMake(x0+c*kBoardPitch,69+r*kBoardPitch,28,28) action:@selector(toggleBoardTile:)];
        tile.tag=idx;
        tile.layer.cornerRadius=4;
        [_scroll addSubview:tile];
        _boardButtons[idx]=tile;
    }
    UIButton *clearBoard=[self roundedButton:@"Clear board" frame:CGRectMake(12,315,105,30) action:@selector(clearBoard)];
    [_scroll addSubview:clearBoard];
    UIButton *edit=[self roundedButton:@"Edit inputs" frame:CGRectMake(125,315,102,30) action:@selector(editInputs)];
    [_scroll addSubview:edit];
    UIButton *clearAll=[self roundedButton:@"Reset all" frame:CGRectMake(235,315,_panelWidth-247,30) action:@selector(clearAll)];
    [_scroll addSubview:clearAll];
    [self addLabel:@"Draw the 3 pieces in their current orientation:" y:357 height:19 font:12];

    CGFloat pieceGridWidth=5*kPiecePitch;
    CGFloat pieceGap=(_panelWidth-3*pieceGridWidth)/4;
    for (int p=0;p<3;++p) {
        CGFloat pieceX=pieceGap+(pieceGridWidth+pieceGap)*p;
        UILabel *pieceTitle=[[UILabel alloc] initWithFrame:CGRectMake(pieceX,381,pieceGridWidth,20)];
        pieceTitle.text=[NSString stringWithFormat:@"PIECE %d",p+1];
        pieceTitle.font=[UIFont boldSystemFontOfSize:11];
        pieceTitle.textAlignment=NSTextAlignmentCenter;
        pieceTitle.textColor=BLColor(164,190,213);
        [_scroll addSubview:pieceTitle];
        for (int r=0;r<5;++r) for (int c=0;c<5;++c) {
            int index=r*5+c;
            UIButton *tile=[self roundedButton:@"" frame:CGRectMake(pieceX+c*kPiecePitch,403+r*kPiecePitch,17,17) action:@selector(togglePieceTile:)];
            tile.tag=p*25+index;
            tile.layer.cornerRadius=2;
            [_scroll addSubview:tile];
            _pieceButtons[p][index]=tile;
        }
    }
    UIButton *solve=[self roundedButton:@"✨ SOLVE CURRENT THREE PIECES" frame:CGRectMake(12,510,_panelWidth-24,39) action:@selector(solveBoard)];
    solve.backgroundColor=BLColor(14,118,110);
    solve.titleLabel.font=[UIFont boldSystemFontOfSize:14];
    [_scroll addSubview:solve];
    _status=[self addLabel:@"Enter your board and all 3 pieces, then tap Solve." y:556 height:42 font:12];
    _moveLabel=[self addLabel:@"" y:602 height:47 font:13];
    _moveLabel.font=[UIFont boldSystemFontOfSize:13];
    UIButton *prev=[self roundedButton:@"← Previous" frame:CGRectMake(12,657,130,37) action:@selector(previousMove)];
    UIButton *next=[self roundedButton:@"Next move →" frame:CGRectMake(_panelWidth-142,657,130,37) action:@selector(nextMove)];
    [_scroll addSubview:prev]; [_scroll addSubview:next];
    _info=[self addLabel:@"Manual preview only. Recommendations estimate survival; future pieces are unknown." y:707 height:32 font:10];
    _info.textColor=BLColor(146,168,191);
    [self refreshTiles];
}
- (void)togglePanel { _panel.hidden=!_panel.hidden; }
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
    _status.text=@"Board cleared. Pieces are unchanged.";
    _moveLabel.text=@"";
    [self refreshTiles];
}
- (void)clearAll {
    _previewing=NO;
    memset(_board,0,sizeof(_board));
    memset(_pieces,0,sizeof(_pieces));
    _status.text=@"Everything cleared. Enter the new board and pieces.";
    _moveLabel.text=@"";
    [self refreshTiles];
}
- (void)editInputs {
    _previewing=NO;
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
