// Synthetic regression matching the structure of the user's green-theme
// screenshot. The private screenshot itself is NOT stored in this repository.
// Three 1x5 light-green pieces, 3 occupied squares, textured background and
// false-delta flecks far below the tray that previously enlarged the pieces.
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/BLVisionCore.h"
#define W 430
#define H 932
static uint8_t *px;
static void paint(int x0,int y0,int x1,int y1,int r,int g,int b) {
 for(int y=y0;y<y1;++y)for(int x=x0;x<x1;++x){
  if(x<0||x>=W||y<0||y>=H)continue;
  uint8_t *p=px+4*((size_t)y*W+x);
  p[0]=r;p[1]=g;p[2]=b;p[3]=255;
 }
}
static void build(void) {
 paint(0,0,W,H,94,141,87);
 // centered green board with distinct four boundaries and seven seams
 paint(21,214,409,604,40,59,46);
 for(int r=0;r<8;r++)for(int c=0;c<8;c++){
  int left=22+(int)lround(c*386.0/8)+2;
  int right=22+(int)lround((c+1)*386.0/8)-2;
  int top=216+(int)lround(r*386.0/8)+2;
  int bottom=216+(int)lround((r+1)*386.0/8)-2;
  paint(left,top,right,bottom,47,64,51);
 }
 for(int r=4;r<=6;r++){
  int x0=22+(int)lround(4*386.0/8)+3;
  int y0=216+(int)lround(r*386.0/8)+3;
  paint(x0,y0,x0+42,y0+42,203,231,215);
 }
 // Three horizontal, five-long pieces on exactly the same tray row.
 int bases[]={39,163,288};
 for(int s=0;s<3;s++)for(int c=0;c<5;c++) {
  int x=bases[s]+21*c;
  paint(x,703,x+19,723,s==1?245:209,s==1?251:242,s==1?248:222);
 }
 // Simulate texture pixels below the tray, classified by the older weak
 // color delta but not by the new bright-piece population.
 paint(240,867,248,873,63,110,62);
 paint(388,871,399,877,60,112,68);
}
static void verify(BLVisionResult *v,int flipped) {
 uint64_t expected=(UINT64_C(1)<<(4*8+4))|
                   (UINT64_C(1)<<(5*8+4))|
                   (UINT64_C(1)<<(6*8+4));
 assert(v->board==expected);
 assert(v->detectedPieces==3);
 assert(v->flippedInput==flipped);
 for(int i=0;i<3;i++) {
  assert(v->pieces[i].count==5);
  for(int j=0;j<5;j++) {
   assert(v->pieces[i].x[j]==j);
   assert(v->pieces[i].y[j]==0);
  }
 }
}
int main(void) {
 px=calloc((size_t)W*H,4);assert(px);
 build();
 BLVisionResult out={0};
 assert(BLVisionAnalyzeRGBA(px,W,H,&out));
 verify(&out,0);
 uint8_t *flipped=malloc((size_t)W*H*4);assert(flipped);
 for(int y=0;y<H;y++)memcpy(flipped+4*(size_t)y*W,
                             px+4*(size_t)(H-1-y)*W,W*4);
 BLVisionResult reversed={0};
 assert(BLVisionAnalyzeRGBA(flipped,W,H,&reversed));
 verify(&reversed,1);
 puts("green textured-theme three 1x5 pieces, three occupied cells and flipped orientation passed");
 free(flipped);free(px);
 return 0;
}
