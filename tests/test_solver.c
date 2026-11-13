#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../src/BlockBlastSolver.h"
int main(void) {
    BLPiece pieces[3] = {0};
    for(int i=0;i<3;++i) { pieces[i].count=1; pieces[i].x[0]=0; pieces[i].y[0]=0; }
    BLSolution a=BLSolve(0,pieces);
    assert(a.movesFound==3);
    assert(a.totalLines==0);
    uint64_t board=UINT64_C(0x7f);
    BLSolution b=BLSolve(board,pieces);
    assert(b.movesFound==3);
    assert(b.totalLines>=1);
    BLSolution c=BLSolve(UINT64_MAX,pieces);
    assert(c.movesFound==0);
    // As pieces are placed, the real tray has only two or one pieces left.
    pieces[1].count=0;
    BLSolution d=BLSolve(0,pieces);
    assert(d.movesFound==2);
    assert(d.moves[0].pieceIndex!=1);
    assert(d.moves[1].pieceIndex!=1);
    pieces[2].count=0;
    BLSolution e=BLSolve(0,pieces);
    assert(e.movesFound==1 && e.moves[0].pieceIndex==0);
    pieces[0].count=0;
    BLSolution f=BLSolve(0,pieces);
    assert(f.movesFound==0);
    puts("solver tests passed");
    return 0;
}
