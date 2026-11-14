// Adaptive vision fallback: geometry and per-frame colors rather than dark-blue palette.
#include "BLAdaptiveTheme.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static inline const uint8_t *P(const uint8_t *im,int w,int h,int x,int y,int flip) {
    return im + ((size_t)(flip ? h-1-y : y)*w + x)*4;
}
static inline int delta(const uint8_t *a,const uint8_t *b) {
    return abs((int)a[0]-(int)b[0])+abs((int)a[1]-(int)b[1])+abs((int)a[2]-(int)b[2]);
}
// Compare RGB color to the actual captured game's background. Hard-coded
// red/blue/green palettes excluded light-pink 3x3 pieces on skin variants.
static int is_tile(const uint8_t *p,const uint8_t bg[3],int threshold) {
    int dr=(int)p[0]-bg[0],dg=(int)p[1]-bg[1],db=(int)p[2]-bg[2];
    int sum=abs(dr)+abs(dg)+abs(db);
    int maxd=abs(dr);
    if(abs(dg)>maxd)maxd=abs(dg);
    if(abs(db)>maxd)maxd=abs(db);
    return sum>=threshold && maxd>=18;
}
static int tile_center(const uint8_t *im,int w,int h,int x,int y,int flip,
                       const uint8_t bg[3],int threshold) {
    if(x<3 || y<3 || x+3>=w || y+3>=h)return 0;
    int hits=0;
    for(int dy=-2;dy<=2;dy+=2)for(int dx=-2;dx<=2;dx+=2)
        hits+=is_tile(P(im,w,h,x+dx,y+dy,flip),bg,threshold);
    return hits>=5;
}
static int loc_score(const uint8_t *im,int w,int h,int flip,int x,int y,int s) {
    const double f[]={.10,.22,.34,.46,.58,.70,.82,.92};
    int d=(int)(w*.008); if(d<3)d=3;
    if(x-d<0 || x+s+d>=w || y-d<0 || y+s+d>=h) return 0;
    int sums[4]={0};
    for(int j=0;j<8;++j) {
        int xx=x+(int)(s*f[j]), yy=y+(int)(s*f[j]);
        sums[0]+=delta(P(im,w,h,x+d,yy,flip),P(im,w,h,x-d,yy,flip));
        sums[1]+=delta(P(im,w,h,x+s-d,yy,flip),P(im,w,h,x+s+d,yy,flip));
        sums[2]+=delta(P(im,w,h,xx,y+d,flip),P(im,w,h,xx,y-d,flip));
        sums[3]+=delta(P(im,w,h,xx,y+s-d,flip),P(im,w,h,xx,y+s+d,flip));
    }
    // All FOUR boundaries of a real, centered, square board must be visible.
    for(int j=0;j<4;++j) if(sums[j]<8*45)return 0;
    return sums[0]+sums[1]+sums[2]+sums[3];
}
static int adaptive_locate(const uint8_t *im,int w,int h,int flip,int *x0,int *y0,int *sz) {
    int best=0,xBest=0,yBest=0,sBest=0;
    int lowS=(int)(w*.83),highS=(int)(w*.95);
    int lowY=(int)(h*.20),highY=(int)(h*.29);
    for(int s=lowS;s<=highS;s+=4) {
        int center=(w-s)/2;
        for(int y=lowY;y<=highY;y+=3) {
            for(int x=center-14;x<=center+14;x+=3) {
                int score=loc_score(im,w,h,flip,x,y,s);
                if(score>best) {best=score;xBest=x;yBest=y;sBest=s;}
            }
        }
    }
    if(best<8*4*60)return 0;
    // A flat UI card or advertisement cannot substitute for an 8x8 board.
    // Repeated cell gutters are structural evidence independent of theme.
    int grid=0,samples=0;
    for(int row=0;row<8;++row)for(int col=1;col<8;++col) {
        int sy=yBest+(int)((row+.5)*sBest/8.0);
        int gx=xBest+(int)(col*sBest/8.0);
        int cx=xBest+(int)((col-.5)*sBest/8.0);
        if(gx>=w || cx>=w || sy>=h)continue;
        const uint8_t *gap=P(im,w,h,gx,sy,flip),*cell=P(im,w,h,cx,sy,flip);
        grid+=delta(gap,cell); ++samples;
    }
    if(!samples || grid/samples<3)return 0;
    *x0=xBest;*y0=yBest;*sz=sBest;return 1;
}
static uint8_t median_channel(const int count[256],int total) {
    int acc=0;for(int i=0;i<256;++i) {acc+=count[i];if(acc*2>=total)return (uint8_t)i;}return 0;
}
static void sample_background(const uint8_t *im,int w,int h,int flip,
                              int x0,int y0,int x1,int y1,uint8_t bg[3]) {
    int hist[3][256]={0},total=0;
    int sx=(x1-x0)/25,sy=(y1-y0)/20;
    if(sx<5)sx=5;if(sy<5)sy=5;
    for(int y=y0+sy/2;y<y1;y+=sy)for(int x=x0+sx/2;x<x1;x+=sx) {
        if(x<0||x>=w||y<0||y>=h)continue;
        const uint8_t *p=P(im,w,h,x,y,flip);
        for(int k=0;k<3;++k)++hist[k][p[k]];
        ++total;
    }
    if(!total) {memset(bg,0,3);return;}
    for(int k=0;k<3;++k)bg[k]=median_channel(hist[k],total);
}
// The 60th-percentile local color difference estimates tray gradients without
// learning colored game pieces as the background. Pixels are sampled sparsely.
static int tray_noise(const uint8_t *im,int w,int h,int flip,
                      int x0,int y0,int x1,int y1,const uint8_t bg[3]) {
    int hist[766]={0},n=0;
    int sx=(x1-x0)/24,sy=(y1-y0)/20;
    if(sx<4)sx=4;if(sy<4)sy=4;
    for(int y=y0+sy/2;y<y1;y+=sy)for(int x=x0+sx/2;x<x1;x+=sx) {
        if(x<0||y<0||x>=w||y>=h)continue;
        int d=delta(P(im,w,h,x,y,flip),bg);
        if(d>765)d=765;
        ++hist[d];++n;
    }
    int seen=0;
    for(int d=0;d<766&&n;++d) {
        seen+=hist[d];
        if(seen*10>=n*6)return d;
    }
    return 0;
}
static int adaptive_scan(const uint8_t *im,int w,int h,int flip,BLVisionResult *out) {
    int left,top,side;
    if(!adaptive_locate(im,w,h,flip,&left,&top,&side))return 0;
    double cell=(double)side/8.0,small=cell*.44;
    if(small<8||small>80)return 0;
    int trayTop=(int)(top+side+side*.07);
    int trayBottom=(int)fmin(h*.95,top+side+side*.70);
    if(trayTop<0||trayBottom<=trayTop+20||trayBottom>=h)return 0;
    BLVisionResult found={0};found.flippedInput=flip;
    found.firstX=(float)(left+cell*.5);found.firstY=(float)(top+cell*.5);
    found.lastX=(float)(left+cell*7.5);found.lastY=(float)(top+cell*7.5);
    // Seams stay background-colored even when many tiles are filled.
    int rh[3][256]={0},n=0;
    for(int row=0;row<8;++row)for(int col=1;col<8;++col) {
        int x=left+(int)lround(col*cell),y=top+(int)lround((row+.5)*cell);
        const uint8_t *p=P(im,w,h,x,y,flip);
        for(int k=0;k<3;++k)++rh[k][p[k]];
        ++n;
    }
    uint8_t boardBg[3];for(int k=0;k<3;++k)boardBg[k]=median_channel(rh[k],n);
    for(int row=0;row<8;++row)for(int col=0;col<8;++col) {
        int x=left+(int)lround((col+.5)*cell);
        int y=top+(int)lround((row+.5)*cell);
        if(tile_center(im,w,h,x,y,flip,boardBg,68))
            found.board|=UINT64_C(1)<<(row*8+col);
    }
    for(int slot=0;slot<3;++slot) {
        int xmin=w,xmax=-1,ymin=h,ymax=-1,hits=0;
        int xfrom=slot*w/3,xto=(slot+1)*w/3;
        uint8_t trayBg[3];
        sample_background(im,w,h,flip,xfrom,trayTop,xto,trayBottom,trayBg);
        int noise=tray_noise(im,w,h,flip,xfrom,trayTop,xto,trayBottom,trayBg);
        int threshold=29+2*noise;
        if(threshold<31)threshold=31;
        if(threshold>88)threshold=88;
        for(int y=trayTop;y<trayBottom;++y)for(int x=xfrom;x<xto;++x) {
            if(!is_tile(P(im,w,h,x,y,flip),trayBg,threshold))continue;
            if(x<xmin)xmin=x;if(x>xmax)xmax=x;
            if(y<ymin)ymin=y;if(y>ymax)ymax=y;
            ++hits;
        }
        if(hits<35)continue;
        int cols=(int)lround((xmax-xmin+1)/small);
        int rows=(int)lround((ymax-ymin+1)/small);
        if(cols<1||cols>5||rows<1||rows>5)return 0;
        BLPiece shape={0};
        for(int row=0;row<rows;++row)for(int col=0;col<cols;++col) {
            int x=(int)lround(xmin+(col+.5)*small);
            int y=(int)lround(ymin+(row+.5)*small);
            if(!tile_center(im,w,h,x,y,flip,trayBg,threshold))continue;
            shape.x[shape.count]=col;shape.y[shape.count]=row;++shape.count;
        }
        if(!shape.count)return 0;
        found.pieces[slot]=shape;++found.detectedPieces;
    }
    if(found.detectedPieces<1)return 0;
    float anchor=(found.firstY+found.lastY)/(2.0f*h);
    if(fabsf(anchor-.44f)>.085f)return 0;
    *out=found;return 1;
}
int BLAdaptiveAnalyzeRGBA(const uint8_t *rgba,int width,int height,BLVisionResult *out) {
    if(!rgba||!out||width<240||height<350||width>1600||height>3000)return 0;
    BLVisionResult straight={0},flipped={0};
    int a=adaptive_scan(rgba,width,height,0,&straight);
    int b=adaptive_scan(rgba,width,height,1,&flipped);
    if(a&&b) {
        float e0=fabsf((straight.firstY+straight.lastY)/(2.0f*height)-.44f);
        float e1=fabsf((flipped.firstY+flipped.lastY)/(2.0f*height)-.44f);
        if(fabsf(e0-e1)<.018f)return 0;
        *out=e0<e1?straight:flipped;
    } else if(a)*out=straight;
    else if(b)*out=flipped;
    else return 0;
    return 1;
}
