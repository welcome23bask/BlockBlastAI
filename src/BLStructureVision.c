#include "BLStructureVision.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef BL_STRUCTURE_TRACE
#include <stdio.h>
#define BL_TRACE(...) fprintf(stderr,__VA_ARGS__)
#else
#define BL_TRACE(...) ((void)0)
#endif

// Skin-neutral geometry + contrast vision. No RGB palette/skin identifiers.
// Conservative: when the grid, empty reference or piece silhouette is
// ambiguous, return 0 and let the overlay hide its recommendation.
typedef struct { int r,g,b; } BLRGB;
typedef struct {
    int valid,x,y,side,flip;
    BLRGB empty,scene;
} BLSceneReference;
static BLSceneReference reference;

void BLStructureReset(void) { memset(&reference,0,sizeof(reference)); }
static inline int absi(int n){return n<0?-n:n;}
static inline int maxi(int a,int b){return a>b?a:b;}
static inline int mini(int a,int b){return a<b?a:b;}
static inline int roundi(double x){return (int)lround(x);}
static inline BLRGB sample(const uint8_t *im,int w,int h,int x,int y,int flip) {
    if(x<0)x=0;if(x>=w)x=w-1;if(y<0)y=0;if(y>=h)y=h-1;
    const uint8_t *p=im+4*((size_t)(flip?h-1-y:y)*w+x);
    BLRGB v={(int)p[0],(int)p[1],(int)p[2]};return v;
}
static inline int diff(BLRGB a,BLRGB b) {
    return absi(a.r-b.r)+absi(a.g-b.g)+absi(a.b-b.b);
}
static BLRGB center_patch(const uint8_t *im,int w,int h,int x,int y,int flip,int rad) {
    BLRGB out={0,0,0};int n=0;
    for(int dy=-rad;dy<=rad;dy+=maxi(1,rad))
        for(int dx=-rad;dx<=rad;dx+=maxi(1,rad)){
            BLRGB p=sample(im,w,h,x+dx,y+dy,flip);
            out.r+=p.r;out.g+=p.g;out.b+=p.b;++n;
        }
    out.r/=n;out.g/=n;out.b/=n;return out;
}
static int median_hist(const int hist[256],int total) {
    int seen=0;for(int i=0;i<256;++i){
        seen+=hist[i];if(seen*2>=total)return i;
    }return 0;
}
static BLRGB region_median(const uint8_t *im,int w,int h,int x0,int y0,int x1,int y1,int flip) {
    int hist[3][256]={{0}},n=0;
    const int step=maxi(3,(x1-x0)/30);
    for(int y=y0+step/2;y<y1;y+=step)
        for(int x=x0+step/2;x<x1;x+=step){
            BLRGB p=sample(im,w,h,x,y,flip);
            hist[0][p.r]++;hist[1][p.g]++;hist[2][p.b]++;++n;
        }
    BLRGB out={0,0,0};
    if(n){out.r=median_hist(hist[0],n);out.g=median_hist(hist[1],n);
          out.b=median_hist(hist[2],n);}
    return out;
}
static int edge_score(const uint8_t *im,int w,int h,int x,int y,int size,int flip) {
    const int d=maxi(3,w/175);
    if(x<2*d||y<2*d||x+size+2*d>=w||y+size+2*d>=h)return 0;
    int sums[4]={0},seams=0;
    for(int j=0;j<9;++j){
        int px=x+roundi((j+.5)*size/9.0);
        int py=y+roundi((j+.5)*size/9.0);
        sums[0]+=diff(sample(im,w,h,x+d,py,flip),sample(im,w,h,x-d,py,flip));
        sums[1]+=diff(sample(im,w,h,x+size-d,py,flip),
                      sample(im,w,h,x+size+d,py,flip));
        sums[2]+=diff(sample(im,w,h,px,y+d,flip),sample(im,w,h,px,y-d,flip));
        sums[3]+=diff(sample(im,w,h,px,y+size-d,flip),
                      sample(im,w,h,px,y+size+d,flip));
    }
    for(int k=0;k<4;++k)if(sums[k]<9*32)return 0;
    const double pitch=(double)size/8.0;
    for(int row=0;row<8;row+=2)
        for(int col=1;col<8;++col){
            int yy=y+roundi((row+.5)*pitch);
            int xx=x+roundi(col*pitch);
            seams+=diff(sample(im,w,h,xx,yy,flip),
                        sample(im,w,h,xx+roundi(pitch*.28),yy,flip));
        }
    if(seams<7*4*4)return 0;
    // All four contours must be supported. A lone UI line is not a board.
    int minSide=sums[0];
    for(int k=1;k<4;++k)if(sums[k]<minSide)minSide=sums[k];
    int edgeTotal=sums[0]+sums[1]+sums[2]+sums[3];
    return edgeTotal+minSide+mini(seams,1200)/3;
}
static int board_geometry_rank(int edge,int y,int side,int height) {
    // The real 8x8 board occupies a stable part of the portrait layout,
    // independent of which seasonal skin is being used. Geometry is primary:
    // this is a tie-breaker against an equally square score animation/header.
    double center=((double)y+side*.5)/height;
    int penalty=roundi(12000.0*fabs(center-.44));
    return edge-penalty;
}
static int locate(const uint8_t *im,int w,int h,int flip,int *x0,int *y0,int *size) {
    int best=0,bx=0,by=0,bs=0;
    int lowS=roundi(w*.79),highS=roundi(w*.955);
    int lowY=roundi(h*.17),highY=roundi(h*.34);
    for(int s=lowS;s<=highS;s+=4) {
        int mid=(w-s)/2;
        int range=maxi(14,roundi(w*.09));
        for(int y=lowY;y<=highY;y+=4)for(int x=mid-range;x<=mid+range;x+=4){
            int score=board_geometry_rank(edge_score(im,w,h,x,y,s,flip),y,s,h);
            if(score>best){best=score;bx=x;by=y;bs=s;}
        }
    }
    if(best==0)return 0;
    // Refine the contour at pixel precision.
    for(int s=maxi(12,bs-5);s<=bs+5;++s)
        for(int y=by-5;y<=by+5;++y)
            for(int x=bx-5;x<=bx+5;++x){
                int score=board_geometry_rank(edge_score(im,w,h,x,y,s,flip),y,s,h);
                if(score>best){best=score;bx=x;by=y;bs=s;}
            }
    if(bs<8*20)return 0;
    *x0=bx;*y0=by;*size=bs;return 1;
}
static int count_cluster(const BLRGB centers[64],int index,int radius) {
    int n=0;
    for(int i=0;i<64;++i)if(diff(centers[i],centers[index])<=radius)++n;
    return n;
}
static int board_mask(const uint8_t *im,int w,int h,int x,int y,int size,int flip,
                      BLRGB scene,uint64_t *board,BLRGB *emptyOut) {
    BLRGB colors[64],baseline={0,0,0};
    const double pitch=(double)size/8.0;
    const int r=maxi(2,mini(5,roundi(pitch*.09)));
    for(int row=0;row<8;++row)for(int col=0;col<8;++col){
        int cx=x+roundi((col+.5)*pitch);
        int cy=y+roundi((row+.5)*pitch);
        colors[row*8+col]=center_patch(im,w,h,cx,cy,flip,r);
    }
    int best=0,bestN=0;
    for(int i=0;i<64;++i){
        int n=count_cluster(colors,i,34);
        if(n>bestN){bestN=n;best=i;}
    }
    if(bestN<7)return 0; // no defensible empty-cell reference
    int validRef=reference.valid && reference.flip==flip &&
                 absi(reference.x-x)<size*.07 &&
                 absi(reference.y-y)<size*.07 &&
                 absi(reference.side-size)<size*.07 &&
                 diff(reference.scene,scene)<90;
    if(validRef)baseline=reference.empty;
    else{
        for(int i=0,n=0;i<64;++i)
            if(diff(colors[i],colors[best])<=34){
                baseline.r+=colors[i].r;baseline.g+=colors[i].g;
                baseline.b+=colors[i].b;++n;
            }
        baseline.r/=bestN;baseline.g/=bestN;baseline.b/=bestN;
    }
    int close=0,disp=0;
    for(int i=0;i<64;++i){
        int d=diff(colors[i],baseline);
        if(d<=42){close++;disp+=d;}
    }
    // A board with few matching empty cells is dangerous unless it has a
    // confirmed baseline from this same stable scene.
    if(close<4 && !validRef)return 0;
    int threshold=maxi(46,(close?disp/close:0)*3+20);
    threshold=mini(threshold,105);
    uint64_t bits=0;
    for(int i=0;i<64;++i)
        if(diff(colors[i],baseline)>threshold)
            bits |=UINT64_C(1)<<i;
    // Uncalibrated, extremely full frames have an ambiguous baseline.
    if(!validRef && __builtin_popcountll(bits)>52)return 0;
    *board=bits;*emptyOut=baseline;return 1;
}
static int shape_connected(const BLPiece *piece) {
    if(!piece||piece->count<=0||piece->count>25)return 0;
    int seen[5][5]={{0}},on[5][5]={{0}},minx=5,miny=5;
    for(int k=0;k<piece->count;++k){
        int x=piece->x[k],y=piece->y[k];
        if(x<0||x>=5||y<0||y>=5||on[y][x])return 0;
        on[y][x]=1;minx=mini(minx,x);miny=mini(miny,y);
    }
    if(minx!=0||miny!=0)return 0;
    int q[25],head=0,tail=0,hit=0;
    q[tail++]=piece->y[0]*5+piece->x[0];
    seen[piece->y[0]][piece->x[0]]=1;
    while(head<tail){
        int v=q[head++],x=v%5,y=v/5;++hit;
        const int dx[4]={1,-1,0,0},dy[4]={0,0,1,-1};
        for(int i=0;i<4;++i){
            int xx=x+dx[i],yy=y+dy[i];
            if(xx<0||yy<0||xx>=5||yy>=5||!on[yy][xx]||seen[yy][xx])continue;
            seen[yy][xx]=1;q[tail++]=yy*5+xx;
        }
    }
    return hit==piece->count;
}
static int tray_slot(const uint8_t *im,int w,int h,int flip,
                     int slot,int trayTop,int trayBottom,double small,BLPiece *out) {
    const int x0=slot*w/3,x1=(slot+1)*w/3;
    BLRGB bg=region_median(im,w,h,x0,trayTop,x1,trayBottom,flip);
    // Wide tray tiles can dominate a slot's sampled colors. Cross-check
    // against two outside scene edges (without memorizing any tile hue).
    BLRGB edge=region_median(im,w,h,maxi(2,w/60),trayTop,
                            maxi(15,w/25),trayBottom,flip);
    if(diff(bg,edge)>64) {
        BLRGB far=region_median(im,w,h,w-maxi(15,w/25),trayTop,
                               w-maxi(2,w/60),trayBottom,flip);
        if(diff(edge,far)<46)bg=edge;
    }
    const int step=maxi(3,w/310),cols=(x1-x0)/step,rows=(trayBottom-trayTop)/step;
    if(cols<5||rows<5)return 0;
    int hist[766]={0},samples=0;
    for(int y=trayTop+step/2;y<trayBottom;y+=step*2)
        for(int x=x0+step/2;x<x1;x+=step*2){
            int d=diff(sample(im,w,h,x,y,flip),bg);
            hist[mini(765,d)]++;++samples;
        }
    int seen=0,q70=0;
    for(int d=0;d<766;++d){
        seen+=hist[d];if(seen*10>=samples*7){q70=d;break;}
    }
    // This is a contrast threshold, never a specific block color.
    const int contrast=mini(115,maxi(39,q70*2+19));
    const int n=rows*cols;
    uint8_t *fg=(uint8_t *)calloc(n,1),*visited=(uint8_t *)calloc(n,1);
    int *queue=(int *)malloc(n*sizeof(int));
    if(!fg||!visited||!queue){free(fg);free(visited);free(queue);return 0;}
    for(int row=0;row<rows;++row)for(int col=0;col<cols;++col){
        int xx=x0+col*step+step/2,yy=trayTop+row*step+step/2;
        fg[row*cols+col]=(uint8_t)(diff(sample(im,w,h,xx,yy,flip),bg)>=contrast);
    }
    int bestArea=0,bestX0=0,bestX1=0,bestY0=0,bestY1=0;
    for(int start=0;start<n;++start) {
        if(!fg[start]||visited[start])continue;
        int head=0,tail=0,firstx=cols,firsty=rows,lastx=0,lasty=0;
        queue[tail++]=start;visited[start]=1;
        while(head<tail){
            int v=queue[head++],x=v%cols,y=v/cols;
            firstx=mini(firstx,x);lastx=maxi(lastx,x);
            firsty=mini(firsty,y);lasty=maxi(lasty,y);
            // A two-grid-point bridge closes the tiny gaps between tiles,
            // while not merging separate distant background decorations.
            for(int dy=-2;dy<=2;++dy)for(int dx=-2;dx<=2;++dx){
                int xx=x+dx,yy=y+dy;
                if(xx<0||yy<0||xx>=cols||yy>=rows)continue;
                int i=yy*cols+xx;
                if(!fg[i]||visited[i])continue;
                visited[i]=1;queue[tail++]=i;
            }
        }
        int minArea=maxi(8,roundi(small*small/(step*step)*.19));
        if(tail<minArea||tail<=bestArea)continue;
        // The component's extents must plausibly be a <=5x5 piece.
        if((lastx-firstx+1)*step>small*5.5 ||
           (lasty-firsty+1)*step>small*5.5)continue;
        bestArea=tail;
        bestX0=x0+firstx*step;bestX1=x0+(lastx+1)*step;
        bestY0=trayTop+firsty*step;bestY1=trayTop+(lasty+1)*step;
    }
    free(fg);free(visited);free(queue);
    memset(out,0,sizeof(*out));
    if(!bestArea)return 1; // this tray slot may already have been used
    int cw=roundi((bestX1-bestX0)/small),ch=roundi((bestY1-bestY0)/small);
    if(cw<1||ch<1||cw>5||ch>5)return 0;
    BLPiece shape={0};
    for(int row=0;row<ch;++row)for(int col=0;col<cw;++col){
        int xx=bestX0+roundi((col+.5)*small);
        int yy=bestY0+roundi((row+.5)*small);
        int nfg=0,ntotal=0;
        int rad=maxi(2,roundi(small*.14));
        for(int dy=-rad;dy<=rad;dy+=maxi(1,rad/2))
            for(int dx=-rad;dx<=rad;dx+=maxi(1,rad/2)){
                ++ntotal;
                BLRGB p=sample(im,w,h,xx+dx,yy+dy,flip);
                if(diff(p,bg)>=contrast)nfg++;
            }
        if(nfg*5<ntotal*3)continue;
        shape.x[shape.count]=col;shape.y[shape.count]=row;
        if(++shape.count>25)return 0;
    }
    if(!shape.count)return 0;
    if(!shape_connected(&shape))return 0;
    *out=shape;return 1;
}
static int scan(const uint8_t *rgba,int w,int h,int flip,BLVisionResult *out,
                BLRGB *refEmpty,BLRGB *refScene,int *refx,int *refy,int *refs) {
    int x=0,y=0,s=0;
    // Reuse the board location while its contour is still visible.
    if(reference.valid && reference.flip==flip &&
       edge_score(rgba,w,h,reference.x,reference.y,reference.side,flip)>0) {
        x=reference.x;y=reference.y;s=reference.side;
    } else if(!locate(rgba,w,h,flip,&x,&y,&s)){BL_TRACE("locate fail flip=%d\\n",flip);return 0;}
    BLRGB scene=region_median(rgba,w,h,mini(w-8,x+5),mini(h-8,y+s+roundi(s*.11)),
                     mini(w,x+roundi(s*.18)),mini(h,y+s+roundi(s*.40)),flip);
    uint64_t board=0;BLRGB empty={0,0,0};
    if(!board_mask(rgba,w,h,x,y,s,flip,scene,&board,&empty)){BL_TRACE("board baseline fail flip=%d geom=%d,%d,%d\\n",flip,x,y,s);return 0;}
    double pitch=(double)s/8.0,small=pitch*.44;
    int trayTop=y+s+roundi(s*.07);
    int trayBottom=mini(roundi(h*.95),y+s+roundi(s*.70));
    if(trayBottom-trayTop<roundi(s*.14)||small<7||small>80){BL_TRACE("tray geometry fail flip=%d\\n",flip);return 0;}
    BLVisionResult found={0};found.board=board;found.flippedInput=flip;
    found.firstX=(float)(x+pitch*.5);found.firstY=(float)(y+pitch*.5);
    found.lastX=(float)(x+pitch*7.5);found.lastY=(float)(y+pitch*7.5);
    for(int slot=0;slot<3;++slot){
        if(!tray_slot(rgba,w,h,flip,slot,trayTop,trayBottom,small,&found.pieces[slot])){
            BL_TRACE("tray slot fail flip=%d slot=%d geom=%d,%d,%d board=%llx\\n",flip,slot,x,y,s,(unsigned long long)board);
            return 0;
        }
        found.detectedPieces+=found.pieces[slot].count>0;
    }
    if(found.detectedPieces==0){BL_TRACE("no pieces flip=%d board=%llx\\n",flip,(unsigned long long)board);return 0;}
    *out=found;*refEmpty=empty;*refScene=scene;
    *refx=x;*refy=y;*refs=s;return 1;
}
int BLStructureAnalyzeRGBA(const uint8_t *rgba,int width,int height,BLVisionResult *out) {
    if(!rgba||!out||width<240||height<350||width>1600||height>3000)return 0;
    BLVisionResult a={0},b={0};BLRGB ae={0,0,0},be={0,0,0},as={0,0,0},bs={0,0,0};
    int ax=0,ay=0,az=0,bx=0,by=0,bz=0;
    int oka=scan(rgba,width,height,0,&a,&ae,&as,&ax,&ay,&az);
    int okb=scan(rgba,width,height,1,&b,&be,&bs,&bx,&by,&bz);
    double ea=oka?fabs(((a.firstY+a.lastY)*.5)/height-.44):100.0;
    double eb=okb?fabs(((b.firstY+b.lastY)*.5)/height-.44):100.0;
    // The playfield is above the tray in portrait. Reject mirrored ambiguity.
    if((!oka&&!okb)||(oka&&okb&&fabs(ea-eb)<.008)){BL_TRACE("orientation ambiguity oka=%d okb=%d ea=%f eb=%f\\n",oka,okb,ea,eb);return 0;}
    int flipped=(eb<ea);
    double error=flipped?eb:ea;
    if(error>.10){BL_TRACE("orientation error=%f\\n",error);return 0;}
    BLVisionResult picked=flipped?b:a;
    reference.valid=1;reference.x=flipped?bx:ax;reference.y=flipped?by:ay;
    reference.side=flipped?bz:az;reference.flip=flipped;
    reference.empty=flipped?be:ae;reference.scene=flipped?bs:as;
    BL_TRACE("read success flip=%d board=%llx counts=%d,%d,%d geom=%d,%d,%d\\n",flipped,(unsigned long long)picked.board,picked.pieces[0].count,picked.pieces[1].count,picked.pieces[2].count,reference.x,reference.y,reference.side);
    *out=picked;return 1;
}
