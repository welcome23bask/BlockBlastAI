// Exercises the exact offline three-piece evaluator compiled for the research lab.
// Never writes to the iPhone game or guesses hidden future trays.
#define BL_RESEARCH_NO_MAIN
#include "whole_tray_value_safe.c"
#include <assert.h>
#include <stdio.h>
int main(void){
  init_shapes();
  State board={0};board.p[0]=4;board.p[1]=3;board.p[2]=10;
  BLPiece three[3]={0};convert_pieces(board.p,three);
  assert(three[0].count==5&&three[1].count==4&&three[2].count==9);
  double f[V];after_features(0,1,2,f);
  for(int i=0;i<V;i++)assert(isfinite(f[i]));
  assert(f[0]==1.0&&f[9]==1.0&&f[10]==1.0);
  BLComboContext c={.known=1,.streak=10,.misses=2};
  BLPiece tiny[3]={0};
  int ids[3]={0,0,0};convert_pieces(ids,tiny);
  BLTwoStrategies q=BLFindTwoStrategies(UINT64_C(0x7f),tiny,c,180000);
  assert(q.smartSurvival.movesFound==3);
  assert(q.smartSurvival.moves[0].linesCleared>0);
  assert(end_combo(1,10,2,&q.smartSurvival,NULL)>0);
  // Both plain and hard-piece streams are deterministic from a seed.
  for(int scenario=0;scenario<3;scenario++){
    Result a=play_or_collect(453111u,scenario,0,0,NULL);
    Result b=play_or_collect(453111u,scenario,0,0,NULL);
    assert(a.illegal==0&&a.moves==b.moves&&a.proxy==b.proxy);
    assert(a.moves<=MAXMOVES);
  }
  puts("PASS whole-tray evaluation, combo deadline, three generators, deterministic full-games and legal moves");
  return 0;
}