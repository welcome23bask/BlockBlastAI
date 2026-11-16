#ifndef BL_VISION_RECOVERY_H
#define BL_VISION_RECOVERY_H
// Distinct failed frames are retries, not permissions to reuse stale plans.
// Hard-stop after multiple failures spanning several seconds so an
// invalid board does not leave Automatic indefinitely armed.
typedef struct {
    unsigned failures;
    unsigned long lastFrame;
    double beganAt;
} BLVisionRecovery;
void BLVisionRecoveryReset(BLVisionRecovery *state);
// Returns 0 for retry, 1 for timeout/stop. Accept only distinct, increasing
// frame identities and elapsed time; a single transient failure cannot stop.
int BLVisionRecoveryFailure(BLVisionRecovery *state,
                            unsigned long frame,double now);
#endif
