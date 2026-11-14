// iPhone v2.5 experimental integration regression: same move-legality contract
// as v1.7, verified combo deadlines, independent selectable baseline.
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../src/BLDualStrategy.h"
#include "../src/BLV25Strategy.h"
static BLPiece bar(int n) {
    BLPiece p={0};p.count=n;
    for(int i=0;i<n;i++)p.x[i]=i;
    return p;
}
static uint64_t clear_board(uint64_t board,int *lines) {
    uint64_t removed=0;*lines=0;
    for(int r=0;r<8;r++){
        uint64_t mask=UINT64_C(255)<<(8*r);
        if((board&mask)==mask){removed|=mask;++*lines;}
    }
    for(int c=0;c<8;c++){
        uint64_t mask=UINT64_C(0x0101010101010101)<<c;
        if((board&mask)==mask){removed|=mask;++*lines;}
    }
    return board&~removed;
}
static void verify(uint64_t board,const BLPiece pieces[3],BLSolution s){
    int used=0,cleared=0;
    assert(s.movesFound>=0&&s.movesFound<=3);
    for(int t=0;t<s.movesFound;t++){
        BLMove m=s.moves[t];
        assert(m.pieceIndex>=0&&m.pieceIndex<3);
        assert((used&(1<<m.pieceIndex))==0);
        const BLPiece *p=&pieces[m.pieceIndex];
        assert(p->count>0);
        assert(m.before==board && m.row>=0 && m.col>=0);
        uint64_t placed=0;
        for(int k=0;k<p->count;k++){
            int y=m.row+p->y[k],x=m.col+p->x[k];
            assert(x>=0&&x<8&&y>=0&&y<8);
            uint64_t bit=UINT64_C(1)<<(8*y+x);
            assert((placed&bit)==0 && (board&bit)==0);
            placed|=bit;
        }
        int lines=0;
        board=clear_board(board|placed,&lines);
        assert(lines==m.linesCleared && m.after==board);
        cleared+=lines;
        used|=1<<m.pieceIndex;
    }
    assert(s.totalLines==cleared);
}
int main(void){
    const BLPiece bars[3]={bar(5),bar(4),bar(5)};
    const uint64_t boards[]={0,UINT64_C(0x7f),UINT64_C(0x101020204),
                             UINT64_C(0x808000000001e100)};
    for(int i=0;i<4;i++){
        BLComboContext combo={0};
        BLTwoStrategies base=BLFindTwoStrategies(boards[i],bars,combo,180000);
        BLTwoStrategies test=BLV25FindTwoStrategies(boards[i],bars,combo,180000);
        verify(boards[i],bars,base.smartSurvival);
        verify(boards[i],bars,test.smartSurvival);
    }
    // One remaining single block completes a row AND preserves last chance.
    BLPiece few[3]={bar(1),{0},{0}};
    BLComboContext emergency={.known=1,.streak=4,.misses=2};
    BLTwoStrategies save=BLV25FindTwoStrategies(UINT64_C(0x7f),
                                      few,emergency,180000);
    assert(save.smartSurvival.movesFound==1);
    assert(save.smartSurvival.moves[0].linesCleared==1);
    verify(UINT64_C(0x7f),few,save.smartSurvival);
    printf("PASS: v2.5 experimental and v1.7 fallback produce legal full sequences; combo deadline saves one line\\n");
    return 0;
}
