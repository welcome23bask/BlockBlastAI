// Regression for the user's black-block navy-board screenshot (709x1536):
// seven black board squares and orange brick tray pieces of lengths 5 / 4 / 5.
// Fixture uses synthetic rectangles; actual user's photo is never committed.
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/BLVisionCore.h"
#define W 709
#define H 1536
static uint8_t *frame;
static void box(int x0,int y0,int x1,int y1,int r,int g,int b) {
    for(int y=y0;y<y1;++y)for(int x=x0;x<x1;++x) {
        if(x<0||y<0||x>=W||y>=H)continue;
        uint8_t *p=frame+4*((size_t)y*W+x);
        p[0]=r;p[1]=g;p[2]=b;p[3]=255;
    }
}
static uint64_t expected(void) {return UINT64_C(0x808000000001e100);}
static void draw(int hasBlack) {
    box(0,0,W,H,131,182,222);
    box(38,356,674,995,32,51,72);
    box(39,358,670,989,38,56,82);
    for(int r=0;r<8;++r)for(int c=0;c<8;++c) {
        int x0=39+(int)lround(c*631./8)+2;
        int x1=39+(int)lround((c+1)*631./8)-2;
        int y0=358+(int)lround(r*631./8)+2;
        int y1=358+(int)lround((r+1)*631./8)-2;
        box(x0,y0,x1,y1,42,69,100);
        if(hasBlack && (expected()&(UINT64_C(1)<<(8*r+c))))
            box(x0,y0,x1,y1,0,0,0);
    }
    const int bases[3]={60,284,473},sizes[3]={5,4,5};
    for(int p=0;p<3;++p)for(int c=0;c<sizes[p];++c) {
        int x=bases[p]+c*35;
        box(x,1157,x+33,1192,226,107,67);
        box(x+2,1159,x+31,1174,238,126,82);
    }
}
static void verify(BLVisionResult *v,int flipped) {
    assert(v->board==expected());
    assert(v->detectedPieces==3);
    assert(v->flippedInput==flipped);
    const int n[3]={5,4,5};
    for(int p=0;p<3;++p) {
        assert(v->pieces[p].count==n[p]);
        for(int i=0;i<n[p];++i) {
            assert(v->pieces[p].x[i]==i);
            assert(v->pieces[p].y[i]==0);
        }
    }
    assert(fabs(v->firstX-78.5)<7);
    assert(fabs(v->firstY-397.5)<7);
}
int main(void) {
    frame=malloc((size_t)W*H*4);assert(frame);
    draw(1);
    BLVisionResult original={0},reversed={0};
    assert(BLVisionAnalyzeRGBA(frame,W,H,&original));
    verify(&original,0);
    uint8_t *reverse=malloc((size_t)W*H*4);assert(reverse);
    for(int y=0;y<H;++y)memcpy(reverse+4*(size_t)y*W,
        frame+4*(size_t)(H-1-y)*W,(size_t)W*4);
    assert(BLVisionAnalyzeRGBA(reverse,W,H,&reversed));
    verify(&reversed,1);
    draw(0);
    BLVisionResult empty={0};
    if(BLVisionAnalyzeRGBA(frame,W,H,&empty))assert(empty.board==0);
    free(reverse);free(frame);
    puts("black theme: 7 black board cells, 5/4/5 orange bars, reverse, empty passed");
    return 0;
}
