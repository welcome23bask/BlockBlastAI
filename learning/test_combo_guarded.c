/* Additional regression for the conservative combo guarding mechanism. */
#define main guarded_learner_main
#include "learn_combo_guarded.c"
#undef main
#include <assert.h>
#include <stdio.h>
int main(void) {
    init_shapes();
    State g={.b=(U64)0x7f,.streak=12,.known=1,.misses=2,
             .p={0,0,0}};
    guardActiveCombo=1;
    Action a=choose(&g,initial);
    assert(a.piece>=0 && a.clears>=1);
    int rest[3]={g.p[0],g.p[1],g.p[2]};rest[a.piece]=-1;
    assert(can_complete_tray(a.after,rest));
    // On an empty board no clear is possible. A fake combo must not make the
    // engine invent one or refuse a legal move.
    g.b=0;
    a=choose(&g,initial);
    assert(a.piece>=0 && a.clears==0);
    assert(!safe_clear_now(0,g.p));
    Result r1=episode(initial,323232u,0);
    Result r2=episode(initial,323232u,0);
    assert(r1.moves==r2.moves && r1.proxy==r2.proxy);
    assert(r1.illegal==0 && r1.moves<=MAXMOVES);
    puts("PASS: single-line combo deadline, complete-tray feasibility, no phantom clear, deterministic and legal");
    return 0;
}
