// Three exact zero-mismatch moves reconstructed from the user's uploaded
// 4x Bazz1TV recording. These positions are replayable ground-truth examples,
// not proof that the solver is optimal at the entire video.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../src/BLTransition.h"
static uint64_t board64(const char *s) {
 uint64_t b=0;
 for(int i=0;i<64;++i) {assert(s[i]=='0'||s[i]=='1');if(s[i]=='1')b|=UINT64_C(1)<<i;}
 return b;
}
static BLPiece piece(const char *pattern) {
 BLPiece p={0};int x=0,y=0;
 for(const char *s=pattern;*s;++s) {
  if(*s=='/') {++y;x=0;continue;}
  if(*s=='#') {p.x[p.count]=x;p.y[p.count]=y;++p.count;}
  ++x;
 }
 return p;
}
static void check(int tsec,const char *before,const char *after,
                  const char *shape,int row,int col,int lines) {
 BLPiece old[3]={0},replenished[3]={piece("#"),piece("##"),piece("###")};
 old[1]=piece(shape);
 BLMove detected={0};
 int okay=BLInferObservedPlacement(board64(before),old,board64(after),replenished,&detected);
 if(!okay)fprintf(stderr,"Failed Bazz case %d seconds\n",tsec);
 assert(okay);
 assert(detected.pieceIndex==1);
 assert(detected.row==row-1 && detected.col==col-1);
 assert(detected.linesCleared==lines);
 assert(detected.before==board64(before));
 assert(detected.after==board64(after));
}
int main(void) {
 // 2:10.6: 3x3 square clears 3 columns; occupied cells 23 => 8.
 check(130,"1111000011110000000000000000000000000000011101001111010011111100",
       "1000000010000000000000000000000000000000000001001000010010001100",
       "###/###/###",3,2,3);
 // 2:41.6: 3x3 square clears 3 columns; occupied cells 27 => 12.
 check(161,"0000110000001100000011001110000011100000111011001110110011101100",
       "0000110000001100000011000000000000000000000011000000110000001100",
       "###/###/###",1,1,3);
 // 5:07.3: zigzag clears 2 prepared lines; 30 => 18.
 check(307,"0001111100011111000111110001111100011111000111000001100000000000",
       "0001001100010011000100110001001100010011000100000001001000000000",
       ".##/##.",7,5,2);
 puts("Three exact Bazz1TV clear transitions replayed correctly.");
 return 0;
}
