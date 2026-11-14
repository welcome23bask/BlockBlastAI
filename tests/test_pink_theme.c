// Regression from actual 480530-point pink-skin screenshot: near-background
// pastel 3x3 tray piece was invisible to the old fixed RGB palette.
// Synthetic color/geometry fixture preserves its visible board and tray.
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/BLVisionCore.h"
#define W 430
#define H 932
static uint8_t *im;
static const int rows[15]={0,0,2,3,3,4,4,4,4,5,7,7,7,7,7};
static const int cols[15]={5,7,5,4,5,1,4,5,7,4,1,3,4,6,7};
static void rect(int x0,int y0,int x1,int y1,int r,int g,int b) {
 for(int y=y0;y<y1;++y)for(int x=x0;x<x1;++x) {
  if(x<0||x>=W||y<0||y>=H)continue;
  uint8_t *p=im+4*((size_t)y*W+x);p[0]=r;p[1]=g;p[2]=b;p[3]=255;
 }
}
static void tray(int x,int y,int light) {
 rect(x,y,x+20,y+20,light?249:197,light?122:36,light?172:98);
}
static void build(void) {
 rect(0,0,W,H,234,148,172);
 rect(20,213,410,604,175,115,135);
 rect(21,214,409,602,140,97,114);
 for(int r=0;r<8;r++)for(int c=0;c<8;c++) {
  int x0=21+(int)lround(c*388./8.)+2;
  int x1=21+(int)lround((c+1)*388./8.)-2;
  int y0=214+(int)lround(r*388./8.)+2;
  int y1=214+(int)lround((r+1)*388./8.)-2;
  rect(x0,y0,x1,y1,153,105,120);
 }
 for(int i=0;i<15;i++) {
  int r=rows[i],c=cols[i],light=i%2;
  int x0=21+(int)lround(c*388./8.)+4;
  int x1=21+(int)lround((c+1)*388./8.)-3;
  int y0=214+(int)lround(r*388./8.)+4;
  int y1=214+(int)lround((r+1)*388./8.)-3;
  rect(x0,y0,x1,y1,light?249:197,light?122:36,light?172:98);
 }
 // 4-cell left L, low-contrast pastel 3x3, deep-pink 2x3 rectangle
 tray(68,680,0);tray(68,702,0);tray(68,724,0);tray(90,724,0);
 for(int r=0;r<3;r++)for(int c=0;c<3;c++)tray(182+c*22,680+r*22,1);
 for(int r=0;r<3;r++)for(int c=0;c<2;c++)tray(320+c*22,680+r*22,0);
}
static void check(BLVisionResult *v,int flip) {
 uint64_t expected=0;
 for(int i=0;i<15;i++)expected|=UINT64_C(1)<<(rows[i]*8+cols[i]);
 assert(v->flippedInput==flip);
 assert(v->board==expected);
 assert(v->detectedPieces==3);
 assert(v->pieces[0].count==4 && v->pieces[1].count==9 && v->pieces[2].count==6);
 assert(v->pieces[0].x[3]==1 && v->pieces[0].y[3]==2);
 for(int r=0;r<3;r++)for(int c=0;c<3;c++) {
  int k=r*3+c;
  assert(v->pieces[1].x[k]==c && v->pieces[1].y[k]==r);
 }
 assert(fabsf(v->firstX-45)<8 && fabsf(v->firstY-238)<8);
}
int main(void) {
 im=malloc((size_t)W*H*4);assert(im);build();
 BLVisionResult a={0},b={0},blank={0};
 assert(BLVisionAnalyzeRGBA(im,W,H,&a));check(&a,0);
 uint8_t *mirrored=malloc((size_t)W*H*4);assert(mirrored);
 for(int y=0;y<H;++y)memcpy(mirrored+4*(size_t)y*W,
                            im+4*(size_t)(H-1-y)*W,W*4);
 assert(BLVisionAnalyzeRGBA(mirrored,W,H,&b));check(&b,1);
 rect(0,0,W,H,234,148,172);
 assert(!BLVisionAnalyzeRGBA(im,W,H,&blank));
 free(mirrored);free(im);
 puts("pink-theme tests passed: 15 cells, L4/3x3/2x3, reverse, blank");
 return 0;
}
