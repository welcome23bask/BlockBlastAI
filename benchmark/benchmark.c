#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../src/BlockBlastSolver.h"

BLSolution BLOldSolve(uint64_t board,const BLPiece pieces[3]);
typedef BLSolution (*Solver)(uint64_t,const BLPiece[3]);
static uint32_t rng;
static uint32_t next_random(void){uint32_t x=rng;x^=x<<13;x^=x>>17;x^=x<<5;return rng=x;}
static uint32_t next_bounded(uint32_t limit){return next_random()%limit;}
static const char *shapePatterns[]={
 "#","##","###","####","#####","#/#","#/#/#","#/#/#/#","#/#/#/#/#",
 "##/##","###/###/###","##/##/##","###/###","##./.##",".##/##.",
 "#./##",".#/##","###/#..","###/..#","###/.#.",".#./###","##./.##/..#",
 "###/#.#","#.#/###","#../###","..#/###","##./##./.#.",".#./###/.#."
};
#define SHAPES ((int)(sizeof(shapePatterns)/sizeof(shapePatterns[0])))
static BLPiece shape_from(int index){
 const char *s=shapePatterns[index]; BLPiece piece={0}; int x=0,y=0;
 for(;*s;++s){if(*s=='/'){y++;x=0;continue;}if(*s=='#'){
   piece.x[piece.count]=x;piece.y[piece.count]=y;piece.count++;
 }x++;} return piece;
}
static void next_tray(BLPiece pieces[3]) {
 for(int i=0;i<3;++i)pieces[i]=shape_from(next_bounded(SHAPES));
}
static int legal(uint64_t board,BLPiece *p,int row,int col){
 if(!p->count)return 0;
 for(int k=0;k<p->count;++k){int x=col+p->x[k],y=row+p->y[k];
  if(x<0||y<0||x>=8||y>=8||(board&(UINT64_C(1)<<(y*8+x))))return 0;
 }return 1;
}
static double elapsed_ms(struct timespec a,struct timespec b){return (double)(b.tv_sec-a.tv_sec)*1000.0+(b.tv_nsec-a.tv_nsec)/1e6;}
typedef struct {uint64_t steps,lines,solves,illegal,completed;double totalMs,maxMs;double *durations;size_t nDurations,capacity;} Stats;
static void store_time(Stats *s,double d){
 if(s->nDurations==s->capacity){s->capacity=s->capacity?2*s->capacity:8192;s->durations=realloc(s->durations,s->capacity*sizeof(double));if(!s->durations){perror("realloc");exit(1);}}
 s->durations[s->nDurations++]=d;s->totalMs+=d;s->maxMs=(s->maxMs>d?s->maxMs:d);
}
static int cmp_double(const void *ap,const void *bp){double a=*(const double*)ap,b=*(const double*)bp;return a<b?-1:a>b?1:0;}
static Stats evaluate(Solver solver,int games,int seedOffset,int maxMoves){
 Stats stats={0};
 for(int g=0;g<games;++g){
   rng=(uint32_t)(0x9e3779b9u^((uint32_t)(g+seedOffset+1)*2654435761u));
   if(!rng)rng=1337;
   uint64_t board=0; BLPiece pieces[3];next_tray(pieces);
   int t=0;
   for(;t<maxMoves;++t){
     struct timespec a,b;clock_gettime(CLOCK_MONOTONIC,&a);
     BLSolution sol=solver(board,pieces);
     clock_gettime(CLOCK_MONOTONIC,&b);
     store_time(&stats,elapsed_ms(a,b));stats.solves++;
     if(!sol.movesFound)break;
     BLMove m=sol.moves[0];
     if(m.pieceIndex<0||m.pieceIndex>2||m.before!=board||!legal(board,&pieces[m.pieceIndex],m.row,m.col)){
       fprintf(stderr,"INVALID SOLVER MOVE game=%d move=%d piece=%d\n",g,t,m.pieceIndex);stats.illegal++;break;
     }
     stats.steps++;stats.lines+=(uint64_t)m.linesCleared;
     board=m.after;pieces[m.pieceIndex].count=0;
     if(!pieces[0].count&&!pieces[1].count&&!pieces[2].count)next_tray(pieces);
   }
   if(t==maxMoves)stats.completed++;
   if((g+1)%500==0) fprintf(stderr,"%d/%d games\n",g+1,games);
 }
 return stats;
}
static void report(char *name,Stats *s,int games){
 qsort(s->durations,s->nDurations,sizeof(double),cmp_double);
 size_t p95=(size_t)((s->nDurations-1)*.95),p99=(size_t)((s->nDurations-1)*.99);
 printf("%s: games=%d avg_moves=%.4f avg_lines=%.4f avg_solve_ms=%.4f p95_ms=%.3f p99_ms=%.3f max_ms=%.3f illegal=%llu capped=%llu\n",name,games,
  (double)s->steps/games,(double)s->lines/games,s->totalMs/s->solves,
  s->durations[p95],s->durations[p99],s->maxMs,(unsigned long long)s->illegal,(unsigned long long)s->completed);
}
int main(int argc,char **argv){
 int games=argc>1?atoi(argv[1]):2000;
 int moves=argc>2?atoi(argv[2]):120;
 int offset=argc>3?atoi(argv[3]):0;
 if(games<1||games>100000||moves<1||moves>10000||offset<0||offset>100000000)return 2;
 fprintf(stderr,"Simulating baseline %d deterministic games (max %d moves/game, seed offset %d)\n",games,moves,offset);
 Stats base=evaluate(BLOldSolve,games,offset,moves);
 fprintf(stderr,"Simulating candidate %d matching games\n",games);
 Stats cand=evaluate(BLSolve,games,offset,moves);
 report("baseline",&base,games);report("candidate",&cand,games);
 double moveRatio=base.steps?100*((double)cand.steps/base.steps-1):0;
 double lineRatio=base.lines?100*((double)cand.lines/base.lines-1):0;
 printf("gain_moves_pct=%.2f gain_lines_pct=%.2f\n",moveRatio,lineRatio);
 int passed=(cand.illegal==0 && base.illegal==0 && cand.steps>base.steps && cand.lines>=base.lines);
 printf("quality_gate=%s\n",passed?"PASS":"FAIL");
 free(base.durations);free(cand.durations);
 return passed?0:1;
}
