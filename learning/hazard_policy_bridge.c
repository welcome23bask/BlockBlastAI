/* Research-only. Share the label simulator's exact observable feature code.
 * No calls to trace_label, trace_play, RNG or next-tray generation at inference.
 * Compiled only with an independently calibrated generated model header.
 */
#define BL_HAZARD_TRACE_LIBRARY
#include "hazard_trace_sim.c"
#include "hazard_policy_model.generated.h"

double BLResearchHazardProbability(uint64_t board, int streak) {
    static int initialized = 0;
    if (!initialized) { init_shapes(); initialized = 1; }
    int largest = 0, pockets = 0;
    trace_regions(board, &largest, &pockets);
    if (streak < 0) streak = 0;
    if (streak > 63) streak = 63;
    const int f[9] = {pop(board), countfit(board,0), countfit(board,1),
        countfit(board,2), trace_hard_zero(board), largest, pockets,
        streak, trace_future_clear(board)};
    double z = bl_hazard_weights[0];
    for (int i=0;i<9;++i) z += bl_hazard_weights[i+1]*f[i]/bl_hazard_scales[i];
    if (z >= 0) return 1.0/(1.0+exp(-z));
    const double t=exp(z);
    return t/(1.0+t);
}
int BLResearchHazardPenalty(uint64_t board, int streak) {
    /* Bounded late rerank; never modify legal placement or urgent rescue. */
    return (int)(1800.0*BLResearchHazardProbability(board,streak)+0.5);
}
