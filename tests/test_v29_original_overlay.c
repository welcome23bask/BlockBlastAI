// Regression for original v1.7 tracking + advanced v2.5 decisions.
// The user reported no yellow hint on an empty board and a false all-filled
// turquoise panel after switching to the new AI. Do not require a first move.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../src/BLFrameSafety.h"
#include "../src/BLGuideSafety.h"
#include "../src/BLDualStrategy.h"
#include "../src/BLV25Strategy.h"

static BLPiece rectangle(int cols,int rows) {
    BLPiece piece={0};
    for(int r=0;r<rows;r++)for(int c=0;c<cols;c++) {
        piece.x[piece.count]=c;
        piece.y[piece.count]=r;
        piece.count++;
    }
    return piece;
}
static BLPiece t_shape(void) {
    BLPiece t={0};t.count=4;
    t.x[0]=1;t.y[0]=0;
    t.x[1]=0;t.y[1]=1;t.x[2]=1;t.y[2]=1;
    t.x[3]=2;t.y[3]=1;return t;
}
static BLPiece l_shape(void) {
    BLPiece l={0};l.count=4;
    l.x[0]=2;l.y[0]=0;
    l.x[1]=0;l.y[1]=1;l.x[2]=1;l.y[2]=1;
    l.x[3]=2;l.y[3]=1;return l;
}
int main(void) {
    BLPiece pieces[3]={rectangle(3,2),t_shape(),l_shape()};
    BLComboContext combo={0};

    // An empty board IS a valid board. Immediately compute a first move,
    // rather than requiring a random player placement to start detection.
    uint64_t empty=UINT64_C(0);
    assert(!BLBoardHasUnclearedLine(empty));
    BLTwoStrategies ai=BLV25FindTwoStrategies(empty,pieces,combo,180000);
    BLTwoStrategies backup=BLFindTwoStrategies(empty,pieces,combo,180000);
    assert(backup.smartSurvival.movesFound>0);
    assert(BLNextGuideSafe(empty,pieces,&backup.smartSurvival));
    if(ai.smartSurvival.movesFound)
        assert(BLNextGuideSafe(empty,pieces,&ai.smartSurvival));
    assert(BLNextGuideSafe(empty,pieces,&ai.smartSurvival) ||
           BLNextGuideSafe(empty,pieces,&backup.smartSurvival));

    // All-filled 64-cell screenshots, or any uncleared full line, are
    // impossible stable states. A score/line animation must not paint a hint.
    assert(BLBoardHasUnclearedLine(UINT64_MAX));
    assert(BLBoardHasUnclearedLine(UINT64_C(0xff)));
    assert(BLBoardHasUnclearedLine(UINT64_C(0x0101010101010101)));
    assert(!BLBoardHasUnclearedLine(UINT64_C(0x7f)));
    assert(!BLNextGuideSafe(UINT64_MAX,pieces,&backup.smartSurvival));

    // Reconstruct the previous on-device pink-board screenshot, with P2
    // already placed. Reject an overlapping square guide and accept a
    // legal square in the upper empty region.
    BLPiece pinkTray[3]={rectangle(3,2),{0},rectangle(2,2)};
    uint64_t pink=UINT64_C(0xfc0cbd39b333b331);
    BLTwoStrategies pinkAI=BLV25FindTwoStrategies(pink,pinkTray,combo,180000);
    BLTwoStrategies pinkOriginal=BLFindTwoStrategies(pink,pinkTray,combo,180000);
    assert(pinkOriginal.smartSurvival.movesFound>0);
    assert(BLNextGuideSafe(pink,pinkTray,&pinkOriginal.smartSurvival));
    if(pinkAI.smartSurvival.movesFound)
        assert(BLNextGuideSafe(pink,pinkTray,&pinkAI.smartSurvival));
    BLSolution illegal={0};illegal.movesFound=1;
    illegal.moves[0].pieceIndex=2;illegal.moves[0].row=0;
    illegal.moves[0].col=4;illegal.moves[0].before=pink;
    illegal.moves[0].after=pink; // overlaps occupied squares
    assert(!BLNextGuideSafe(pink,pinkTray,&illegal));
    printf("PASS v2.9 empty board immediate legal guide, pink board smart strategy, impossible full-board reject and original fallback\n");
    return 0;
}
