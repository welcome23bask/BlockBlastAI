// Deterministic palette/texture fuzz: 128 distinct synthetic board + tray skins.
// No official Block Blast assets are reproduced. Shape truth is independent of hue.
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/BLVisionCore.h"
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
static int valid(const BLVisionResult *r){
 if(r->board!=0 || r->detectedPieces!=3)return 0;
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
 int pass=0;
 for(int n=0;n<128;n++){
  draw(n);
  BLVisionResult r={0};
  int ok=BLVisionAnalyzeRGBA(im,W,H,&r);
  if(ok&&valid(&r))pass++;
  else fprintf(stderr,"FAIL palette %d: scanned=%d board=%llx pieces=%d counts=%d/%d/%d\n",n,ok,(unsigned long long)r.board,r.detectedPieces,r.pieces[0].count,r.pieces[1].count,r.pieces[2].count);
 }
 printf("Palette fuzz: %d/128 synthetic skins parsed correctly\n",pass);
 free(im);
 assert(pass==128);
 return 0;
}
