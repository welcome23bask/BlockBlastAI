/* BlockBlastAI v2.0 - reproducible offline self-play learning lab.
   This is an independent game-model approximation, NOT the actual game's
   proprietary piece-generation or points algorithm.
   Optimizes a 12-feature linear afterstate evaluator via paired-seed
   evolutionary self-play. NEVER modify a live iPhone dylib from this program.
   Build: cc -O3 -std=c11 learning/learn.c -lm -o learn
   Usage: ./learn [rounds=40] [candidates=5] [seeds_per_round=40] [holdout=250]
*/
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include <time.h>
#include <limits.h>

#define F 12
#define MAXMOVES 200
#define N_SHAPES 28
typedef uint64_t U64;
typedef struct { int n,w,h; int dx[9],dy[9]; U64 placed[64]; } Shape;
static Shape shapes[N_SHAPES];
static const char *patterns[N_SHAPES]={
 "#","##","###","####","#####","#/#","#/#/#","#/#/#/#","#/#/#/#/#",
 "##/##","###/###/###","##/##/##","###/###","##./.##",".##/##.",
 "#./##",".#/##","###/#..","###/..#","###/.#.",".#./###","##./.##/..#",
 "###/#.#","#.#/###","#../###","..#/###","##./##./.#.",".#./###/.#."
};
typedef struct { int n; U64 masks[64]; } Fit;
static Fit fit[5];
static U64 rowMask[8],colMask[8];
static uint32_t rng;
static uint32_t random32(void){uint32_t x=rng;x^=x<<13;x^=x>>17;x^=x<<5;return rng=x;}
static void seed_rng(uint32_t n){rng=n?n:0xa1b2c3d4u;}
static double randu(void){return (random32()>>8)*(1.0/16777216.0);}
static int pop(U64 b){return __builtin_popcountll(b);}
static void init_shapes(void) {
    for(int i=0;i<8;++i) {
        rowMask[i]=(U64)255<<(8*i);
        colMask[i]=UINT64_C(0x0101010101010101)<<i;
    }
    for(int i=0;i<N_SHAPES;++i) {
        Shape *s=&shapes[i];int x=0,y=0;
        for(const char *p=patterns[i];*p;p++){
            if(*p=='/'){++y;x=0;continue;}
            if(*p=='#'){if(s->n>=9)abort();s->dx[s->n]=x;s->dy[s->n]=y;++s->n;
                if(s->w<x+1)s->w=x+1;if(s->h<y+1)s->h=y+1;}
            ++x;
        }
        if(!s->n||s->w>8||s->h>8)abort();
        for(int r=0;r<=8-s->h;r++)for(int c=0;c<=8-s->w;c++) {
            U64 mask=0;
            for(int k=0;k<s->n;k++)mask|=UINT64_C(1)<<((r+s->dy[k])*8+c+s->dx[k]);
            s->placed[r*8+c]=mask;
        }
    }
    const int widths[]={3,5,1,3,2},heights[]={3,1,5,2,3};
    for(int p=0;p<5;p++)for(int r=0;r<=8-heights[p];r++)
      for(int c=0;c<=8-widths[p];c++) {
        U64 mask=0;
        for(int y=0;y<heights[p];y++)for(int x=0;x<widths[p];x++)
            mask|=UINT64_C(1)<<((r+y)*8+c+x);
        fit[p].masks[fit[p].n++]=mask;
      }
}
static int countfit(U64 board,int idx){
    int n=0;Fit *f=&fit[idx];
    for(int i=0;i<f->n;i++)n+=!(board&f->masks[i]);
    return n;
}
static U64 place(U64 before,U64 mask,int *lines) {
    U64 b=before|mask,erase=0;*lines=0;
    for(int i=0;i<8;i++){
        if((b&rowMask[i])==rowMask[i]){erase|=rowMask[i];++*lines;}
        if((b&colMask[i])==colMask[i]){erase|=colMask[i];++*lines;}
    }
    return b&~erase;
}
typedef struct { U64 b;int p[3],misses,streak,known; } State;
typedef struct { int piece,row,col,clears;U64 after; } Action;
typedef struct {int moves,clears,breaks,skipped,illegal;double proxy;} Result;
static void new_tray(State *s,int stress){
    static const int big[]={9,10,11,12,13,14,21,22,23,24,25,26,27};
    for(int i=0;i<3;i++){
        int p=(int)(random32()%N_SHAPES);
        if(stress && random32()%3<2)p=big[random32()%(sizeof(big)/sizeof(big[0]))];
        s->p[i]=p;
    }
}
static int options_on(U64 b,int piece){
    Shape *s=&shapes[piece];
    int n=0;
    for(int r=0;r<=8-s->h;r++)
      for(int c=0;c<=8-s->w;c++)n+=!(b&s->placed[r*8+c]);
    return n;
}
static int all_placements(U64 b,const int pieces[3]){
    int n=0;for(int i=0;i<3;i++)if(pieces[i]>=0)n+=options_on(b,pieces[i]);
    return n;
}
static int can_clear_now(U64 b,const int pieces[3]){
    for(int i=0;i<3;i++)if(pieces[i]>=0){
        Shape *p=&shapes[pieces[i]];
        for(int r=0;r<=8-p->h;r++)for(int c=0;c<=8-p->w;c++){
            U64 mask=p->placed[r*8+c];if(mask&b)continue;
            int lines=0;(void)place(b,mask,&lines);
            if(lines)return 1;
        }
    }
    return 0;
}
static int isolated(U64 board) {
    int isolated=0;
    for(int r=0;r<8;r++)for(int c=0;c<8;c++) {
        int i=r*8+c;if(board&(UINT64_C(1)<<i))continue;
        int neighbors=0;
        if(r>0)neighbors+=!(board&(UINT64_C(1)<<(i-8)));
        if(r<7)neighbors+=!(board&(UINT64_C(1)<<(i+8)));
        if(c>0)neighbors+=!(board&(UINT64_C(1)<<(i-1)));
        if(c<7)neighbors+=!(board&(UINT64_C(1)<<(i+1)));
        if(neighbors<=1)++isolated;
    }
    return isolated;
}
static void features(const State *g,Action a,double f[F]){
    U64 b=a.after;
    const int free=64-pop(b),s3=countfit(b,0),h5=countfit(b,1),
       v5=countfit(b,2),rect=countfit(b,3)+countfit(b,4);
    memset(f,0,F*sizeof(double));
    f[0]=free/64.0;
    f[1]=fmin(s3,10)/10.0;
    f[2]=fmin(h5,10)/10.0;
    f[3]=fmin(v5,10)/10.0;
    f[4]=fmin(rect,16)/16.0;
    f[5]=-fmin(isolated(b),12)/12.0;
    f[6]=a.clears/5.0;
    f[7]=(g->known&&g->streak>0&&a.clears>0)?1.0:0.0;
    f[8]=(g->known&&g->streak>0&&g->misses==2&&a.clears==0)?-1.0:0.0;
    int next[3]={g->p[0],g->p[1],g->p[2]};
    next[a.piece]=-1;
    f[9]=fmin(all_placements(b,next),40)/40.0;
    f[10]=(free<28 && s3==0)?-1.0:0.0;
    int near=0;
    for(int r=0;r<8;r++)near+=pop(b&rowMask[r])>=6;
    for(int c=0;c<8;c++)near+=pop(b&colMask[c])>=6;
    f[11]=near/16.0;
}
static const double initial[F]={
    2.5,3.2,1.2,1.2,1.6,1.0,3.8,3.0,7.0,1.8,2.5,0.5
};
static Action choose(const State *g,const double weights[F]){
    Action best={.piece=-1};double top=-1e30;
    for(int i=0;i<3;i++)if(g->p[i]>=0){
        Shape *sh=&shapes[g->p[i]];
        for(int r=0;r<=8-sh->h;r++)for(int c=0;c<=8-sh->w;c++){
            U64 mask=sh->placed[r*8+c];
            if(g->b&mask)continue;
            Action a={.piece=i,.row=r,.col=c};
            a.after=place(g->b,mask,&a.clears);
            double x[F];features(g,a,x);
            double score=0;for(int j=0;j<F;j++)score+=weights[j]*x[j];
            if(score>top+1e-9){top=score;best=a;}
        }
    }
    return best;
}
static Result episode(const double w[F],uint32_t seed,int stress){
    seed_rng(seed);State g={0};new_tray(&g,stress);Result out={0};
    for(int k=0;k<MAXMOVES;k++){
        Action a=choose(&g,w);
        if(a.piece<0)break;
        if(g.p[a.piece]<0){out.illegal++;break;}
        Shape *s=&shapes[g.p[a.piece]];
        U64 mask=s->placed[a.row*8+a.col];
        if((mask&g.b)||!mask){out.illegal++;break;}
        if(g.known&&g.streak>0&&g.misses==2&&a.clears==0) {
            if(can_clear_now(g.b,g.p))out.skipped++;
        }
        g.b=a.after;g.p[a.piece]=-1;
        ++out.moves;
        if(a.clears>0){
            ++out.clears;g.known=1;g.streak=g.streak<63?g.streak+1:63;g.misses=0;
        }else if(g.known){
            ++g.misses;
            if(g.misses>=3){++out.breaks;g.streak=0;g.misses=0;}
        }
        int score_streak=g.streak>8?8:g.streak;
        out.proxy+=s->n+10.0*a.clears*a.clears*(score_streak?score_streak:1);
        if(g.p[0]<0&&g.p[1]<0&&g.p[2]<0)new_tray(&g,stress);
    }
    return out;
}
typedef struct {
    int games,illegal;double moves,score,clears,breaks,skips;
    int *lengths;
} Stats;
static Stats evaluate(const double w[F],int seeds,uint32_t baseSeed,int pairedStress){
    Stats s={0};s.games=seeds;s.lengths=calloc(seeds,sizeof(int));if(!s.lengths)abort();
    for(int i=0;i<seeds;i++){
        const int stress=pairedStress?i%2:0;
        uint32_t sd=baseSeed+(uint32_t)(i*977+1);
        Result e=episode(w,sd,stress);
        s.moves+=e.moves;s.score+=e.proxy;s.clears+=e.clears;
        s.breaks+=e.breaks;s.skips+=e.skipped;s.illegal+=e.illegal;
        s.lengths[i]=e.moves;
    }
    return s;
}
static int intcmp(const void *a,const void *b){
    int x=*(const int*)a,y=*(const int*)b;return x>y?1:x<y?-1:0;
}
static double fitness(Stats *s) {
    double n=s->games;
    // Survival dominates. Reward small clears and uninterrupted combos;
    // estimated scores play a limited secondary role.
    return (s->moves+0.9*s->clears-2.0*s->breaks
            -1.8*s->skips+0.003*fmin(s->score,300000*n))/n
           -500*s->illegal;
}
static void report(const char *label,Stats *s) {
    qsort(s->lengths,s->games,sizeof(int),intcmp);
    const int p10=s->lengths[(s->games-1)/10];
    const int med=s->lengths[s->games/2];
    printf("%s games=%d avg_moves=%.3f median=%d p10=%d avg_clears=%.3f "
           "avg_combo_breaks=%.3f missed_clear_deadlines=%g "
           "score_proxy=%.2f illegal=%d fitness=%.4f\n",
        label,s->games,s->moves/s->games,med,p10,s->clears/s->games,
        s->breaks/s->games,s->skips,s->score/s->games,s->illegal,fitness(s));
}
static void free_stats(Stats *s){free(s->lengths);s->lengths=NULL;}
static int intarg(const char *s,int lower,int upper){
    char *end=NULL;long v=strtol(s,&end,10);
    if(!s[0]||*end||v<lower||v>upper){fprintf(stderr,"Invalid argument: %s\n",s);exit(2);}
    return (int)v;
}
static void export_weights(const double w[F],const char *path,int promoted) {
    FILE *f=fopen(path,"w");if(!f){perror(path);exit(1);}
    fprintf(f,"{\n  \"format\": \"BLTrainedValue-v1\",\n"
              "  \"source\": \"offline evolving linear policy, synthetic Block Blast\",\n"
              "  \"features\": [\"free\",\"fit_3x3\",\"fit_horizontal_5\",\"fit_vertical_5\","
              "\"fit_rectangles\",\"isolated\",\"lines_now\",\"clear_preserves_streak\","
              "\"combo_deadline_miss\",\"tray_placement_options\",\"crowded_missing_3x3\","
              "\"almost_full_lines\"],\n  \"weights\": [");
    for(int i=0;i<F;i++)fprintf(f,"%s%.9g",i?", ":"",w[i]);
    fprintf(f,"],\n  \"promoted_for_ios\": %s,\n"
              "  \"warning\": \"No real game scoring or next-piece distribution verified\"\n}\n",
            promoted?"true":"false");
    fclose(f);
}
static void export_holdout(const char *path,Stats *baseline,Stats *trained,
                           int pass,int gamesTrain) {
    FILE *f=fopen(path,"w");if(!f){perror(path);exit(1);}
    fprintf(f,"{\n  \"trained_games\": %d,\n  \"holdout_games_per_agent\": %d,\n"
         "  \"baseline\": {\"avg_moves\": %.4f, \"avg_proxy_score\": %.4f, "
         "\"p10\": %d, \"combo_breaks_per_game\": %.4f, \"illegal\": %d},\n"
         "  \"trained\": {\"avg_moves\": %.4f, \"avg_proxy_score\": %.4f, "
         "\"p10\": %d, \"combo_breaks_per_game\": %.4f, \"illegal\": %d},\n"
         "  \"passed_safety_gate\": %s,\n"
         "  \"notes\": \"Proxy metrics. Baseline is hand-weighted one-step policy, NOT the v1.7 three-piece AI.\"\n}\n",
         gamesTrain,baseline->games,baseline->moves/baseline->games,
         baseline->score/baseline->games,baseline->lengths[(baseline->games-1)/10],
         baseline->breaks/baseline->games,baseline->illegal,
         trained->moves/trained->games,trained->score/trained->games,
         trained->lengths[(trained->games-1)/10],
         trained->breaks/trained->games,trained->illegal,pass?"true":"false");
    fclose(f);
}
int main(int argc,char **argv){
    const int rounds=argc>1?intarg(argv[1],1,150):40;
    const int candidates=argc>2?intarg(argv[2],1,16):5;
    const int seeds=argc>3?intarg(argv[3],2,200):40;
    const int holdout=argc>4?intarg(argv[4],20,1000):250;
    init_shapes();
    double champion[F];memcpy(champion,initial,sizeof(champion));
    seed_rng(0x56271628);
    int trainedGames=0;
    printf("TRAIN: rounds=%d candidates=%d paired_seeds=%d holdout=%d "
           "max_episode_moves=%d pieces=%d\n",
           rounds,candidates,seeds,holdout,MAXMOVES,N_SHAPES);
    for(int step=0;step<rounds;step++){
        uint32_t base=100000u+(uint32_t)step*200000u;
        Stats baseStats=evaluate(champion,seeds,base,1);
        trainedGames+=seeds;
        double bestFitness=fitness(&baseStats);
        double best[F];memcpy(best,champion,sizeof(best));
        free_stats(&baseStats);
        double anneal=0.4*(1.0-0.8*(double)step/rounds);
        for(int attempt=0;attempt<candidates;attempt++){
            double trial[F];
            for(int j=0;j<F;j++){
                double perturb=(randu()*2.0-1.0)*anneal*(fabs(champion[j])+2.0);
                trial[j]=champion[j]+perturb;
                if(trial[j]>32)trial[j]=32;
                if(trial[j]<-32)trial[j]=-32;
            }
            Stats e=evaluate(trial,seeds,base,1);
            trainedGames+=seeds;
            double score=fitness(&e);
            if(score>bestFitness+0.00001){bestFitness=score;memcpy(best,trial,sizeof(best));}
            free_stats(&e);
        }
        memcpy(champion,best,sizeof(best));
        if((step+1)%5==0||step==0){
            printf("Progress round %d/%d: training_games=%d paired_fitness=%.3f\n",
                   step+1,rounds,trainedGames,bestFitness);
            fflush(stdout);
        }
    }
    Stats base=evaluate(initial,holdout,90000000u,1);
    Stats learned=evaluate(champion,holdout,90000000u,1);
    report("HOLDOUT baseline_one_step",&base);
    report("HOLDOUT trained_one_step",&learned);
    // Gate is intentionally strict. Only holdout performance can earn
    // promotion; learning is never integrated into the phone automatically.
    int pass=learned.illegal==0 && base.illegal==0 &&
             learned.moves>=base.moves*1.03 &&
             learned.lengths[(holdout-1)/10]>=base.lengths[(holdout-1)/10] &&
             learned.breaks<=base.breaks*1.10 &&
             learned.score>=base.score*0.98;
    printf("TRAINING COMPLETE: total_self_play_episodes=%d; "
           "holdout_episodes=%d; safety_gate=%s\n",
           trainedGames,2*holdout,pass?"PASS":"FAIL");
    export_weights(champion,"learned_weights.json",0);
    export_holdout("learning_results.json",&base,&learned,pass,trainedGames);
    free_stats(&base);free_stats(&learned);
    return 0;
}
