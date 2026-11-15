#include "BLVisionCore.h"
#include "BLAdaptiveTheme.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Block Blast's dark-navy board, separated from the lighter blue background.
// Every color check operates in RGB bytes; no screen pixel is sent off-device.
static inline const uint8_t *pixel(const uint8_t *im,int w,int h,int x,int y,int flip) {
    return im + 4*((size_t)(flip ? h-1-y : y)*(size_t)w+(size_t)x);
}
static int dark(const uint8_t *p) {
    return p[0]<53 && p[1]<74 && p[2]<115;
}
static int block(const uint8_t *p) {
    int r=p[0],g=p[1],b=p[2];
    // Cyan, deep blue, red/orange, green, and yellow. In particular, the
    // blue sky (about RGB 57,85,150) MUST NOT be classified as a piece.
    return (g>114 && b>143 && b-r>32)
        || (b>171 && b-r>65)
        || (r>135 && r-g>35 && r-b>32)
        || (g>112 && g-r>30 && g-b>10)
        || (r>154 && g>117 && r-b>52)
        // Purple blocks were missing from the initial screenshot palette.
        || (b>125 && r>75 && b-g>36 && r-g>25 && b>r*.90);
}
static int block_center(const uint8_t *im,int w,int h,int x,int y,int flip) {
    if(x<3||y<3||x>=w-3||y>=h-3) return 0;
    int count=0;
    for(int dy=-2;dy<=2;dy+=2) for(int dx=-2;dx<=2;dx+=2)
        count+=block(pixel(im,w,h,x+dx,y+dy,flip));
    return count>=5;
}
static int locate(const uint8_t *im,int w,int h,int flip,int *left,int *top,int *side) {
    int first=-1;
    const int low=(int)(h*.18), high=(int)(h*.40);
    const int from=(int)(w*.025),to=(int)(w*.975);
    for(int y=low;y<high && y+4<h;++y) {
        int xmin=w,xmax=-1,n=0;
        for(int x=from;x<to;++x) {
            if(dark(pixel(im,w,h,x,y,flip))) {
                if(x<xmin)xmin=x;
                if(x>xmax)xmax=x;
                ++n;
            }
        }
        if(n<w*.09 || xmax-xmin<w*.75) continue;
        int stable=1;
        for(int offset=2;offset<=4;offset+=2) {
            int count=0;
            for(int x=(int)(w*.06);x<(int)(w*.94);x+=2)
                count+=dark(pixel(im,w,h,x,y+offset,flip));
            if(count<(int)(w*.09/2.0)) stable=0;
        }
        if(stable) {first=y;break;}
    }
    if(first<0) return 0;
    int x0=w,x1=-1;
    for(int y=first+5;y<first+17 && y<h;y+=3)
        for(int x=0;x<w;++x) {
            if(!dark(pixel(im,w,h,x,y,flip))) continue;
            if(x<x0)x0=x;
            if(x>x1)x1=x;
        }
    if(x1-x0<w*.74||x1-x0>w*1.05) return 0;
    int s=x1-x0+1;
    int t=first-2;
    if(t<0 || t+s>=h || s<(int)(w*.74)) return 0;
    // A real grid must still have dark cells close to its lower edge;
    // a toolbar, dialog or loading screen should not pass this check.
    int lowerDark=0;
    for(int x=(int)(w*.1);x<(int)(w*.9);x+=4)
        lowerDark+=dark(pixel(im,w,h,x,t+s-14,flip));
    if(lowerDark<5) return 0;
    *left=x0;*top=t;*side=s;
    return 1;
}
static int scan(const uint8_t *im,int w,int h,int flip,BLVisionResult *out) {
    int left=0,top=0,side=0;
    if(!locate(im,w,h,flip,&left,&top,&side))return 0;
    double cell=(double)side/8.0;
    BLVisionResult found;
    memset(&found,0,sizeof(found));
    found.flippedInput=flip;
    found.firstX=(float)(left+cell*.5);
    found.firstY=(float)(top+cell*.5);
    found.lastX=(float)(left+cell*7.5);
    found.lastY=(float)(top+cell*7.5);
    for(int r=0;r<8;++r)for(int c=0;c<8;++c) {
        int x=(int)lround(left+(c+.5)*cell);
        int y=(int)lround(top+(r+.5)*cell);
        if(block_center(im,w,h,x,y,flip))
            found.board|=UINT64_C(1)<<(r*8+c);
    }
    const double small=cell*.44;
    if(small<8.0||small>80.0)return 0;
    int trayTop=(int)(top+side+side*.07);
    int trayBottom=(int)fmin(h*.95,top+side+side*.70);
    if(trayTop<0||trayBottom<=trayTop+20||trayBottom>=h) return 0;
    for(int p=0;p<3;++p) {
        int xmin=w,xmax=-1,ymin=h,ymax=-1,n=0;
        int xfrom=p*w/3,xto=(p+1)*w/3;
        for(int y=trayTop;y<trayBottom;++y)for(int x=xfrom;x<xto;++x) {
            if(!block(pixel(im,w,h,x,y,flip)))continue;
            if(x<xmin)xmin=x;
            if(x>xmax)xmax=x;
            if(y<ymin)ymin=y;
            if(y>ymax)ymax=y;
            ++n;
        }
        if(n<30)continue; // After a move one or two slots are empty.
        int cols=(int)lround((xmax-xmin+1)/small);
        int rows=(int)lround((ymax-ymin+1)/small);
        if(cols<1||cols>5||rows<1||rows>5)return 0;
        BLPiece shape={0};
        for(int r=0;r<rows;++r)for(int c=0;c<cols;++c) {
            int x=(int)lround(xmin+(c+.5)*small);
            int y=(int)lround(ymin+(r+.5)*small);
            if(!block_center(im,w,h,x,y,flip))continue;
            if(shape.count>=25)return 0;
            shape.x[shape.count]=c;
            shape.y[shape.count]=r;
            shape.count++;
        }
        if(shape.count==0)return 0;
        found.pieces[p]=shape;
        ++found.detectedPieces;
    }
    // A scan without any identifiable tray pieces is not actionable.
    // It is safer to show no highlight than to hallucinate a move.
    if(found.detectedPieces<1) return 0;
    *out=found;
    return 1;
}
// The real Block Blast board is centered around 44% of portrait screen height,
// with the tray BELOW it. A vertically mirrored screenshot can also look like
// a board: the real score/crown/header then appears in the fake "tray" region.
// Never select the image orientation by counting apparent tray pieces: that
// exact rule mirrored the board and turned a vertical 1x4 into a horizontal 4x1.
// This anchor is intentionally conservative; if geometry is ambiguous, do not
// suggest a placement instead of guessing and highlighting an illegal move.
static float board_anchor_error(const BLVisionResult *v,int height) {
    float center=(v->firstY+v->lastY)/(2.0f*(float)height);
    return fabsf(center-0.44f);
}
// Strict optional fallback for solid BLACK filled squares on a dark-blue
// board, with warm/orange tray pieces. Unlike the existing bright-palette
// detector, this measures luminance and the grid seam color. It runs before
// generic scanners, so a partial scan cannot silently omit black cells.
static int black_block_center(const uint8_t *im,int w,int h,int x,int y,int flip,
                              int seamR,int seamG,int seamB) {
    if(x<3||y<3||x>=w-3||y>=h-3)return 0;
    int hits=0;
    for(int dy=-2;dy<=2;dy+=2)for(int dx=-2;dx<=2;dx+=2){
        const uint8_t *p=pixel(im,w,h,x+dx,y+dy,flip);
        hits+=(p[0]<39 && p[1]<45 && p[2]<53 &&
               (int)p[0]+p[1]+p[2] < seamR+seamG+seamB-75);
    }
    return hits>=7;
}
static int warm_tray_pixel(const uint8_t *p) {
    // Matches orange/red brick pieces, not the blue sky background.
    return p[0]>=142 && (int)p[0]-(int)p[1]>=38 &&
           (int)p[1]-(int)p[2]>=10 && p[1]>=45 && p[2]<155;
}
static int black_theme_scan(const uint8_t *im,int w,int h,int flip,BLVisionResult *out) {
    int x0,y0,side;
    if(!locate(im,w,h,flip,&x0,&y0,&side))return 0;
    double cell=(double)side/8.0,small=cell*.44;
    if(small<8||small>80)return 0;
    if(fabs((y0+side/2.0)/h-0.44)>0.085)return 0;
    // Use grid seams, not filled block centers, as the board background.
    int seam[3]={0};
    for(int row=0;row<8;++row)for(int col=1;col<8;++col){
        int x=x0+(int)lround(col*cell),y=y0+(int)lround((row+.5)*cell);
        const uint8_t *p=pixel(im,w,h,x,y,flip);
        for(int k=0;k<3;k++)seam[k]+=p[k];
    }
    for(int k=0;k<3;k++)seam[k]/=56;
    // Strict blue/navy guard ensures we do NOT alter pink, green or wood skins.
    if(seam[2]<seam[1]+8 || seam[2]<seam[0]+24 ||
       seam[0]+seam[1]+seam[2]<100 || seam[0]+seam[1]+seam[2]>400)return 0;
    BLVisionResult result={0};
    result.flippedInput=flip;
    result.firstX=x0+cell*.5;result.firstY=y0+cell*.5;
    result.lastX=x0+cell*7.5;result.lastY=y0+cell*7.5;
    int black=0;
    for(int row=0;row<8;++row)for(int col=0;col<8;++col){
        int x=x0+(int)lround((col+.5)*cell);
        int y=y0+(int)lround((row+.5)*cell);
        if(black_block_center(im,w,h,x,y,flip,seam[0],seam[1],seam[2])){
            result.board|=UINT64_C(1)<<(row*8+col);++black;
        }
    }
    // Never interpret ordinary dark-blue empty cells as a black-piece board.
    if(black<1)return 0;
    int trayTop=(int)(y0+side+side*.07);
    int trayBottom=(int)fmin(h*.95,y0+side+side*.70);
    if(trayTop<0||trayBottom<=trayTop+20||trayBottom>=h)return 0;
    for(int slot=0;slot<3;++slot) {
        int xMin=w,xMax=-1,yMin=h,yMax=-1,n=0;
        for(int y=trayTop;y<trayBottom;++y)for(int x=slot*w/3;x<(slot+1)*w/3;++x){
            if(!warm_tray_pixel(pixel(im,w,h,x,y,flip)))continue;
            if(x<xMin)xMin=x;if(x>xMax)xMax=x;
            if(y<yMin)yMin=y;if(y>yMax)yMax=y;
            ++n;
        }
        if(n<60)continue; // Empty slots are valid immediately after moves.
        int cols=(int)lround((xMax-xMin+1)/small);
        int rows=(int)lround((yMax-yMin+1)/small);
        if(cols<1||cols>5||rows<1||rows>5)return 0;
        BLPiece p={0};
        for(int row=0;row<rows;++row)for(int col=0;col<cols;++col){
            int x=(int)lround(xMin+(col+.5)*small);
            int y=(int)lround(yMin+(row+.5)*small);
            int hits=0;
            for(int dy=-2;dy<=2;dy+=2)for(int dx=-2;dx<=2;dx+=2)
                hits+=warm_tray_pixel(pixel(im,w,h,x+dx,y+dy,flip));
            if(hits<5)continue;
            if(p.count>=25)return 0;
            p.x[p.count]=col;p.y[p.count]=row;++p.count;
        }
        if(!p.count)return 0;
        result.pieces[slot]=p;++result.detectedPieces;
    }
    if(result.detectedPieces==0)return 0;
    *out=result;
    return 1;
}
int BLVisionAnalyzeRGBA(const uint8_t *rgba,int width,int height,BLVisionResult *result) {
    if(!rgba||!result||width<240||height<350||width>1600||height>3000)return 0;
    // Black-on-navy skin requires a darkness-relative check before the old
    // bright palette can incorrectly accept the tray but omit black cells.
    BLVisionResult blackNormal={0},blackFlipped={0};
    int blackA=black_theme_scan(rgba,width,height,0,&blackNormal);
    int blackB=black_theme_scan(rgba,width,height,1,&blackFlipped);
    if(blackA||blackB){
        if(blackA&&blackB)return 0; // ambiguous orientation: refuse to guess
        *result=blackA?blackNormal:blackFlipped;
        return 1;
    }
    BLVisionResult normal,upsideDown;
    int a=scan(rgba,width,height,0,&normal);
    int b=scan(rgba,width,height,1,&upsideDown);
    if(!a&&!b) return BLAdaptiveAnalyzeRGBA(rgba,width,height,result);
    float errorA=a?board_anchor_error(&normal,height):1.0f;
    float errorB=b?board_anchor_error(&upsideDown,height):1.0f;
    // If both are plausible but indistinguishable, don't guess.
    if(a&&b&&fabsf(errorA-errorB)<0.018f)return 0;
    BLVisionResult *chosen=errorA<errorB?&normal:&upsideDown;
    // Refuse screens whose detected board is too far from the actual game's
    // portrait layout; a wrong orientation must never create a false highlight.
    if(board_anchor_error(chosen,height)>0.085f)return 0;
    *result=*chosen;
    return 1;
}
