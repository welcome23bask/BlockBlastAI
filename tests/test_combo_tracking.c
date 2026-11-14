// Combo history must be derived from true board/tray transitions, not points.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../src/BLTransition.h"
static BLPiece single(void) {BLPiece p={0};p.count=1;p.x[0]=0;p.y[0]=0;return p;}
static BLPiece bar2(void) {BLPiece p={0};p.count=2;p.x[1]=1;return p;}
static uint64_t row7(void){return UINT64_C(0x7f);}
int main(void) {
 BLComboContext c={0};
 c=BLComboAdvance(c,0);assert(!c.known);
 c=BLComboAdvance(c,1);assert(c.known&&c.streak==1&&c.misses==0);
 c=BLComboAdvance(c,0);c=BLComboAdvance(c,0);
 assert(c.streak==1&&c.misses==2);
 c=BLComboAdvance(c,2);assert(c.streak==2&&c.misses==0);
 c=BLComboAdvance(c,0);c=BLComboAdvance(c,0);c=BLComboAdvance(c,0);
 assert(c.streak==0&&c.misses==0);
 // Regular observed single-cell placement completes a horizontal line.
 BLPiece before[3]={single(),bar2(),single()};
 BLPiece after[3]={0};after[1]=before[1];after[2]=before[2];
 BLMove move={0};
 assert(BLInferObservedPlacement(row7(),before,0,after,&move));
 assert(move.pieceIndex==0&&move.row==0&&move.col==7&&move.linesCleared==1);
 // A completed tray immediately replenishes three new shapes.
 BLPiece last[3]={0};last[2]=single();
 BLPiece refill[3]={single(),bar2(),single()};
 assert(BLInferObservedPlacement(row7(),last,0,refill,&move));
 assert(move.pieceIndex==2&&move.row==0&&move.col==7&&move.linesCleared==1);
 // A new tray on an unchanged board is NOT a placement.
 BLPiece oldEmpty[3]={0};
 assert(!BLInferObservedPlacement(0,oldEmpty,0,refill,&move));
 // A wrong board observation must never advance combo.
 uint64_t phantom=UINT64_C(1)<<37;
 assert(!BLInferObservedPlacement(row7(),before,phantom,after,&move));
 // Unknown score cannot be used as a starting streak.
 BLComboContext known={23,2,1};
 BLSolution a=BLSolveWithCombo(0,refill,known);
 assert(a.movesFound>=1 && a.movesFound<=3);
 assert(a.moves[0].before==0);
 puts("verified combo tracking regression passed");
 return 0;
}
