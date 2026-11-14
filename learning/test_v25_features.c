// Verify the 30 research feature signals across six stages. Tests do not
// alter the device scanner, iPhone dylib, or production v1.7 solver.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#define BL_FEATURES_TESTING 1
#include "BLV25Strategy.c"
int main(void){
    const uint64_t empty=UINT64_C(0),full=UINT64_MAX;
    const int zeros[6]={0},ones[6]={1,1,1,1,1,1};
    assert(v25_rectfit(empty,4,4)==25);
    assert(v25_rectfit(empty,4,2)==35);
    assert(v25_rectfit(empty,2,4)==35);
    assert(v25_rectfit(empty,6,1)==24);
    assert(v25_rectfit(empty,1,6)==24);
    assert(v25_rectfit(full,3,3)==0);
    assert(v25_group1(empty)>0 && v25_group1(full)==0);
    assert(v25_group2(empty)>0 && v25_group2(full)==0);
    assert(v25_group3(empty)==0 && v25_group3(UINT64_C(0x7f))>0);
    assert(v25_linecount(UINT64_C(0x7f),1,7)==1);
    assert(v25_group4(empty)>0 && v25_group4(full)==0);
    assert(v25_group5(empty)>0);
    assert(v25_group6(empty)>0 && v25_group6(full)<0);
    assert(v25_pair3x3(empty)==1);
    assert(v25_pair3x3(full)==0);
    int regions=0,large=0,tiny=0;
    v25_regions(empty,&regions,&large,&tiny);
    assert(regions==1 && large==64 && tiny==0);
    v25_regions(full,&regions,&large,&tiny);
    assert(regions==0 && large==0 && tiny==0);
    BLSetV25Weights(zeros,6);
    assert(v25_extra_score(empty)==0);
    int prior=0;
    for(int stage=1;stage<=6;stage++){
       BLSetV25Weights(ones,stage);
       int now=v25_extra_score(empty);
       int g=stage==1?v25_group1(empty):
             stage==2?v25_group2(empty):
             stage==3?v25_group3(empty):
             stage==4?v25_group4(empty):
             stage==5?v25_group5(empty):v25_group6(empty);
       assert(now-prior==g);
       prior=now;
    }
    BLSetV25Weights(zeros,6);
    int q0=survival_value(UINT64_C(0x1000001));
    BLSetV25Weights(ones,6);
    int q1=survival_value(UINT64_C(0x1000001));
    assert(q0!=q1);
    puts("PASS v2.5: 30 coded signals in six incremental groups, 8x8 bounds, empty/full topology and feature gates");
    return 0;
}
