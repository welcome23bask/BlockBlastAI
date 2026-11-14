// SMART SURVIVAL field regressions: protect an active streak even if a
// one-line clear is the correct choice instead of a five-line third move.
#include "../src/BLDualStrategy.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
static BLPiece square3(void) {
 BLPiece p={0};
 for(int r=0;r<3;r++)for(int c=0;c<3;c++) {
   p.x[p.count]=c;p.y[p.count]=r;++p.count;
 } return p;
}
static BLPiece rightL(void) {
 BLPiece p={0};p.count=4;
 p.x[0]=1;p.y[0]=0;
 p.x[1]=1;p.y[1]=1;
 p.x[2]=0;p.y[2]=2;p.x[3]=1;p.y[3]=2;
 return p;
}
static BLPiece one(void) {
 BLPiece p={0};p.count=1;return p;
}
static uint64_t board(void) {
 uint64_t b=0;
 for(int r=0;r<5;r++)for(int c=3;c<=4;c++)
   b|=UINT64_C(1)<<(r*8+c);
 for(int r=5;r<=6;r++)b|=UINT64_C(1)<<(r*8+6);
 return b;
}
static void legal(const BLSolution *sol,const BLPiece pieces[3],uint64_t b) {
 unsigned used=0;
 for(int i=0;i<sol->movesFound;i++) {
   BLMove m=sol->moves[i];
   assert(m.pieceIndex>=0&&m.pieceIndex<3&&!(used&(1u<<m.pieceIndex)));
   used|=1u<<m.pieceIndex;
   assert(m.before==b);
   uint64_t x=b;
   for(int j=0;j<pieces[m.pieceIndex].count;j++) {
      int c=m.col+pieces[m.pieceIndex].x[j];
      int r=m.row+pieces[m.pieceIndex].y[j];
      assert(r>=0&&r<8&&c>=0&&c<8);
      uint64_t cell=UINT64_C(1)<<(8*r+c);
      assert(!(x&cell));x|=cell;
   }
   int clears=0;uint64_t er=0;
   for(int r=0;r<8;r++) {
      uint64_t v=UINT64_C(255)<<(r*8);
      if((x&v)==v){er|=v;++clears;}
   }
   for(int c=0;c<8;c++) {
      uint64_t v=UINT64_C(0x0101010101010101)<<c;
      if((x&v)==v){er|=v;++clears;}
   }
   assert(clears==m.linesCleared);
   b=x&~er;assert(m.after==b);
 }
}
static uint32_t rng=0x09ad2f31;
static uint32_t next(void) {
 uint32_t x=rng;x^=x<<13;x^=x>>17;x^=x<<5;return rng=x;
}
int main(void) {
 BLPiece pieces[3]={square3(),rightL(),square3()};
 BLComboContext due={.known=1,.streak=17,.misses=2};
 BLTwoStrategies sol=BLFindTwoStrategies(board(),pieces,due,500000);
 assert(sol.exhaustive&&sol.smartSurvival.movesFound==3);
 assert(sol.maxLines.totalLines==5);
 assert(sol.smartComboPreserved==1);
 assert(sol.smartSurvival.moves[0].linesCleared>=1);
 assert(sol.smartSurvival.totalLines>=1);
 legal(&sol.smartSurvival,pieces,board());
 printf("User board: MAX LINES=%d; SMART SURVIVAL=%d; first clear=%d; future quality=%d\n",
        sol.maxLines.totalLines,sol.smartSurvival.totalLines,
        sol.smartSurvival.moves[0].linesCleared,sol.smartFutureQuality);
 // A verified chain with one tiny clear must remain better than blindly
 // waiting for a bigger clear that would expire the last-chance window.
 BLPiece onePieces[3]={one(),one(),one()};
 BLTwoStrategies single=BLFindTwoStrategies(UINT64_C(0x7f),onePieces,due,180000);
 assert(single.smartSurvival.movesFound==3);
 assert(single.smartSurvival.moves[0].linesCleared>0);
 legal(&single.smartSurvival,onePieces,UINT64_C(0x7f));
 // If no clear is possible, the engine must still legally place all three.
 BLTwoStrategies impossible=BLFindTwoStrategies(0,onePieces,due,180000);
 assert(impossible.smartSurvival.movesFound==3);
 legal(&impossible.smartSurvival,onePieces,0);
 // Randomized crowding/partial tray must NEVER generate illegal highlights.
 for(int k=0;k<240;++k) {
   BLPiece p[3]={square3(),rightL(),one()};
   uint64_t b=0;
   for(int j=0;j<k%51;++j)b|=UINT64_C(1)<<(next()%64);
   if(k%4==0)p[2].count=0;
   BLTwoStrategies res=BLFindTwoStrategies(b,p,due,9000);
   legal(&res.smartSurvival,p,b);
   assert(res.exploredNodes<=9000);
 }
 puts("SMART SURVIVAL combo-first, one-line, full-tray, random legality passed.");
 return 0;
}
