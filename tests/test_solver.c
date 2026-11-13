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
    puts("solver tests passed");
    return 0;
}
