#ifndef BLOCK_BLAST_SOLVER_H
#define BLOCK_BLAST_SOLVER_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int count;
    int x[25];
    int y[25];
} BLPiece;

typedef struct {
    int pieceIndex;
    int row;
    int col;
    int linesCleared;
    uint64_t before;
    uint64_t after;
} BLMove;

typedef struct {
    int movesFound;
    int totalLines;
    int approximate;
    BLMove moves[3];
} BLSolution;

// A combo is trusted only after we have witnessed at least one legal clear
// transition on the device. We do not infer score or initial streak from pixels.
// A clear on any of the next three moves maintains the observed chain.
typedef struct {
    int streak;          // verified count of consecutive clearing events (capped)
    int misses;          // non-clearing placements since last clear: 0, 1, 2
    int known;           // zero until a real clear is observed
} BLComboContext;
static inline BLComboContext BLComboAdvance(BLComboContext c,int linesCleared) {
    if(linesCleared>0) {
        c.known=1;
        if(c.streak<63)++c.streak;
        c.misses=0;
    } else if(c.known) {
        ++c.misses;
        if(c.misses>=3) {c.misses=0;c.streak=0;}
    }
    return c;
}
BLSolution BLSolveWithCombo(uint64_t board,const BLPiece pieces[3],BLComboContext combo);

// Input: bit r*8+c (0 = top-left). 3 fixed-orientation pieces normalized to (0,0).
// Result: beam-search recommendation, NOT a mathematically guaranteed optimal future move.
BLSolution BLSolve(uint64_t board, const BLPiece pieces[3]);

#ifdef __cplusplus
}
#endif
#endif
