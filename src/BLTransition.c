#include "BLTransition.h"
static int same_piece(const BLPiece *a,const BLPiece *b) {
    if(a->count!=b->count || a->count<0 || a->count>25)return 0;
    for(int i=0;i<a->count;++i)
        if(a->x[i]!=b->x[i] || a->y[i]!=b->y[i])return 0;
    return 1;
}
int BLIsExpectedTransition(uint64_t oldBoard, const BLPiece oldTray[3],
                           const BLSolution *solution, uint64_t observedBoard,
                           const BLPiece observedTray[3]) {
    if(!oldTray || !observedTray || !solution || solution->movesFound<1 ||
       solution->movesFound>3)return 0;
    const BLMove *move=&solution->moves[0];
    int used=move->pieceIndex;
    if(used<0 || used>2 || oldTray[used].count<=0 ||
       move->before!=oldBoard || move->after!=observedBoard ||
       observedTray[used].count!=0) return 0;
    for(int p=0;p<3;++p)
        if(p!=used && !same_piece(&oldTray[p],&observedTray[p]))return 0;
    return 1;
}
