#define BL_TENX_EMBEDDED
#define BL_TENX_NODE_BUDGET 18000
#include "train_tenx.c"
#include <assert.h>
#include <stdio.h>
int main(void){
    init_shapes();
    // Bottom of a full row: one legal single-cell clear resets miss=2.
    State s={.b=UINT64_C(0x7f),.known=1,.streak=15,.misses=2,
             .p={0,0,0}};
    BLMove stale={.pieceIndex=0,.row=2,.col=2,.linesCleared=0,
                  .before=s.b,.after=s.b|(UINT64_C(1)<<18)};
    assert(safe_clear_now(s.b,s.p));
    assert(v23_deadline_rescue(&s,&stale)==1);
    assert(stale.pieceIndex>=0&&stale.pieceIndex<3);
    assert(stale.linesCleared>=1&&stale.before==s.b);
    U64 mask=shapes[s.p[stale.pieceIndex]].placed[stale.row*8+stale.col];
    assert(mask&&!(mask&s.b));
    int cleared=0;U64 after=place(s.b,mask,&cleared);
    assert(after==stale.after && cleared==stale.linesCleared);
    int rest[3]={s.p[0],s.p[1],s.p[2]};
    rest[stale.pieceIndex]=-1;
    assert(can_complete_tray(after,rest));
    // Never invent a clear from a completely empty board.
    s.b=0;
    stale.linesCleared=0;
    assert(!v23_deadline_rescue(&s,&stale));
    // No demand for a clear before the deadline.
    s.b=UINT64_C(0x7f);s.misses=1;
    assert(!v23_deadline_rescue(&s,&stale));
    puts("PASS: exact safe one-line combo rescue, tray continuation and no invented clears");
    return 0;
}
