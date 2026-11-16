// Research-only 8x8 / three-visible-piece legality regression tests.
// Does not call the actual Block Blast app or rely on proprietary scoring.
#include "../src/BLDualStrategy.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void BLSetTrainWeights(const int weights[4]);
int BLResearchPairCoverage(uint64_t board);
int BLResearchSampledFutureBonus(uint64_t board);
BLTwoStrategies BLV17FindTwoStrategies(uint64_t board,const BLPiece pieces[3],
                                      BLComboContext combo,uint32_t budget);
static uint64_t place_checked(uint64_t b,uint64_t mask,int *cleared) {
    uint64_t all=b|mask,erase=0;
    *cleared=0;
    for(int r=0;r<8;r++) {
        uint64_t m=UINT64_C(0xff)<<(8*r);
        if((all&m)==m){erase|=m;(*cleared)++;}
    }
    for(int c=0;c<8;c++) {
        uint64_t m=UINT64_C(0x0101010101010101)<<c;
        if((all&m)==m){erase|=m;(*cleared)++;}
    }
    return all&~erase;
}
static void validate_plan(uint64_t board,const BLPiece pieces[3],
                          const BLSolution *plan,int expected) {
    assert(expected<0 || plan->movesFound==expected);
    assert(plan->movesFound>=0 && plan->movesFound<=3);
    unsigned used=0;
    for(int i=0;i<plan->movesFound;i++) {
        BLMove m=plan->moves[i];
        assert(m.pieceIndex>=0&&m.pieceIndex<3);
        assert(!(used&(1u<<m.pieceIndex)));
        used|=1u<<m.pieceIndex;
        assert(m.row>=0&&m.row<8&&m.col>=0&&m.col<8);
        assert(m.before==board);
        const BLPiece *piece=&pieces[m.pieceIndex];
        uint64_t mask=0;
        for(int k=0;k<piece->count;k++) {
            int row=m.row+piece->y[k],col=m.col+piece->x[k];
            assert(row>=0&&row<8&&col>=0&&col<8);
            uint64_t bit=UINT64_C(1)<<(8*row+col);
            assert(!(mask&bit));
            mask|=bit;
        }
        assert(!(mask&board)); // Cannot place on occupied squares.
        int clears=0;
        uint64_t after=place_checked(board,mask,&clears);
        assert(m.linesCleared==clears&&m.after==after);
        board=after;
    }
}
int main(void) {
    int lines=0;
    uint64_t fullCross=(UINT64_C(0xff)&~UINT64_C(1))|
          (UINT64_C(0x0101010101010101)&~UINT64_C(1));
    assert(place_checked(fullCross,UINT64_C(1),&lines)==0&&lines==2);
    assert(place_checked(fullCross|(UINT64_C(1)<<63),
                         UINT64_C(1),&lines)==(UINT64_C(1)<<63)&&lines==2);
    // The cheap future-tray stress proxy must require two nonoverlapping
    // legal large-piece landings, not merely two independently fitting
    // shapes that collide in the same small patch.
    assert(BLResearchPairCoverage(0)==6);
    assert(BLResearchPairCoverage(UINT64_MAX)==0);
    assert(BLResearchPairCoverage(~UINT64_C(0x70707))==0);
    // Exact legal three-piece synthetic stress trays must fit on an empty
    // 8x8 board and fail on a completely occupied one. The probes are
    // fixed counterfactual scenarios and cannot peek at actual future RNG.
    assert(BLResearchSampledFutureBonus(0)>0);
    assert(BLResearchSampledFutureBonus(UINT64_MAX)<0);
    BLPiece ones[3]={0};
    for(int i=0;i<3;i++){ones[i].count=1;ones[i].x[0]=0;ones[i].y[0]=0;}
    BLComboContext combo={0};
    int weights[4]={120,120,30,60};
    BLSetTrainWeights(weights);
    BLTwoStrategies trained=BLFindTwoStrategies(0,ones,combo,180000);
    validate_plan(0,ones,&trained.smartSurvival,3);
    BLTwoStrategies baseline=BLV17FindTwoStrategies(0,ones,combo,180000);
    validate_plan(0,ones,&baseline.smartSurvival,3);
    trained=BLFindTwoStrategies(UINT64_MAX,ones,combo,180000);
    baseline=BLV17FindTwoStrategies(UINT64_MAX,ones,combo,180000);
    validate_plan(UINT64_MAX,ones,&trained.smartSurvival,0);
    validate_plan(UINT64_MAX,ones,&baseline.smartSurvival,0);
    // Two legitimate steps, followed by a new call with one tray piece
    // already consumed: the solver may not accidentally reuse old pieces.
    BLPiece partial[3]={ones[0],ones[1],ones[2]};
    partial[1].count=0;
    trained=BLFindTwoStrategies(0,partial,combo,180000);
    validate_plan(0,partial,&trained.smartSurvival,2);
    // If a witnessed combo is on its final non-clearing allowance and a
    // complete legal tray route can clear now, the first move must clear.
    BLComboContext urgent={.known=1,.streak=5,.misses=2};
    uint64_t almostRow=UINT64_C(0xfe); // top row missing (0,0)
    trained=BLFindTwoStrategies(almostRow,ones,urgent,180000);
    validate_plan(almostRow,ones,&trained.smartSurvival,3);
    assert(trained.smartSurvival.moves[0].linesCleared>0);
    // Extreme search truncation: the independent emergency check should
    // still find a legal first-move clear and full three-piece completion.
    trained=BLFindTwoStrategies(almostRow,ones,urgent,1);
    validate_plan(almostRow,ones,&trained.smartSurvival,3);
    assert(trained.smartSurvival.moves[0].linesCleared>0);
    // Proactive case: after one miss, a known full-tray clearing route
    // should preserve the observed streak before the final deadline.
    BLComboContext proactive={.known=1,.streak=5,.misses=1};
    trained=BLFindTwoStrategies(almostRow,ones,proactive,180000);
    validate_plan(almostRow,ones,&trained.smartSurvival,3);
    int proClears=0;
    for(int k=0;k<trained.smartSurvival.movesFound;k++)
        proClears+=trained.smartSurvival.moves[k].linesCleared;
    assert(proClears>0);
    // A fresh empty board has no immediately clearable row or column from
    // singletons; an urgency flag must NEVER invent a line clear.
    trained=BLFindTwoStrategies(0,ones,urgent,180000);
    validate_plan(0,ones,&trained.smartSurvival,3);
    assert(trained.smartSurvival.moves[0].linesCleared==0);
    // Challenge the solver's exact masks with large, irregular pieces as
    // well as singleton boards. Do not assume any complete tray exists.
    BLPiece mixed[3]={0};
    mixed[0].count=9;
    for(int y=0;y<3;y++)for(int x=0;x<3;x++) {
        int k=y*3+x;mixed[0].x[k]=x;mixed[0].y[k]=y;
    }
    mixed[1].count=5;
    for(int x=0;x<5;x++){mixed[1].x[x]=x;mixed[1].y[x]=0;}
    mixed[2].count=5;
    const int lx[5]={0,0,0,1,2},ly[5]={0,1,2,2,2};
    for(int k=0;k<5;k++){mixed[2].x[k]=lx[k];mixed[2].y[k]=ly[k];}
    uint32_t rng=0x593def21u;
    for(int trial=0;trial<24;trial++) {
        rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;
        uint64_t board=(uint64_t)rng;
        rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;
        board|=((uint64_t)rng<<32);
        // Both solver variants must return internally legal move-by-move
        // transitions even on cramped, unevenly filled synthetic boards.
        trained=BLFindTwoStrategies(board,mixed,combo,180000);
        validate_plan(board,mixed,&trained.smartSurvival,-1);
        baseline=BLV17FindTwoStrategies(board,mixed,combo,180000);
        validate_plan(board,mixed,&baseline.smartSurvival,-1);
    }
    printf("TENX_RULES_SELFTEST=PASS (row+column clears, legal 3-piece search, used-piece mask)\n");
    return 0;
}
