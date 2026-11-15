// Paired synthetic full-game benchmark. Not an exact game engine/score model.
// Both strategies see identical seeded future tray draws and start boards.
// Compare baseline v3.3 Smart Survival against optional guarded v3.4 choice.
// The existing 3-try clear override is applied to BOTH strategies.
#include "../src/BLFuture100.h"
#include "../src/BLV25Strategy.h"
#include "../src/BLThreeMoveClear.h"
#include "../src/BLGuideSafety.h"
#include "../src/BLFrameSafety.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

typedef struct {
    uint32_t rng;
    int moves,lines,completions,invalid,comboBreaks;
    int avoidableDeadlines,guardedChanges;
    int scoreProxy;
} BLSim;
static uint32_t roll(BLSim *sim) {
    uint32_t x=sim->rng;
    x^=x<<13;x^=x>>17;x^=x<<5;
    sim->rng=x?x:UINT32_C(0x6d2b79f5);
    return sim->rng;
}
static void piece(BLSim *sim,BLPiece *out) {
    memset(out,0,sizeof(*out));
    uint64_t mask=0;int w=0,h=0,cells=0;
    // Observed masks are an approximate dataset, not verified game odds.
    assert(BLFuture100DescribeShape((int)(roll(sim)%35U),&mask,&w,&h,&cells));
    for(int y=0;y<h;++y)for(int x=0;x<w;++x)
        if(mask&(UINT64_C(1)<<(y*8+x))) {
            int k=out->count++;
            out->x[k]=x;out->y[k]=y;
        }
    assert(out->count==cells);
}
static uint64_t startBoard(BLSim *sim,int mode) {
    if(mode==0)return 0;
    uint64_t board=0;
    for(int r=0;r<8;++r)for(int c=0;c<8;++c) {
        uint32_t n=roll(sim)%100U;
        int filled=mode==1 ? (n<23) :
            (c<4 ? n<47:n<12); // shifted-heavy occupancy, not exact game
        if(filled)board|=UINT64_C(1)<<(r*8+c);
    }
    // Real settled game boards have no complete, uncleared lines.
    for(int r=0;r<8;++r){
        uint64_t row=UINT64_C(255)<<(8*r);
        if((board&row)==row)board&=~(UINT64_C(1)<<(r*8+7));
    }
    for(int c=0;c<8;++c){
        uint64_t col=UINT64_C(0x0101010101010101)<<c;
        if((board&col)==col)board&=~(UINT64_C(1)<<(7*8+c));
    }
    assert(!BLBoardHasUnclearedLine(board));
    return board;
}
static BLSim play(uint32_t seed,int mode,int guarded) {
    BLSim sim={0};sim.rng=seed?seed:1;
    uint64_t board=startBoard(&sim,mode);
    BLComboContext combo={0};
    // Cap to a small, repeatable number of trays: this is an inexpensive
    // benchmark smoke suite, not a claim of converged long-run performance.
    for(int round=0;round<8;++round){
        BLPiece tray[3]={{0}};
        for(int p=0;p<3;++p)piece(&sim,&tray[p]);
        for(int step=0;step<3;++step) {
            BLTwoStrategies options=BLV25FindTwoStrategies(board,tray,combo,3000);
            BLSolution chosen=options.smartSurvival;
            if(chosen.movesFound>0 &&
               !BLNextGuideSafe(board,tray,&chosen)) {
                BLTwoStrategies backup=BLFindTwoStrategies(board,tray,combo,3000);
                chosen=backup.smartSurvival;
            }
            if(guarded && chosen.movesFound>0) {
                int changed=0;
                chosen=BLFuture100GuardedChoice(board,tray,combo,&chosen,
                    &options.keepCombo,&options.maxLines,&changed);
                sim.guardedChanges+=changed;
            }
            // The real v3.4 GUI always overrides the heuristic with a
            // validated, visible clear route, so apply same rule to baseline.
            BLThreeMovePlan clear=BLFindThreeMoveClear(board,tray,combo,3000);
            if(clear.found && BLGuideValidateSolution(board,tray,&clear.solution)
                           && BLNextGuideSafe(board,tray,&clear.solution))
                chosen=clear.solution;
            if(chosen.movesFound<1) {
                sim.completions=round;
                return sim;
            }
            if(!BLNextGuideSafe(board,tray,&chosen)) {
                ++sim.invalid;
                sim.completions=round;
                return sim;
            }
            BLMove action=chosen.moves[0];
            if(action.pieceIndex<0||action.pieceIndex>2||
               tray[action.pieceIndex].count==0 ||
               action.before!=board) {
                ++sim.invalid;sim.completions=round;return sim;
            }
            if(combo.known && combo.streak>0 && combo.misses==2 &&
               !action.linesCleared && clear.found && clear.firstClear==1)
                ++sim.avoidableDeadlines;
            if(combo.known && combo.streak>0 && combo.misses==2 &&
               !action.linesCleared)++sim.comboBreaks;
            board=action.after;
            sim.moves++;
            sim.lines+=action.linesCleared;
            sim.scoreProxy+=tray[action.pieceIndex].count*10+
                            action.linesCleared*100;
            combo=BLComboAdvance(combo,action.linesCleared);
            memset(&tray[action.pieceIndex],0,sizeof(tray[action.pieceIndex]));
            assert(!BLBoardHasUnclearedLine(board));
        }
        sim.completions=round+1;
    }
    return sim;
}
static int cmp(const void *a,const void *b) {
    int x=*(const int *)a,y=*(const int *)b;
    return (x>y)-(x<y);
}
int main(void) {
    const int trials=10;
    const uint32_t seeds[]={0x3011e137U,0x7e91a401U,0x1911c551U,
        0x62248d01U,0x81021a79U,0x2246c533U,0x924b30c1U,
        0x1a73e251U,0xa10074d3U,0x041f12e1U};
    int totalInvalid=0;
    for(int mode=0;mode<3;++mode){
        long baseMoves=0,nextMoves=0,baseLines=0,nextLines=0;
        long baseScore=0,nextScore=0,baseBreak=0,nextBreak=0;
        int bArr[trials],nArr[trials],upgraded=0,baseDeadline=0,newDeadline=0;
        for(int i=0;i<trials;++i){
            uint32_t seed=seeds[i]^((uint32_t)mode*UINT32_C(0x9e3779b9));
            BLSim base=play(seed,mode,0);
            BLSim changed=play(seed,mode,1);
            baseMoves+=base.moves;nextMoves+=changed.moves;
            baseLines+=base.lines;nextLines+=changed.lines;
            baseScore+=base.scoreProxy;nextScore+=changed.scoreProxy;
            baseBreak+=base.comboBreaks;nextBreak+=changed.comboBreaks;
            baseDeadline+=base.avoidableDeadlines;
            newDeadline+=changed.avoidableDeadlines;
            totalInvalid+=base.invalid+changed.invalid;
            bArr[i]=base.moves;nArr[i]=changed.moves;
            upgraded+=changed.guardedChanges;
        }
        qsort(bArr,trials,sizeof(int),cmp);
        qsort(nArr,trials,sizeof(int),cmp);
        printf("mode=%d games=%d avg_moves=%.2f/%.2f p10_moves=%d/%d "
               "avg_lines=%.2f/%.2f score_proxy=%.1f/%.1f "
               "combo_breaks=%ld/%ld avoidable_deadlines=%d/%d "
               "guarded_swaps=%d\n",
               mode,trials,baseMoves/(double)trials,nextMoves/(double)trials,
               bArr[0],nArr[0],baseLines/(double)trials,nextLines/(double)trials,
               baseScore/(double)trials,nextScore/(double)trials,
               baseBreak,nextBreak,baseDeadline,newDeadline,upgraded);
    }
    assert(totalInvalid==0);
    puts("PASS paired deterministic synthetic games: no illegal placements");
    puts("IMPORTANT: synthetic proxy score; no proof of real Block Blast advantage.");
    return 0;
}
