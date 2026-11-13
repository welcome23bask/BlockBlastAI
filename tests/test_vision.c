#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "../src/BLVisionCore.h"

#define W 430
#define H 932
static uint8_t *img;
static void rect(int x0,int y0,int x1,int y1,int r,int g,int b) {
    for(int y=y0;y<y1;++y)for(int x=x0;x<x1;++x) {
        if(x<0||x>=W||y<0||y>=H)continue;
        uint8_t *p=img+4*((size_t)y*W+x);
        p[0]=r;p[1]=g;p[2]=b;p[3]=255;
    }
}
static void tile(int row,int col,int r,int g,int b) {
    int x=23+col*385/8+4,y=216+row*385/8+4;
    rect(x,y,x+39,y+39,r,g,b);
}
static void tray(int slot,int rows,int cols,int r,int g,int b) {
    int x=slot==0?58:(slot==1?208:338);
    int y=665;
    for(int yy=0;yy<rows;++yy)for(int xx=0;xx<cols;++xx)
        rect(x+xx*21,y+yy*21,x+xx*21+19,y+yy*21+19,r,g,b);
}
static void init_fixture(void) {
    img=malloc((size_t)W*H*4); assert(img);
    rect(0,0,W,H,57,85,148); // light blue sky, not a filled block
    rect(23,216,408,601,33,44,90); // true 8×8 board
    tile(1,3,49,100,222);
    tile(2,1,237,118,39);tile(2,2,237,118,39);tile(2,3,49,100,222);tile(2,6,28,178,223);
    tile(3,1,237,118,39);tile(3,2,237,118,39);tile(3,3,49,100,222);tile(3,6,28,178,223);
    tile(4,3,49,100,222);tile(4,6,28,178,223);
    tile(5,3,49,100,222);
    // 3x2, 1x3 and 2x3 tray shapes from an actual Block Blast layout.
    tray(0,2,3,208,45,53);tray(1,3,1,244,126,38);tray(2,3,2,35,178,41);
}
static int near(float a,float b){return fabsf(a-b)<6.5f;}
int main(void){
    init_fixture();
    uint64_t expect=0;
    int occupied[][2]={{1,3},{2,1},{2,2},{2,3},{2,6},
                       {3,1},{3,2},{3,3},{3,6},{4,3},{4,6},{5,3}};
    for(int i=0;i<12;++i)expect|=UINT64_C(1)<<(occupied[i][0]*8+occupied[i][1]);
    BLVisionResult a;
    assert(BLVisionAnalyzeRGBA(img,W,H,&a));
    assert(a.board==expect);
    assert(a.detectedPieces==3);
    assert(a.pieces[0].count==6&&a.pieces[1].count==3&&a.pieces[2].count==6);
    assert(near(a.firstX,47)&&near(a.firstY,240));
    assert(near(a.lastX,383)&&near(a.lastY,576));
    assert(a.flippedInput==0);
    // CoreGraphics can supply vertically mirrored pixel rows.
    // Auto-orientation must produce IDENTICAL top-to-bottom board and tray.
    uint8_t *flip=malloc((size_t)W*H*4);assert(flip);
    for(int y=0;y<H;++y)memcpy(flip+4*(size_t)y*W,img+4*(size_t)(H-1-y)*W,(size_t)W*4);
    BLVisionResult b;
    assert(BLVisionAnalyzeRGBA(flip,W,H,&b));
    assert(b.board==expect&&b.flippedInput==1);
    assert(b.detectedPieces==3);
    assert(near(b.firstY,a.firstY) && near(b.lastY,a.lastY));
    free(flip);
    // A loading screen must NOT produce fake move instructions.
    rect(0,0,W,H,57,85,148);
    BLVisionResult none;
    assert(!BLVisionAnalyzeRGBA(img,W,H,&none));
    free(img);
    puts("vision tests passed: board orientation, colors, 3 tray pieces, geometry, no false scan");
    return 0;
}
