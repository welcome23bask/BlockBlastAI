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

// Input: bit r*8+c (0 = top-left). 3 fixed-orientation pieces normalized to (0,0).
// Result: beam-search recommendation, NOT a mathematically guaranteed optimal future move.
BLSolution BLSolve(uint64_t board, const BLPiece pieces[3]);

#ifdef __cplusplus
}
#endif
#endif
