// Synthetic regression for the user's red board theme, not their actual screenshot.
// The first tray item is a 3x3 red piece with TWO bright rows and a darker
// third row. Earlier theme adaptation cropped it to a 3x2, guiding six blocks.
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/BLVisionCore.h"
#define W 430
#define H 932
static uint8_t *px;
static void rect(int x0,int y0,int x1,int y1,int r,int g,int b) {
    for(int y=y0;y<y1;++y)for(int x=x0;x<x1;++x) {
        if(x<0||y<0||x>=W||y>=H)continue;
        uint8_t *p=px+4*((size_t)y*W+x);
        p[0]=r;p[1]=g;p[2]=b;p[3]=255;
    }
}
static void tile(int x,int y,int r,int g,int b) {
    rect(x,y,x+21,y+21,r,g,b);
}
static void draw(void) {
    rect(0,0,W,H,154,46,24);
    // Standard portrait 8x8 playfield, strongly separated by dark gutters.
    rect(20,214,410,604,36,22,17);
    for(int row=0;row<8;++row)for(int col=0;col<8;++col) {
        int x0=22+(int)lround(col*386.0/8)+2;
        int x1=22+(int)lround((col+1)*386.0/8)-2;
        int y0=216+(int)lround(row*386.0/8)+2;
        int y1=216+(int)lround((row+1)*386.0/8)-2;
        rect(x0,y0,x1,y1,61,34,26);
    }
    // Yellow overlay border on 2x3 bottom-right cells. Borders alone are
    // NOT evidence that six physical pieces actually exist on the board.
    for(int row=6;row<8;++row)for(int col=5;col<8;++col) {
        int x=22+(int)lround(col*386.0/8)+3;
        int y=216+(int)lround(row*386.0/8)+3;
        rect(x,y,x+40,y+2,247,206,4);
        rect(x,y+40,x+40,y+42,247,206,4);
        rect(x,y,x+2,y+42,247,206,4);
        rect(x+38,y,x+40,y+42,247,206,4);
    }
    // P1: 3x3 red with light top rows and a real darker bottom row.
    for(int row=0;row<3;++row)for(int col=0;col<3;++col) {
        int x=58+21*col,y=680+21*row;
        tile(x,y,row<2?243:214,row<2?104:42,row<2?72:21);
    }
    // P2: yellow T, four blocks.
    tile(204,691,246,190,2);
    for(int col=0;col<3;++col)tile(183+21*col,712,246,190,2);
    // P3: yellow L, four blocks.
    tile(351,691,246,190,2);
    for(int col=0;col<3;++col)tile(309+21*col,712,246,190,2);
}
static void verify(BLVisionResult *v,int flipped) {
    assert(v->flippedInput==flipped);
    assert(v->board==0);
    assert(v->detectedPieces==3);
    assert(v->pieces[0].count==9);
    for(int row=0;row<3;++row)for(int col=0;col<3;++col) {
        int found=0;
        for(int k=0;k<v->pieces[0].count;++k)
            if(v->pieces[0].x[k]==col && v->pieces[0].y[k]==row)found=1;
        assert(found);
    }
    assert(v->pieces[1].count==4);
    assert(v->pieces[2].count==4);
    assert(fabs(v->firstY-240.125)<10);
}
int main(void) {
    px=calloc((size_t)W*H,4);assert(px);
    draw();
    BLVisionResult out={0};
    assert(BLVisionAnalyzeRGBA(px,W,H,&out));
    verify(&out,0);
    uint8_t *flip=malloc((size_t)W*H*4);assert(flip);
    for(int y=0;y<H;++y)
        memcpy(flip+4*(size_t)y*W,px+4*(size_t)(H-1-y)*W,(size_t)W*4);
    BLVisionResult reverse={0};
    assert(BLVisionAnalyzeRGBA(flip,W,H,&reverse));
    verify(&reverse,1);
    free(flip);free(px);
    puts("red adaptive theme: dark-bottom 3x3 + T + L, 9/4/4 pieces, yellow overlay, flip passed");
    return 0;
}
