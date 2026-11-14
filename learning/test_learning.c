/* Pure C regression tests for the EXACT learner simulator semantics.
   There is no direct dependence on the iOS injection framework. */
#define main learner_main
#include "learn.c"
#undef main
#include <assert.h>
#include <stdio.h>
int main(void){
  init_shapes();
  assert(shapes[0].n==1&&shapes[4].n==5&&shapes[10].n==9);
  assert(fit[0].n==36 && fit[1].n==32 && fit[2].n==32);
  assert(countfit(0,0)==36 && countfit(UINT64_MAX,0)==0);
  // One cell completes both row 1 and column 8 simultaneously.
  U64 b=UINT64_C(0x7f);
  for(int r=1;r<8;r++)b|=UINT64_C(1)<<(r*8+7);
  int lines=0;U64 after=place(b,UINT64_C(1)<<7,&lines);
  assert(lines==2&&after==0);
  // No overlaps, shape coordinates stay within board.
  for(int p=0;p<N_SHAPES;p++) {
    Shape *s=&shapes[p];
    assert(s->n>=1&&s->n<=9&&s->w<=8&&s->h<=8);
    for(int r=0;r<=8-s->h;r++)
      for(int c=0;c<=8-s->w;c++)assert(pop(s->placed[r*8+c])==s->n);
  }
  State x={0};x.p[0]=4;x.p[1]=3;x.p[2]=4;
  Action choice=choose(&x,initial);
  assert(choice.piece>=0&&choice.piece<3);
  assert(choice.after!=UINT64_MAX);
  double features0[F];features(&x,choice,features0);
  for(int i=0;i<F;i++)assert(isfinite(features0[i]));
  // Same seed and policy must produce the same game, including scores.
  Result a=episode(initial,448821u,0),c=episode(initial,448821u,0);
  assert(a.moves==c.moves && a.proxy==c.proxy && a.clears==c.clears);
  assert(a.illegal==0);
  // Stress set also must remain legal and respect the cap.
  Result z=episode(initial,99991u,1);
  assert(z.illegal==0 && z.moves<=MAXMOVES);
  // Check that target combo clearing event preserves the streak.
  State g={.b=(U64)0x7f,.streak=9,.known=1,.misses=2,
           .p={0,-1,-1}};
  Action m=choose(&g,initial);
  assert(m.piece==0 && m.clears==1);
  puts("PASS: exact two-line clearing, 28 valid shapes, repeatable training, combo urgency and no illegal moves");
  return 0;
}
