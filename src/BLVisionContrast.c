#include "BLVisionContrast.h"
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
static const uint8_t *at(const uint8_t *rgba,int w,int h,int x,int y,int flip) {
    if(flip)y=h-1-y;
    return rgba+((size_t)y*w+x)*4;
}
static int d3(const uint8_t a[3],const uint8_t b[3]) {
    return abs((int)a[0]-b[0])+abs((int)a[1]-b[1])+abs((int)a[2]-b[2]);
}
static void sample(const uint8_t *im,int w,int h,int x,int y,int flip,
                   uint8_t rgb[3]) {
    int sums[3]={0};
    for(int dy=-2;dy<=2;dy+=2)for(int dx=-2;dx<=2;dx+=2){
        const uint8_t *p=at(im,w,h,x+dx,y+dy,flip);
        for(int k=0;k<3;k++)sums[k]+=p[k];
    }
    for(int k=0;k<3;k++)rgb[k]=(uint8_t)((sums[k]+4)/9);
}
int BLVisionContrastBoard(const uint8_t *im,int w,int h,
                          const BLVisionResult *geom,uint64_t *out,int *margin) {
    if(!im||!geom||!out||w<240||h<350||w>1600||h>3000)return 0;
    double stepX=(geom->lastX-geom->firstX)/7.0;
    double stepY=(geom->lastY-geom->firstY)/7.0;
    if(stepX<15||stepY<15||
       fabs(stepX-stepY)>fmax(stepX,stepY)*.14)return 0;
    const int flip=geom->flippedInput;
    double left=geom->firstX-stepX*.5,top=geom->firstY-stepY*.5;
    if(left<3||top<3||left+8*stepX+3>=w||
       top+8*stepY+3>=h)return 0;
    // Median of interior grid crossings provides color reference even
    // when most board cells are occupied by brightly beveled blocks.
    int hist[3][256]={0};
    for(int row=1;row<=7;row++)for(int col=1;col<=7;col++){
        uint8_t rgb[3];
        sample(im,w,h,(int)lround(left+col*stepX),
            (int)lround(top+row*stepY),flip,rgb);
        for(int k=0;k<3;k++)++hist[k][rgb[k]];
    }
    uint8_t background[3]={0};
    for(int k=0;k<3;k++){
        int sum=0;
        for(int v=0;v<256;v++){sum+=hist[k][v];if(sum>=25){
            background[k]=(uint8_t)v;break;
        }}
    }
    int distances[64],low=765,high=0;
    for(int row=0;row<8;row++)for(int col=0;col<8;col++){
        int x=(int)lround(geom->firstX+col*stepX);
        int y=(int)lround(geom->firstY+row*stepY);
        uint8_t rgb[3];sample(im,w,h,x,y,flip,rgb);
        int d=d3(rgb,background);
        distances[row*8+col]=d;
        if(d<low)low=d;
        if(d>high)high=d;
    }
    if(high-low<75)return 0; // uniform or ambiguous board
    // Multi-colored skins form MANY bright clusters. A 2-means classifier
    // may wrongly put dark-red occupied pieces in the empty class when
    // blue/white blocks are much brighter. Instead locate the FIRST
    // substantial contrast gap above the darkest board-center population.
    int sorted[64];
    memcpy(sorted,distances,sizeof(sorted));
    for(int i=1;i<64;i++){
        int v=sorted[i],j=i-1;
        while(j>=0&&sorted[j]>v){sorted[j+1]=sorted[j];--j;}
        sorted[j+1]=v;
    }
    int cut=-1;
    for(int i=3;i<64;i++){
        if(sorted[i]-sorted[i-1]>=45 &&
           sorted[i-1]-sorted[0]<=65) {
            cut=i;
            break;
        }
    }
    if(cut<0)return 0;
    double middle=(sorted[cut-1]+sorted[cut])*.5;
    uint64_t result=0;int near=0,far=0,uncertain=0;
    for(int i=0;i<64;i++){
        double d=distances[i];
        if(fabs(d-middle)<12.0)++uncertain;
        if(d>middle){result|=UINT64_C(1)<<i;++far;}
        else ++near;
    }
    if(uncertain>2 || near<3 || far<1)return 0;
    // A settled board cannot contain a row or column that has already
    // cleared; reject suspicious cluster splits before any drag.
    for(int r=0;r<8;r++)if(((result>>(r*8))&255U)==255U)return 0;
    for(int c=0;c<8;c++)if((result&(UINT64_C(0x0101010101010101)<<c)) ==
                                       (UINT64_C(0x0101010101010101)<<c))return 0;
    *out=result;
    if(margin)*margin=sorted[cut]-sorted[cut-1];
    return 1;
}


// Human-reviewed screenshots in the user's 41-capture set exposed a bug in
// seam-RGB distance: dark-green tiles and empty navy squares can have very
// similar distance from the seam even while their raw luminance is very
// different. This second analyzer uses 64 fixed tile-center RGB samples.
int BLVisionLumaFromCenters(const uint8_t centers[64][3],
                            BLVisionLumaEvidence *out) {
    if(!centers||!out)return 0;
    int sorted[64],brightness[64];
    for(int i=0;i<64;i++) {
        brightness[i]=(int)centers[i][0]+centers[i][1]+centers[i][2];
        sorted[i]=brightness[i];
    }
    for(int i=1;i<64;i++){
        int v=sorted[i],j=i-1;
        while(j>=0&&sorted[j]>v){sorted[j+1]=sorted[j];--j;}
        sorted[j+1]=v;
    }
    // Detect the *first* large gap above the coherent dark board
    // background, not the largest gap separating bright block colors.
    int cut=-1;
    for(int i=2;i<64;i++) {
        if(sorted[i]-sorted[i-1]>=50 &&
           sorted[i-1]-sorted[0]<=75){cut=i;break;}
    }
    if(cut<0)return 0;
    int gap=sorted[cut]-sorted[cut-1];
    uint64_t filled=0,empty=0;
    int sum[3]={0,0,0},emptyN=0;
    int threshold=(sorted[cut]+sorted[cut-1])/2;
    for(int i=0;i<64;i++){
        if(brightness[i]>threshold)filled|=UINT64_C(1)<<i;
        else {
            for(int k=0;k<3;k++)sum[k]+=centers[i][k];
            ++emptyN;
        }
    }
    if(emptyN<2||emptyN>62)return 0;
    int bg[3];
    for(int k=0;k<3;k++)bg[k]=(sum[k]+emptyN/2)/emptyN;
    for(int i=0;i<64;i++) {
        if((filled>>i)&1)continue;
        int colorDistance=0;
        for(int k=0;k<3;k++)
            colorDistance+=abs((int)centers[i][k]-bg[k]);
        // A cell whose color deviates from the measured empty cluster
        // is UNKNOWN, not certainly empty. False negatives are safer.
        if(abs(brightness[i]-(bg[0]+bg[1]+bg[2]))<=28 &&
           colorDistance<=42)
            empty|=UINT64_C(1)<<i;
    }
    if(!empty)return 0;
    for(int row=0;row<8;row++)
        if(((filled>>(row*8))&255U)==255U)return 0;
    for(int col=0;col<8;col++){
        uint64_t mask=UINT64_C(0x0101010101010101)<<col;
        if((filled&mask)==mask)return 0;
    }
    *out=(BLVisionLumaEvidence){filled,empty,gap,emptyN};
    return 1;
}

int BLVisionLumaBoard(const uint8_t *im,int w,int h,
                      const BLVisionResult *geom,
                      BLVisionLumaEvidence *out) {
    if(!im||!geom||!out||w<240||h<350||w>1600||h>3000)
        return 0;
    double dx=(geom->lastX-geom->firstX)/7.0;
    double dy=(geom->lastY-geom->firstY)/7.0;
    double left=geom->firstX-dx*.5,top=geom->firstY-dy*.5;
    if(dx<15||dy<15 || fabs(dx-dy)>fmax(dx,dy)*.14 ||
       left<3||top<3||left+dx*8+3>=w||top+dy*8+3>=h)
        return 0;
    uint8_t centers[64][3];
    for(int row=0;row<8;row++)for(int col=0;col<8;col++){
        int x=(int)lround(geom->firstX+col*dx);
        int y=(int)lround(geom->firstY+row*dy);
        sample(im,w,h,x,y,geom->flippedInput,
               centers[row*8+col]);
    }
    return BLVisionLumaFromCenters(centers,out);
}
