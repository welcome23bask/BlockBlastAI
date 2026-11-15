#include "BLFrameSafety.h"
#include "BLGuideSafety.h"
#include <stdint.h>

int BLBoardHasUnclearedLine(uint64_t board) {
    for(int row=0;row<8;++row) {
        uint64_t mask=UINT64_C(255)<<(row*8);
        if((board&mask)==mask)return 1;
    }
    for(int col=0;col<8;++col) {
        uint64_t mask=UINT64_C(0x0101010101010101)<<col;
        if((board&mask)==mask)return 1;
    }
    return 0;
}
int BLNextGuideSafe(uint64_t board,const BLPiece tray[3],const BLSolution *answer) {
    if(!tray||!answer||answer->movesFound<1||answer->movesFound>3 ||
       BLBoardHasUnclearedLine(board))return 0;
    BLSolution next={0};
    next.movesFound=1;
    next.moves[0]=answer->moves[0];
    return BLGuideValidateSolution(board,tray,&next);
}
