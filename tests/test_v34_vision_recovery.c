#include "../src/BLVisionRecovery.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    BLVisionRecovery r={0};
    assert(!BLVisionRecoveryFailure(&r,100,10));
    assert(r.failures==1);
    for(unsigned i=1;i<12;i++){
        assert(!BLVisionRecoveryFailure(&r,100,10.5+i*.20));
        assert(r.failures==1);
    }
    for(unsigned i=1;i<=10;i++)
        assert(!BLVisionRecoveryFailure(&r,100+i,11+i*.15));
    assert(r.failures==11);
    assert(!BLVisionRecoveryFailure(&r,111,13)); // below four seconds
    assert(r.failures==12);
    assert(BLVisionRecoveryFailure(&r,112,15));   // 13 fresh frames
    BLVisionRecoveryReset(&r);
    assert(r.failures==0);
    assert(!BLVisionRecoveryFailure(&r,200,20));
    assert(!BLVisionRecoveryFailure(&r,201,20.2));
    BLVisionRecoveryReset(&r);
    assert(!BLVisionRecoveryFailure(&r,202,20.4));
    puts("PASS automatic scan: transient failures retry, duplicates ignored, sustained uncertainty halts, success resets");
    return 0;
}
