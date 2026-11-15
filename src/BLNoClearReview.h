#ifndef BL_NO_CLEAR_REVIEW_H
#define BL_NO_CLEAR_REVIEW_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
// Fifty new, separately captured frames must agree on the ENTIRE validated
// board/tray signature after a no-clear search. Also requires >=2 seconds.
// Pure C so the rule can be unit-tested without UIKit or ReplayKit.
#define BL_NO_CLEAR_RECHECKS 50U
typedef struct {
    uint64_t signature;
    unsigned long lastFrame;
    double startedAt;
    unsigned checks;
    unsigned active;
    unsigned complete;
} BLNoClearReview;

void BLNoClearReviewReset(BLNoClearReview *review);
void BLNoClearReviewBegin(BLNoClearReview *review, uint64_t signature,
                          unsigned long currentFrame, double now);
// Returns -1 on signature change/cancel, 0 while waiting, +1 on confirmation.
// The same frame counter NEVER contributes two confirmations.
int BLNoClearReviewObserve(BLNoClearReview *review, uint64_t signature,
                           unsigned long frameCounter, double now);
#ifdef __cplusplus
}
#endif
#endif
