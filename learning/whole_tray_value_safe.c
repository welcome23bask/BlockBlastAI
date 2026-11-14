/* Offline three-piece afterstate-value learner research harness.
   Uses the exact v1.7 whole-tray engine for legal candidate sequences.
   Collects game-trajectory returns, never exposes simulated future pieces
   to the decision policy. No iPhone runtime modifications. */
#define main original_training_main
#include "learn_combo_guarded.c"
#undef main
#include "../src/BLDualStrategy.h"
#include <assert.h>

#define V 12
#define TRAJECTORY 210
static const char *names[V]={"empty","fit3x3","fit5h","fit5v","rectangles","shape_coverage","large_coverage","isolated","near_complete_lines","combo_active","combo_urgency","combo_streak"};
static const int big_shapes[]={9,10,11,12,13,14,21,22,23,24,25,26,27};
static double weights[V]={0};
static double bias=0;
static int changes=0;
static int rejectedDeadline=0, rejectedFutureSpace=0;
static void convert_pieces(const int ps[3],BLPiece out[3]){
 memset(out,0,3*sizeof(*out));
 for(int i=0;i<3;i++)if(ps[i]>=0){Shape *s=&shapes[ps[i]];out[i].count=s->n;
  for(int j=0;j<s->n;j++){out[i].x[j]=s->dx[j];out[i].y[j]=s->dy[j];}}
}
static int end_combo(int known,int streak,int misses,const BLSolution *s,int *end_misses){
 for(int k=0;k<s->movesFound;k++){
  if(s->moves[k].linesCleared>0){known=1;streak=(streak<63)?streak+1:63;misses=0;}
  else if(known){if(++misses>=3){streak=0;misses=0;}}
 }
 if(end_misses)*end_misses=misses;
 return streak;
}
static void after_features(U64 b,int streak,int misses,double f[V]){
 f[0]=(64-pop(b))/64.0;
 f[1]=fmin(20,countfit(b,0))/20.0;
 f[2]=fmin(20,countfit(b,1))/20.0;
 f[3]=fmin(20,countfit(b,2))/20.0;
 f[4]=fmin(35,countfit(b,3)+countfit(b,4))/35.0;
 int kinds=0,large=0;
 for(int i=0;i<N_SHAPES;i++)kinds+=options_on(b,i)>0;
 for(size_t i=0;i<sizeof(big_shapes)/sizeof(big_shapes[0]);i++)large+=options_on(b,big_shapes[i])>0;
 f[5]=kinds/(double)N_SHAPES;
 f[6]=large/(double)(sizeof(big_shapes)/sizeof(big_shapes[0]));
 f[7]=fmin(12,isolated(b))/12.0;
 int near=0;
 for(int i=0;i<8;i++){near+=pop(b&rowMask[i])>=6;near+=pop(b&colMask[i])>=6;}
 f[8]=near/16.0;
 f[9]=(streak>0)?1.:0.;
 f[10]=(streak>0)?misses/2.0:0.;
 f[11]=fmin(streak,20)/20.0;
}
static double predict(U64 board,int streak,int misses){
 double f[V];after_features(board,streak,misses,f);
 double z=bias;for(int i=0;i<V;i++)z+=f[i]*weights[i];
 return z;
}
static int model_load(const char *file){
 FILE *fp=fopen(file,"r");if(!fp){perror(file);return 0;}
 char text[8192]={0};size_t n=fread(text,1,sizeof(text)-1,fp);fclose(fp);text[n]=0;
 char *p=strstr(text,"\"weights\"");
 if(!p||!(p=strchr(p,'[')))return 0;++p;
 for(int i=0;i<V;i++){
  char *end=NULL;weights[i]=strtod(p,&end);
  if(end==p||!isfinite(weights[i]))return 0;
  p=end;while(*p==' '||*p==',')p++;
 }
 p=strstr(text,"\"bias\"");if(!p||!(p=strchr(p,':')))return 0;
 bias=strtod(p+1,NULL);return isfinite(bias);
}
static const BLSolution *choose_plan(const State *g,const BLTwoStrategies *ans,int useModel){
 const BLSolution *baseline=&ans->smartSurvival;
 if(!useModel||baseline->movesFound<1)return baseline;
 int remaining=(g->p[0]>=0)+(g->p[1]>=0)+(g->p[2]>=0);
 if(baseline->movesFound!=remaining)return baseline;
 const BLSolution *choices[]={&ans->maxLines,&ans->keepCombo};
 const BLSolution *chosen=baseline;
 int oldMiss=0;
 int oldStreak=end_combo(g->known,g->streak,g->misses,baseline,&oldMiss);
 double top=predict(baseline->moves[remaining-1].after,oldStreak,oldMiss);
 for(int i=0;i<2;i++){
  const BLSolution *s=choices[i];if(s->movesFound!=remaining)continue;
  int miss=0,streak=end_combo(g->known,g->streak,g->misses,s,&miss);
  if(oldStreak>0&&streak==0)continue;
  // A safe single-line clear is more important than a larger future clear.
  // Never break a due-next-move combo if the original plan clears now.
  if(g->known && g->streak>0 && g->misses==2 &&
     baseline->moves[0].linesCleared>0 && s->moves[0].linesCleared==0){
     ++rejectedDeadline;continue;
  }
  U64 next=s->moves[remaining-1].after,base=baseline->moves[remaining-1].after;
  if(pop(next)>pop(base)+1)continue;
  int safe=1;
  for(int j=0;j<3;j++)if(countfit(base,j)>0&&countfit(next,j)==0)safe=0;
  if(!safe)continue;
  int coverageA=0,coverageB=0,largeA=0,largeB=0;
  for(int j=0;j<N_SHAPES;j++) {
    coverageA+=options_on(base,j)>0;
    coverageB+=options_on(next,j)>0;
  }
  for(size_t j=0;j<sizeof(big_shapes)/sizeof(big_shapes[0]);j++) {
    largeA+=options_on(base,big_shapes[j])>0;
    largeB+=options_on(next,big_shapes[j])>0;
  }
  if(coverageB<coverageA || largeB<largeA){++rejectedFutureSpace;continue;}
  double value=predict(next,streak,miss);
  // Only override the robust local engine with a material learned advantage.
  if(value>top+15.0){chosen=s;top=value;}
 }
 changes+=chosen!=baseline;return chosen;
}
typedef struct {U64 board;int streak,misses;int step;} Snap;
// A deliberately shifted holdout generator: 75% long/hard pieces, not
// seen in the two-distribution value learner training sessions.
static void draw_tray(State *g,int scenario) {
    new_tray(g,scenario>0);
    if(scenario==2){
        static const int hard[]={4,8,10,11,21,22,23,24,25,26,27};
        for(int p=0;p<3;p++) if(random32()%4!=0)
            g->p[p]=hard[random32()%(sizeof(hard)/sizeof(hard[0]))];
    }
}
static Result play_or_collect(uint32_t seed,int stress,int useModel,int game_id,FILE *out){
 seed_rng(seed);State g={0};draw_tray(&g,stress);Result result={0};
 Snap samples[TRAJECTORY];int nSamples=0;
 for(int turn=0;turn<MAXMOVES;turn++){
  BLPiece pp[3];convert_pieces(g.p,pp);
  BLComboContext combo={.streak=g.streak,.misses=g.misses,.known=g.known};
  BLTwoStrategies ans=BLFindTwoStrategies(g.b,pp,combo,180000);
  const BLSolution *solution=choose_plan(&g,&ans,useModel);
  if(!solution->movesFound)break;
  BLMove move=solution->moves[0];
  if(move.pieceIndex<0||move.pieceIndex>2||g.p[move.pieceIndex]<0||move.before!=g.b||
    move.row<0||move.row>=8||move.col<0||move.col>=8){result.illegal++;break;}
  Shape *shape=&shapes[g.p[move.pieceIndex]];
  U64 mask=shape->placed[move.row*8+move.col];int lines=0;
  U64 after=place(g.b,mask,&lines);
  if(!mask||(g.b&mask)||after!=move.after||lines!=move.linesCleared){result.illegal++;break;}
  if(g.known&&g.streak>0&&g.misses==2&&lines==0 &&safe_clear_now(g.b,g.p))result.skipped++;
  result.moves++;
  int multiplier=g.streak>8?8:g.streak;
  result.proxy+=shape->n+10.0*lines*lines*(multiplier?multiplier:1);
  g.b=after;g.p[move.pieceIndex]=-1;
  if(lines){g.known=1;g.streak=g.streak<63?g.streak+1:63;g.misses=0;++result.clears;}
  else if(g.known&&++g.misses>=3){g.misses=0;g.streak=0;++result.breaks;}
  if(g.p[0]<0&&g.p[1]<0&&g.p[2]<0){
    if(out && nSamples<TRAJECTORY){samples[nSamples++]=(Snap){g.b,g.streak,g.misses,result.moves};}
    draw_tray(&g,stress);
  }
 }
 // Cap at 200 moves: censor rather than label a nonterminal win as death.
 if(out&&result.moves<MAXMOVES){
  for(int i=0;i<nSamples;i++){
    double f[V];after_features(samples[i].board,samples[i].streak,samples[i].misses,f);
    fprintf(out,"%d,%d,%d",game_id,stress,result.moves-samples[i].step);
    for(int j=0;j<V;j++)fprintf(out,",%.9g",f[j]);fputc('\n',out);
  }
 }
 return result;
}
static int compare_int(const void *a,const void *b){return *(const int*)a-*(const int*)b;}
#ifndef BL_RESEARCH_NO_MAIN
int main(int argc,char **argv){
 if(argc<2){fprintf(stderr,"Usage: %s collect CSV seedsPerScenario | test MODEL nSeeds [seedOffset]\n",argv[0]);return 2;}
 init_shapes();
 if(strcmp(argv[1],"collect")==0){
  if(argc<4)return 2;
  FILE *out=fopen(argv[2],"w");if(!out){perror(argv[2]);return 2;}
  fprintf(out,"game,scenario,remaining");for(int k=0;k<V;k++)fprintf(out,",%s",names[k]);fputc('\n',out);
  int n=intarg(argv[3],1,200),id=0;
  for(int scenario=0;scenario<2;scenario++)for(int i=0;i<n;i++){
   uint32_t seed=18000000u+scenario*500000u+i*3157u;
   Result r=play_or_collect(seed,scenario,0,id++,out);
   if(r.illegal){fprintf(stderr,"Illegal training move\n");return 1;}
  }
  fclose(out);printf("Training data collected from %d independent v1.7 games.\n",id);return 0;
 }
 if(strcmp(argv[1],"test")==0){
  if(argc<4||!model_load(argv[2]))return 2;
  int n=intarg(argv[3],1,300),offset=argc>4?intarg(argv[4],0,100000000):0;
  for(int scenario=0;scenario<3;scenario++){
    long moves[2]={0},breaks[2]={0},skips[2]={0},illegal[2]={0};double proxy[2]={0};
    int *lens[2]={malloc(n*sizeof(int)),malloc(n*sizeof(int))};
    for(int p=0;p<2;p++)for(int i=0;i<n;i++){
      uint32_t seed=135000000u+(uint32_t)offset+scenario*10000000u+i*3137u;
      Result r=play_or_collect(seed,scenario,p,i,NULL);
      moves[p]+=r.moves;breaks[p]+=r.breaks;skips[p]+=r.skipped;illegal[p]+=r.illegal;
      proxy[p]+=r.proxy;lens[p][i]=r.moves;
    }
    for(int p=0;p<2;p++)qsort(lens[p],n,sizeof(int),compare_int);
    printf("scenario=%s games=%d baseline_moves=%.3f learned_moves=%.3f baseline_p10=%d learned_p10=%d baseline_proxy=%.1f learned_proxy=%.1f baseline_combo_breaks_per_100=%.3f learned_combo_breaks_per_100=%.3f baseline_avoidable_skips=%ld learned_avoidable_skips=%ld illegal=%ld+%ld overrides=%d\n",
      scenario==0?"normal":(scenario==1?"stress":"shifted_heavy"),n,(double)moves[0]/n,(double)moves[1]/n,
      lens[0][(n-1)/10],lens[1][(n-1)/10],proxy[0]/n,proxy[1]/n,
      100.*breaks[0]/(moves[0]?moves[0]:1),100.*breaks[1]/(moves[1]?moves[1]:1),
      skips[0],skips[1],illegal[0],illegal[1],changes);
    free(lens[0]);free(lens[1]);
  }
  return 0;
 }
 return 2;
}

#endif /* BL_RESEARCH_NO_MAIN */