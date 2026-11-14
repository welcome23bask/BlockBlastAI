#ifndef BL_V25_STRATEGY_H
#define BL_V25_STRATEGY_H
#include "BLDualStrategy.h"
// Experimental 2026-11-14 research model. No guarantees for real gameplay.
// Matches v2.5 trained coefficients: prior=[120,120,0,-20],
// 30-feature groups=[0,0,0,1,0,0]. Separate from the untouched v1.7 solver.
BLTwoStrategies BLV25FindTwoStrategies(uint64_t board,const BLPiece pieces[3],
                                      BLComboContext combo,uint32_t nodeBudget);
#endif
