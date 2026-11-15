#include "../src/BLTrainedVision.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
int main(void) {
 const int w=400,h=900;
 unsigned char *im=calloc(w*h,4);assert(im);
 for(int i=0;i<w*h;++i){im[4*i]=36;im[4*i+1]=43;im[4*i+2]=85;im[4*i+3]=255;}
 BLVisionResult s={0};s.firstX=55;s.lastX=335;s.firstY=175;s.lastY=455;
 for(int r=0;r<8;++r)for(int c=0;c<8;++c)if((r+c)%4==0){
  int x=55+c*40,y=175+r*40;
  s.board|=UINT64_C(1)<<(r*8+c);
  for(int dy=-8;dy<=8;++dy)for(int dx=-8;dx<=8;++dx){
   int i=((y+dy)*w+x+dx)*4;im[i]=200;im[i+1]=210;im[i+2]=50;
  }
 }
 assert(BLTrainedVisionOpinionRGBA(im,w,h,&s)==1);
 s.board ^= UINT64_C(1)<<3;
 assert(BLTrainedVisionOpinionRGBA(im,w,h,&s)==-1);
 s.board ^= UINT64_C(1)<<3;
 for(int i=0;i<w*h;++i){im[4*i]=190;im[4*i+1]=145;im[4*i+2]=185;}
 assert(BLTrainedVisionOpinionRGBA(im,w,h,&s)==0);
 free(im);puts("PASS learned skin gate, agreement and conflict rejection");return 0;
}
