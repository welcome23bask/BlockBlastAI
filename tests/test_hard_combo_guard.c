// Regression targeting the user's real observation:
// v1.3 PINK COMBO dropped verified streaks when the board was crowded.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../src/BLTransition.h"

static BLPiece piece(const char *s) {
 BLPiece p={0};int x=0,y=0;
 for(;*s;++s) {
   if(*s=='/') {++y;x=0;continue;}
   if(*s=='#') {p.x[p.count]=x;p.y[p.count]=y;++p.count;}
   ++x;
 }
 return p;
}
static BLComboContext play(BLComboContext c,const BLSolution *sol,int moves) {
 for(int i=0;i<moves;++i)c=BLComboAdvance(c,sol->moves[i].linesCleared);
 return c;
}
int main(void) {
 BLComboContext due={.streak=25,.misses=2,.known=1};
 // 36/64 occupied cells. The first piece must complete row 1 at col 8:
 // any other first move breaks the combo. Clearing row 1 makes room for
 // the 2x2 piece while leaving a safe place for the 3-cell bar.
 uint64_t hard=(UINT64_C(0x7f)) |
               (UINT64_C(0x3f)<<8) |
               (UINT64_C(0x55)<<16) |
               (UINT64_C(0x55)<<24) |
               (UINT64_C(0x55)<<32) |
               (UINT64_C(0x7f)<<40) |
               (UINT64_C(0x55)<<48);
 BLPiece tray[3]={piece("#"),piece("##/##"),piece("###")};
 BLSolution answer=BLSolveWithCombo(hard,tray,due);
 assert(answer.movesFound==3);
 assert(answer.moves[0].linesCleared>0); // last chance MUST be rescued
 assert(play(due,&answer,3).streak>0);
 // There are situations with no possible clear in the entire tray.
 // Do NOT hallucinate a combo or refuse valid moves to protect it.
 BLPiece tiny[3]={piece("#"),piece("#"),piece("#")};
 BLSolution impossible=BLSolveWithCombo(0,tiny,due);
 assert(impossible.movesFound==3);
 assert(play(due,&impossible,3).streak==0);
 // One remaining setup move should still permit a later line clear.
 BLComboContext twoLeft={.streak=13,.misses=1,.known=1};
 BLSolution partial=BLSolveWithCombo(UINT64_C(0x7f),tray,twoLeft);
 assert(partial.movesFound==3);
 assert(play(twoLeft,&partial,2).streak>0);
 // Independent observation log persists through a brief missed capture
 // without guessing at a placement.
 BLComboTracker tracker={0};
 BLPiece before[3]={piece("#"),piece("##"),piece("###")};
 BLPiece after[3]={0}; after[1]=before[1]; after[2]=before[2];
 assert(!BLComboTrackerObserve(&tracker,UINT64_C(0x7f),before,100.0));
 assert(!tracker.streak.known);
 assert(BLComboTrackerObserve(&tracker,0,after,100.5));
 assert(tracker.streak.known&&tracker.streak.streak==1&&tracker.streak.misses==0);
 // One skipped frame is NOT evidence that the board or streak changed.
 assert(!BLComboTrackerObserve(&tracker,0,after,101.4));
 assert(tracker.streak.streak==1);
 // Contradictory board changes are not credited as clears.
 assert(!BLComboTrackerObserve(&tracker,UINT64_C(1)<<37,after,101.8));
 assert(!tracker.streak.known);
 // A long recording interruption cannot carry an unverified old combo.
 BLComboTrackerReset(&tracker);
 BLComboTrackerObserve(&tracker,UINT64_C(0x7f),before,200.0);
 BLComboTrackerObserve(&tracker,0,after,200.5);
 assert(tracker.streak.streak==1);
 assert(!BLComboTrackerObserve(&tracker,0,after,205.5));
 assert(!tracker.streak.known);
 puts("Hard-board combo guard, survival fallback and transient-tracker tests passed.");
 return 0;
}
