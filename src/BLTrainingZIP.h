#ifndef BL_TRAINING_ZIP_H
#define BL_TRAINING_ZIP_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
// Minimal standard ZIP "store" archive, no private frameworks or networking.
// Paths are relative ASCII names beneath folder; ../ and absolute paths
// are rejected. Writes readable ZIPs on iOS 15+ and desktop platforms.
int BLTrainingWriteZIP(const char *archive, const char *folder,
                       const char *const *relativePaths, size_t count);
#ifdef __cplusplus
}
#endif
#endif
