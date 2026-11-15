// v2.8 regression: real-user pink-board occupancy transcribed from screenshot.
// Only unit-test fixtures (not user's personal screenshot) enter the repository.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../src/BLGuideSafety.h"
#include "../src/BLDualStrategy.h"
#include "../src/BLV25Strategy.h"

static BLPiece rectpiece(int cols,int rows) {
    BLPiece p={0};
    for(int y=0;y<rows;++y)for(int x=0;x<cols;++x){
        p.x[p.count]=x;p.y[p.count]=y;p.count++;
    }
    return p;
}
static uint64_t screenshot_board(void) {
    const int positions[][2]={
      {0,0},{0,4},{0,5},
      {1,0},{1,1},{1,4},{1,5},{1,7},
      {2,0},{2,1},{2,4},{2,5},
      {3,0},{3,1},{3,4},{3,5},{3,7},
      {4,0},{4,3},{4,4},{4,5},
      {5,0},{5,2},{5,3},{5,4},{5,5},{5,7},
      {6,2},{6,3},
      {7,2},{7,3},{7,4},{7,5},{7,6},{7,7}
    };
    uint64_t board=0;
    for(unsigned i=0;i<sizeof(positions)/sizeof(positions[0]);++i)
        board|=UINT64_C(1)<<(positions[i][0]*8+positions[i][1]);
    return board;
}
static BLSolution manual(uint64_t board,int idx,int row,int col,
                         const BLPiece p[3]){
    BLSolution s={0};s.movesFound=1;
    BLMove *m=&s.moves[0];
    m->pieceIndex=idx;m->row=row;m->col=col;m->before=board;
    uint64_t bits=0;
    for(int k=0;k<p[idx].count;++k){
        int yy=row+p[idx].y[k],xx=col+p[idx].x[k];
        if(yy>=0&&yy<8&&xx>=0&&xx<8)
            bits|=UINT64_C(1)<<(yy*8+xx);
    }
    m->after=board|bits;
    return s;
}
int main(void) {
    uint64_t board=screenshot_board();
    BLPiece pieces[3]={0};
    pieces[0]=rectpiece(3,2); // visible left 6-square rectangle
    pieces[2]=rectpiece(2,2); // visible right 4-square square; P2 used
    BLSolution good=manual(board,2,0,2,pieces); // genuinely free 2x2
    assert(BLGuideValidateSolution(board,pieces,&good));
    BLSolution overlap=manual(board,2,0,4,pieces);
    assert(!BLGuideValidateSolution(board,pieces,&overlap));
    BLSolution outside=manual(board,2,7,1,pieces);
    assert(!BLGuideValidateSolution(board,pieces,&outside));
    BLSolution stale=good;stale.moves[0].after^=UINT64_C(1);
    assert(!BLGuideValidateSolution(board,pieces,&stale));
    BLSolution fakeClear=good;fakeClear.moves[0].linesCleared=1;
    assert(!BLGuideValidateSolution(board,pieces,&fakeClear));
    BLPiece invalid[3];memcpy(invalid,pieces,sizeof(pieces));
    invalid[2].x[3]=4;invalid[2].y[3]=4;
    assert(!BLGuideValidateSolution(board,invalid,&good));

    // Keep newer intelligent strategy: validate both the familiar smart
    // original and the experimental new smart strategy on the exact fixture.
    BLComboContext combo={0};
    BLTwoStrategies old=BLFindTwoStrategies(board,pieces,combo,180000);
    BLTwoStrategies smart=BLV25FindTwoStrategies(board,pieces,combo,180000);
    if(old.smartSurvival.movesFound)
        assert(BLGuideValidateSolution(board,pieces,&old.smartSurvival));
    if(smart.smartSurvival.movesFound)
        assert(BLGuideValidateSolution(board,pieces,&smart.smartSurvival));
    puts("PASS v2.8: legal pink board P3 2x2, illegal overlap/offscreen/stale/shape rejected, both AI solvers validated");
    return 0;
}
