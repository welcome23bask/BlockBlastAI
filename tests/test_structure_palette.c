// Deterministic palette/texture fuzz: 128 distinct synthetic board + tray skins.
// No official Block Blast assets are reproduced. Shape truth is independent of hue.
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/BLStructureVision.h"

#define W 430
#define H 932
static unsigned char *im;
static void rgb(int x0,int y0,int x1,int y1,int r,int g,int b) {
 for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++){
  if(x<0||y<0||x>=W||y>=H)continue;
  unsigned char *p=im+4*((size_t)y*W+x);
  p[0]=r;p[1]=g;p[2]=b;p[3]=255;
 }
}
static int clamp(int x) {return x<1?1:(x>254?254:x);}
static void piece(int x,int y,int tile[3]){
 rgb(x,y,x+21,y+21,tile[0],tile[1],tile[2]);
 rgb(x+2,y+2,x+19,y+6,clamp(tile[0]+7),clamp(tile[1]+7),clamp(tile[2]+7));
}
static void draw(int n) {
 int bg[3]={48+(n*31)%130,49+(n*43)%130,51+(n*67)%130};
 int opposite=(n%2)?-1:1;
 int board[3],empty[3],tile[3];
 for(int k=0;k<3;k++){
  board[k]=clamp(bg[k]+opposite*(75+(n%3)*6));
  empty[k]=clamp(board[k]-(board[k]>100?11:-11));
  tile[k]=clamp(bg[k]-(bg[k]>130?96:-96));
 }
 rgb(0,0,W,H,bg[0],bg[1],bg[2]);
 rgb(20,214,410,604,board[0],board[1],board[2]);
 for(int row=0;row<8;row++)for(int col=0;col<8;col++){
  int x0=22+(int)lround(col*386./8)+2;
  int x1=22+(int)lround((col+1)*386./8)-2;
  int y0=216+(int)lround(row*386./8)+2;
  int y1=216+(int)lround((row+1)*386./8)-2;
  rgb(x0,y0,x1,y1,empty[0],empty[1],empty[2]);
 }
 int p0[3],p1[3],p2[3];
 for(int k=0;k<3;k++){
  p0[k]=tile[k];
  p1[k]=clamp(bg[k]+(bg[k]>130?-110:110));
  p2[k]=clamp(bg[k]+((k+n)%2?115:-115));
 }
 for(int row=0;row<3;row++)for(int col=0;col<3;col++){
  int q[3]={p0[0],p0[1],p0[2]};
  if(row==2)for(int k=0;k<3;k++)q[k]=clamp(q[k]+(bg[k]-q[k])/5);
  piece(58+21*col,679+21*row,q);
 }
 piece(204,691,p1);
 for(int col=0;col<3;col++)piece(183+21*col,712,p1);
 piece(351,691,p2);
 for(int col=0;col<3;col++)piece(309+21*col,712,p2);
}
static uint64_t occupied_mask(void){
 return (UINT64_C(1)<<(2*8+3)) | (UINT64_C(1)<<(4*8+6)) | (UINT64_C(1)<<(6*8+1));
}
static void draw_occupied(int n) {
 int bg[3]={48+(n*31)%130,49+(n*43)%130,51+(n*67)%130};
 int sign=(n%2)?-1:1,colors[3];
 for(int k=0;k<3;k++){
  int board=clamp(bg[k]+sign*(75+(n%3)*6));
  int empty=clamp(board-(board>100?11:-11));
  colors[k]=clamp(empty+(empty>124?-115:115));
 }
 const int rr[3]={2,4,6},cc[3]={3,6,1};
 for(int i=0;i<3;i++){
  int x0=22+(int)lround(cc[i]*386.0/8)+2;
  int x1=22+(int)lround((cc[i]+1)*386.0/8)-2;
  int y0=216+(int)lround(rr[i]*386.0/8)+2;
  int y1=216+(int)lround((rr[i]+1)*386.0/8)-2;
  rgb(x0,y0,x1,y1,colors[0],colors[1],colors[2]);
 }
}
static int valid(const BLVisionResult *r,uint64_t expected){
 if(r->board!=expected || r->detectedPieces!=3)return 0;
 if(r->pieces[0].count!=9 || r->pieces[1].count!=4 || r->pieces[2].count!=4)return 0;
 for(int row=0;row<3;row++)for(int col=0;col<3;col++){
  int found=0;
  for(int j=0;j<r->pieces[0].count;j++)
   if(r->pieces[0].x[j]==col && r->pieces[0].y[j]==row)found=1;
  if(!found)return 0;
 }
 return 1;
}
int main(void){
 im=calloc((size_t)W*H,4);assert(im);
 int pass=0,filledPass=0;
 for(int n=0;n<128;n++){
  draw(n);
  BLStructureReset();
  BLVisionResult r={0};
  int ok=BLStructureAnalyzeRGBA(im,W,H,&r);
  if(ok&&valid(&r,0))pass++;
  else fprintf(stderr,"FAIL empty palette %d: ok=%d board=%llx counts=%d/%d/%d\\n",n,ok,(unsigned long long)r.board,r.pieces[0].count,r.pieces[1].count,r.pieces[2].count);
  draw_occupied(n);
  BLStructureReset();
  BLVisionResult filled={0};
  int bok=BLStructureAnalyzeRGBA(im,W,H,&filled);
  if(bok&&valid(&filled,occupied_mask()))filledPass++;
  else fprintf(stderr,"FAIL occupied palette %d: scanned=%d board=%llx expected=%llx pieces=%d counts=%d/%d/%d\\n",n,bok,(unsigned long long)filled.board,(unsigned long long)occupied_mask(),filled.detectedPieces,filled.pieces[0].count,filled.pieces[1].count,filled.pieces[2].count);
 }
 printf("Structure/contrast fuzz: %d/128 empty and %d/128 occupied synthetic skins parsed correctly\n",pass,filledPass);
 free(im);
 assert(pass==128 && filledPass==128);
 return 0;
}
