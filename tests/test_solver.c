#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../src/BlockBlastSolver.h"

static int is_legal(uint64_t board,const BLPiece*p,int r,int c) {
    if(r<0||c<0)return 0;
    for(int i=0;i<p->count;++i) {
        int x=c+p->x[i],y=r+p->y[i];
        if(x>=8||y>=8||(board&(UINT64_C(1)<<(y*8+x))))return 0;
    }
    return 1;
}
int main(void) {
    BLPiece pieces[3]={0};
    for(int i=0;i<3;++i) {
        pieces[i].count=1;pieces[i].x[0]=0;pieces[i].y[0]=0;
    }
    BLSolution a=BLSolve(0,pieces);
    assert(a.movesFound==3&&a.totalLines==0);
    uint64_t board=UINT64_C(0x7f);
    BLSolution b=BLSolve(board,pieces);
    assert(b.movesFound==3&&b.totalLines>=1);
    BLSolution c=BLSolve(UINT64_MAX,pieces);
    assert(c.movesFound==0);
    pieces[1].count=0;
    BLSolution d=BLSolve(0,pieces);
    assert(d.movesFound==2);
    assert(d.moves[0].pieceIndex!=1&&d.moves[1].pieceIndex!=1);
    pieces[2].count=0;
    BLSolution e=BLSolve(0,pieces);
    assert(e.movesFound==1&&e.moves[0].pieceIndex==0);
    pieces[0].count=0;
    BLSolution f=BLSolve(0,pieces);
    assert(f.movesFound==0);
    pieces[0].count=3;
    pieces[0].x[0]=0;pieces[0].x[1]=1;pieces[0].x[2]=2;
    BLSolution s=BLSolve(0,pieces);
    assert(s.movesFound==1);
    assert(is_legal(0,&pieces[0],s.moves[0].row,s.moves[0].col));
    pieces[0].count=2;pieces[0].x[1]=0;pieces[0].y[1]=0;
    assert(BLSolve(0,pieces).movesFound==0);
    puts("solver tests passed (path legality, partial trays, invalid shapes)");
    return 0;
}
