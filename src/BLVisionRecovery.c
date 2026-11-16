#include "BLVisionRecovery.h"
void BLVisionRecoveryReset(BLVisionRecovery *state) {
    if(!state)return;
    state->failures=0;
    state->lastFrame=0;
    state->beganAt=0;
}
int BLVisionRecoveryFailure(BLVisionRecovery *state,
                            unsigned long frame,double now) {
    if(!state || !frame || now<=0)return 1;
    if(state->failures && frame<=state->lastFrame)return 0;
    if(!state->failures)state->beganAt=now;
    state->lastFrame=frame;
    if(state->failures<1000000U)++state->failures;
    // At least twelve NEW failing frames AND at least four seconds.
    // We never execute even one drag while a failure is pending.
    return state->failures>=12U && now-state->beganAt>=4.0;
}
