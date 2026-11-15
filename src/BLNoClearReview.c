#include "BLNoClearReview.h"
#include <string.h>
void BLNoClearReviewReset(BLNoClearReview *state) {
    if(state)memset(state,0,sizeof(*state));
}
void BLNoClearReviewBegin(BLNoClearReview *state,uint64_t signature,
                          unsigned long currentFrame,double now) {
    if(!state)return;
    BLNoClearReviewReset(state);
    state->signature=signature;
    state->lastFrame=currentFrame;
    state->startedAt=now;
    state->active=1;
}
int BLNoClearReviewObserve(BLNoClearReview *state,uint64_t signature,
                           unsigned long frameCounter,double now) {
    if(!state||!state->active)return -1;
    if(signature!=state->signature || now<state->startedAt) {
        BLNoClearReviewReset(state);return -1;
    }
    if(frameCounter<=state->lastFrame)return 0;
    state->lastFrame=frameCounter;
    if(state->checks<BL_NO_CLEAR_RECHECKS)++state->checks;
    if(state->checks>=BL_NO_CLEAR_RECHECKS && now-state->startedAt>=2.0) {
        state->active=0;state->complete=1;return 1;
    }
    return 0;
}
