#ifndef BL_DUAL_STRATEGY_H
#define BL_DUAL_STRATEGY_H
#include "BlockBlastSolver.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
// Independently designed strategies informed by the public behavior of
// blockblastsolverss.com; NOT their code, service, API, trademark, or algorithm.
typedef enum { BLModeMaxLines=0, BLModeKeepCombo=1, BLModeSmartSurvival=2 } BLStrategyMode;
typedef struct {
    BLSolution maxLines;
    BLSolution keepCombo;
    BLSolution smartSurvival;
    int maxFirstClear;   // 1..3; 0 means none
    int comboFirstClear; // 1..3; 0 means none
    int maxComboPreserved;
    int keepComboPreserved;
    int smartComboPreserved;
    int smartComboRejectedForSafety;
    int smartFutureQuality;
    int exhaustive;
    uint32_t exploredNodes;
} BLTwoStrategies;
// Searches exact current-three moves until budget exhaustion. The caller gets
// the best legal candidates found, with exhaustive=0 if truncated. Nothing
// predicts the actual unseen next tray or the game's true scoring formula.
// SMART SURVIVAL prioritizes an active combo's one-line clear, not a
// large combo bonus. Complete-tray legality and future board survival are
// evaluated before accepting a combo-saving path.
BLTwoStrategies BLFindTwoStrategies(uint64_t board,const BLPiece pieces[3],
                                     BLComboContext combo,uint32_t nodeBudget);
#ifdef __cplusplus
}
#endif
#endif
