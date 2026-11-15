// Synthetic 430x932 watermelon palette: filled red tiles, empty dark-green
// board, themed gutter bleed, Z4 and five-bar tray. Derived from screenshot
// geometry without uploading the user's image to a public repository.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../src/BLVisionCore.h"
#define W 430
#define H 932
static unsigned char im[(size_t)W*H*4];
static void box(int x0,int y0,int x1,int y1,int r,int g,int b){
 for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++){
  if(x<0||y<0||x>=W||y>=H)continue;
  int k=4*(y*W+x);
  im[k]=r;im[k+1]=g;im[k+2]=b;im[k+3]=255;
 }
}
static void melon(int x,int y,int size){
 box(x+2,y+2,x+size-2,y+size-2,246,101,93);
 if(size>32)box(x+2,y+size-7,x+size-2,y+size-2,235,236,206);
}
int main(void){
 box(0,0,W,H,68,142,91);
 box(22,214,408,601,11,51,24);
 uint64_t expected=UINT64_C(0x7e0c8781f797e7c7);
 for(int r=0;r<8;r++)for(int c=0;c<8;c++){
  int x=24+c*48,y=216+r*48;
  box(x+2,y+2,x+46,y+46,17,56,27);
  if(expected&(UINT64_C(1)<<(r*8+c)))melon(x,y,48);
 }
 // The old vertical-seam median was contaminated by these nearby tiles,
 // making an entirely filled board. Intersections are the real gutters.
 for(int r=0;r<8;r++)for(int c=1;c<8;c++){
  int x=24+c*48,y=216+r*48+24;
  box(x-1,y-3,x+2,y+3,230,96,87);
 }
 melon(57,700,22);melon(79,700,22);
 melon(79,722,22);melon(101,722,22);
 for(int c=0;c<5;c++)melon(165+c*22,706,22);
 BLVisionResult out={0};
 assert(BLVisionAnalyzeRGBA(im,W,H,&out));
 assert(out.board==expected);
 assert(__builtin_popcountll(out.board)==37);
 assert(out.detectedPieces==2);
 assert(out.pieces[0].count==4 && out.pieces[1].count==5);
 for(int row=0;row<8;++row){
  uint64_t rowMask=UINT64_C(255)<<(8*row);
  assert((out.board&rowMask)!=rowMask);
 }
 puts("PASS 430x932 themed grid: no fake full row, 37 occupied, Z4 + five-bar");
 return 0;
}
