#ifndef BL_DUAL_STRATEGY_H
#define BL_DUAL_STRATEGY_H
#include "BlockBlastSolver.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
// Two independently designed strategies informed by the public behavior of
// blockblastsolverss.com; NOT their code, service, API, trademark, or algorithm.
typedef enum { BLModeMaxLines=0, BLModeKeepCombo=1 } BLStrategyMode;
typedef struct {
    BLSolution maxLines;
    BLSolution keepCombo;
    int maxFirstClear;   // 1..3; 0 means none
    int comboFirstClear; // 1..3; 0 means none
    int maxComboPreserved;
    int keepComboPreserved;
    int exhaustive;
    uint32_t exploredNodes;
} BLTwoStrategies;
// Searches exact current-three moves until budget exhaustion. The caller gets
// the best legal candidates found, with exhaustive=0 if truncated. Nothing
// predicts the actual unseen next tray or the game's true scoring formula.
BLTwoStrategies BLFindTwoStrategies(uint64_t board,const BLPiece pieces[3],
                                     BLComboContext combo,uint32_t nodeBudget);
#ifdef __cplusplus
}
#endif
#endif
