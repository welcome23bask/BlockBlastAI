// Long-run combo benchmark comparing v1.3 and v1.4 with REAL streak
// history supplied at every solve. Piece distribution and score are synthetic:
// this is a robustness test, NOT a verified simulation of iOS Block Blast.
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "../src/BlockBlastSolver.h"
BLSolution BLV13SolveWithCombo(uint64_t,const BLPiece[3],BLComboContext);
typedef BLSolution (*Solve)(uint64_t,const BLPiece[3],BLComboContext);
static uint32_t state;
static uint32_t rnd(void){uint32_t x=state;x^=x<<13;x^=x>>17;x^=x<<5;return state=x;}
static const char *patterns[]={
 "#","##","###","####","#####","#/#","#/#/#","#/#/#/#","#/#/#/#/#",
 "##/##","###/###/###","##/##/##","###/###","##./.##",".##/##.",
 "#./##",".#/##","###/#..","###/..#","###/.#.",".#./###","##./.##/..#",
 "###/#.#","#.#/###","#../###","..#/###","##./##./.#.",".#./###/.#."
};
static BLPiece make(const char *s){BLPiece p={0};int x=0,y=0;
 for(;*s;s++){if(*s=='/'){++y;x=0;}else{if(*s=='#'){p.x[p.count]=x;p.y[p.count]=y;++p.count;}++x;}}return p;}
static int difficult=0;
static void tray(BLPiece out[3]){
 static const int big[]={9,10,11,12,13,14,21,22,23,24,25,26,27};
 for(int i=0;i<3;i++){
  int which=rnd()%(sizeof(patterns)/sizeof(patterns[0]));
  if(difficult && rnd()%3<2)which=big[rnd()%(sizeof(big)/sizeof(big[0]))];
  out[i]=make(patterns[which]);
 }}
static int legal(uint64_t board,const BLPiece *p,int r,int c,uint64_t *out) {
 uint64_t mask=0;
 for(int i=0;i<p->count;i++){
  int x=c+p->x[i],y=r+p->y[i];if(x<0||x>=8||y<0||y>=8)return 0;
  uint64_t bit=UINT64_C(1)<<(y*8+x);if((board|mask)&bit)return 0;mask|=bit;
 }if(out)*out=mask;return 1;
}
static uint64_t clear(uint64_t placed,int *n){
 uint64_t erased=0;*n=0;
 for(int r=0;r<8;r++){uint64_t m=UINT64_C(255)<<(r*8);if((placed&m)==m){erased|=m;++*n;}}
 for(int c=0;c<8;c++){uint64_t m=UINT64_C(0x0101010101010101)<<c;if((placed&m)==m){erased|=m;++*n;}}
 return placed&~erased;
}
static int exists_clear(uint64_t board,const BLPiece p[3]){
 for(int k=0;k<3;k++)if(p[k].count)for(int r=0;r<8;r++)for(int c=0;c<8;c++){
  uint64_t mask=0;if(!legal(board,&p[k],r,c,&mask))continue;
  int n=0;(void)clear(board|mask,&n);if(n)return 1;
 }return 0;
}
typedef struct {
 int games;
 long moves,clears,breaks,skippedDeadline;
 double proxyPoints,solveMs,maxMs;
 int *lengths;
 int illegal;
} Stat;
static double ms(struct timespec a,struct timespec b){
 return (double)(b.tv_sec-a.tv_sec)*1000.0+(double)(b.tv_nsec-a.tv_nsec)/1e6;
}
static Stat run(Solve solve,int games,int cap,int seedOffset){
 Stat s={0};s.games=games;s.lengths=calloc(games,sizeof(int));if(!s.lengths)abort();
 for(int g=0;g<games;g++){
  state=(uint32_t)(0x9e3779b9u^((uint32_t)(g+1+seedOffset)*2654435761u));if(!state)state=1;
  uint64_t board=0;BLPiece pieces[3];tray(pieces);BLComboContext combo={0};
  int t=0;
  for(;t<cap;t++){
   struct timespec a,b;clock_gettime(CLOCK_MONOTONIC,&a);
   BLSolution sol=solve(board,pieces,combo);
   clock_gettime(CLOCK_MONOTONIC,&b);
   double dt=ms(a,b);s.solveMs+=dt;if(dt>s.maxMs)s.maxMs=dt;
   if(sol.movesFound<=0)break;
   BLMove move=sol.moves[0];
   if(move.pieceIndex<0||move.pieceIndex>2||!pieces[move.pieceIndex].count||
      move.before!=board){s.illegal++;break;}
   uint64_t mask=0;if(!legal(board,&pieces[move.pieceIndex],move.row,move.col,&mask)){
    s.illegal++;break;
   }
   int lines=0;uint64_t next=clear(board|mask,&lines);
   if(next!=move.after||lines!=move.linesCleared){s.illegal++;break;}
   if(combo.known&&combo.streak>0&&combo.misses==2){
     if(exists_clear(board,pieces)&&!lines)s.skippedDeadline++;
     if(!lines)s.breaks++;
   }
   combo=BLComboAdvance(combo,lines);
   if(lines)s.clears++;
   int multiplier=combo.streak>8?8:combo.streak;
   s.proxyPoints+=pieces[move.pieceIndex].count+(double)(10*lines*lines)*(multiplier>0?multiplier:1);
   s.moves++;
   board=next;pieces[move.pieceIndex].count=0;
   if(!pieces[0].count&&!pieces[1].count&&!pieces[2].count)tray(pieces);
  }
  s.lengths[g]=t;
  if((g+1)%25==0)fprintf(stderr,"%d/%d games evaluated (hard=%d)\n",g+1,games,difficult);
 }
 return s;
}
static int compare(const void*a,const void*b){int x=*(const int*)a,y=*(const int*)b;return x<y?-1:x>y;}
static void report(const char *label,Stat *s) {
 qsort(s->lengths,s->games,sizeof(int),compare);
 printf("%s: games=%d avg_moves=%.2f median=%d p10=%d avg_combo_breaks=%.3f skipped_clear_at_deadline=%ld avg_clearing_moves=%.2f score_proxy=%.1f mean_solve_ms=%.3f max_solve_ms=%.3f invalid=%d\n",
 label,s->games,(double)s->moves/s->games,s->lengths[s->games/2],s->lengths[(s->games-1)/10],
 (double)s->breaks/s->games,s->skippedDeadline,(double)s->clears/s->games,s->proxyPoints/s->games,
 s->solveMs/(s->moves+s->games),s->maxMs,s->illegal);
}
int main(int argc,char**argv) {
 int games=argc>1?atoi(argv[1]):60,cap=argc>2?atoi(argv[2]):1000,
 offset=argc>3?atoi(argv[3]):700000;
 difficult=argc>4?atoi(argv[4]):0;
 if(games<1||games>500||cap<1||cap>5000||offset<0||offset>100000000||(difficult!=0&&difficult!=1))return 2;
 Stat old=run(BLV13SolveWithCombo,games,cap,offset);
 Stat now=run(BLSolveWithCombo,games,cap,offset);
 report("pinkcombo_v13",&old);report("combo_guard_v14",&now);
 printf("avg_move_gain_pct=%.2f avg_score_proxy_gain_pct=%.2f deadline_skips_delta=%ld\n",
 old.moves?100.0*((double)now.moves/old.moves-1.0):0,
 old.proxyPoints?100.0*(now.proxyPoints/old.proxyPoints-1):0,
 now.skippedDeadline-old.skippedDeadline);
 free(old.lengths);free(now.lengths);
 return (old.illegal||now.illegal)?1:0;
}
