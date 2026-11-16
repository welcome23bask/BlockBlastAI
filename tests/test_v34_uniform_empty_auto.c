// v3.4: An all-empty 8x8 grid has no occupied-vs-empty luminance gap.
// 128 synthetic palettes; abort if a single occupied block is called empty.
// These are deterministic tests, not real gameplay training screenshots.
#include "../src/BLVisionContrast.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define W 430
#define H 932
static unsigned char *pixels;
static int clamp(int n){return n<1?1:n>254?254:n;}
static void rect(int x0,int y0,int x1,int y1,int r,int g,int b){
    for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++){
        if(x<0||y<0||x>=W||y>=H)continue;
        unsigned char *p=pixels+4*((size_t)y*W+x);
        p[0]=(unsigned char)clamp(r);p[1]=(unsigned char)clamp(g);
        p[2]=(unsigned char)clamp(b);p[3]=255;
    }
}
static BLVisionResult geometry(void){
    BLVisionResult g={0};
    g.firstX=46;g.firstY=252;g.lastX=382;g.lastY=588;
    return g;
}
static void draw(int variant,int occupied,int seamless){
    int c[3]={40+(variant*17)%130,43+(variant*37)%125,
              44+(variant*67)%130};
    int edge[3];
    for(int k=0;k<3;k++)edge[k]=clamp(c[k]+(c[k]>130?-31:31));
    rect(0,0,W,H,edge[0],edge[1],edge[2]);
    // Board bounds x22..406, y228..612, step 48; cell centers are
    // x46 + 48*c, y252 + 48*r. Cell crossing points remain seam-colored.
    for(int row=0;row<8;row++)for(int col=0;col<8;col++){
        int n=(row*8+col)%5-2;
        int x=22+col*48,y=228+row*48;
        rect(x+4,y+4,x+44,y+44,c[0]+n,c[1]-n,c[2]+n);
    }
    if(occupied) {
        // Test both saturated bright and very dark tiles over changing skins.
        int r=3,col=5,x=22+col*48,y=228+r*48;
        int sign=(variant%2)?1:-1;
        rect(x+4,y+4,x+44,y+44,
             clamp(c[0]+sign*100),clamp(c[1]+sign*100),
             clamp(c[2]+sign*100));
    }
    if(seamless)rect(22,228,406,612,c[0],c[1],c[2]);
}
int main(void){
    pixels=(unsigned char *)calloc((size_t)W*H,4);assert(pixels);
    BLVisionResult g=geometry();
    for(int i=0;i<128;i++){
        uint64_t empty=0;
        draw(i,0,0);
        assert(BLVisionUniformEmptyBoard(pixels,W,H,&g,&empty));
        assert(empty==UINT64_MAX);
        draw(i,1,0);
        empty=0;
        assert(!BLVisionUniformEmptyBoard(pixels,W,H,&g,&empty));
        assert(empty==0);
        draw(i,0,1);
        empty=0;
        assert(!BLVisionUniformEmptyBoard(pixels,W,H,&g,&empty));
        assert(empty==0);
    }
    free(pixels);
    puts("PASS 128 empty color themes, 128 occupied-block vetoes, and 128 seam-free false-positive vetoes");
    return 0;
}
