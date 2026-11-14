#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/BLVisionCore.h"

// Representative wood-theme colors, board geometry and three pieces from
// the supplied 709x1536 iPhone screenshot (synthetic fixture, not screenshot bytes).
#define W 430
#define H 932
static uint8_t *frame;
static void paint(int x0,int y0,int x1,int y1,int r,int g,int b) {
    for(int y=y0;y<y1;++y)for(int x=x0;x<x1;++x) {
        if(x<0||x>=W||y<0||y>=H)continue;
        uint8_t *p=frame+4*((size_t)y*W+x);
        p[0]=r;p[1]=g;p[2]=b;p[3]=255;
    }
}
static void board_tile(int row,int col,int r,int g,int b) {
    int x=23+(int)lround(col*385.0/8)+3;
    int y=216+(int)lround(row*385.0/8)+3;
    int xe=23+(int)lround((col+1)*385.0/8)-3;
    int ye=216+(int)lround((row+1)*385.0/8)-3;
    paint(x,y,xe,ye,r,g,b);
}
static void tray_tile(int x,int y,int r,int g,int b) {
    paint(x,y,x+19,y+19,r,g,b);
}
static void build_wood(void) {
    paint(0,0,W,H,192,137,106); // textured light-wood approximation
    paint(23,216,408,601,73,38,18);
    for(int r=0;r<8;++r)for(int c=0;c<8;++c) {
        int x0=23+(int)lround(c*385.0/8)+2;
        int y0=216+(int)lround(r*385.0/8)+2;
        int x1=23+(int)lround((c+1)*385.0/8)-2;
        int y1=216+(int)lround((r+1)*385.0/8)-2;
        paint(x0,y0,x1,y1,80,45,25);
    }
    board_tile(0,0,211,62,60);
    board_tile(1,3,148,91,204);
    board_tile(6,6,58,181,68);
    // Red 3x3, green L4, purple L4
    for(int r=0;r<3;++r)for(int c=0;c<3;++c)
        tray_tile(58+c*21,665+r*21,215,60,65);
    for(int r=0;r<3;++r) {
        tray_tile(205,665+r*21,55,175,60);
        tray_tile(338,665+r*21,155,85,207);
    }
    tray_tile(226,707,55,175,60);
    tray_tile(359,707,155,85,207);
}
static void check(const BLVisionResult *v,int flipped) {
    assert(v->flippedInput==flipped);
    uint64_t board=UINT64_C(1)<<(0*8+0);
    board|=UINT64_C(1)<<(1*8+3);
    board|=UINT64_C(1)<<(6*8+6);
    assert(v->board==board);
    assert(v->detectedPieces==3);
    assert(v->pieces[0].count==9);
    for(int i=0;i<9;++i) {
        assert(v->pieces[0].x[i]==i%3);
        assert(v->pieces[0].y[i]==i/3);
    }
    for(int p=1;p<3;++p) {
        assert(v->pieces[p].count==4);
        for(int i=0;i<3;++i) {assert(v->pieces[p].x[i]==0);assert(v->pieces[p].y[i]==i);}
        assert(v->pieces[p].x[3]==1&&v->pieces[p].y[3]==2);
    }
    assert(fabsf(v->firstX-47)<8 && fabsf(v->firstY-240)<8);
    assert(fabsf(v->lastX-384)<8 && fabsf(v->lastY-577)<8);
}
int main(void) {
    frame=malloc((size_t)W*H*4);assert(frame);
    build_wood();
    BLVisionResult upright={0};
    assert(BLVisionAnalyzeRGBA(frame,W,H,&upright));
    check(&upright,0);
    uint8_t *mirror=malloc((size_t)W*H*4);assert(mirror);
    for(int y=0;y<H;++y)
        memcpy(mirror+4*(size_t)y*W,frame+4*(size_t)(H-1-y)*W,W*4);
    BLVisionResult flipped={0};
    assert(BLVisionAnalyzeRGBA(mirror,W,H,&flipped));
    check(&flipped,1);
    paint(0,0,W,H,192,137,106);
    BLVisionResult none={0};
    assert(!BLVisionAnalyzeRGBA(frame,W,H,&none)); // no imaginary grid
    free(mirror);free(frame);
    puts("wood theme vision regression passed: board/tray, 3 shapes, reversed frame, no false grid");
    return 0;
}
