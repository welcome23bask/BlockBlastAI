/* Six adaptive cycles of five new whole-tray evaluation features.
 * Research only: never modifies live iPhone .dylib.
 * Each stage uses paired training seeds, independent diagnostic seeds,
 * and final unseen holdout seeds. Exact same baseline and candidate RNG.
 */
#define BL_TENX_EMBEDDED
#ifndef BL_TENX_NODE_BUDGET
#define BL_TENX_NODE_BUDGET 3500
#endif
#ifndef MAXMOVES
#define MAXMOVES 250
#endif
#include "train_tenx.c"
#include <float.h>
void BLSetV25Weights(const int *w,int groups);
#define GROUPS 6
#define LIMIT 20
typedef struct { double moves,score,breaks,misses;int illegal,p10,n, lengths[LIMIT]; } Totals;
typedef struct {Totals original[3],candidate[3];double reward;int valid;} Eval;
static int smaller(const void*a,const void*b){int x=*(const int*)a,y=*(const int*)b;return x<y?-1:x>y;}
static void accumulate(Totals *t,Result r){
    t->lengths[t->n++]=r.moves;
    t->moves+=r.moves;t->score+=r.proxy;t->breaks+=r.breaks;
    t->misses+=r.skipped;t->illegal+=r.illegal;
}
static void percentile(Totals *t){
    int temp[LIMIT];memcpy(temp,t->lengths,sizeof(int)*t->n);
    qsort(temp,t->n,sizeof(int),smaller);
    t->p10=temp[(t->n-1)/10];
}
static Eval evaluate(const int gamma[6],int stage,int epoch,int count){
    Eval e={0};e.valid=1;
    const int originalWeights[4]={0};
    const int incumbentWeights[4]={120,120,0,-20};
    BLSetV25Weights(gamma,stage);
    for(int scenario=0;scenario<3;scenario++){
        double logSurvival=0,logScore=0;
        for(int i=0;i<count;i++){
            uint32_t seed=seed_for(scenario,i,epoch);
            Result b=episode_complete(seed,scenario,originalWeights,1);
            Result a=episode_complete(seed,scenario,incumbentWeights,0);
            accumulate(&e.original[scenario],b);
            accumulate(&e.candidate[scenario],a);
            logSurvival+=log((a.moves+4.0)/(b.moves+4.0));
            logScore+=log((a.proxy+50.0)/(b.proxy+50.0));
            if(b.illegal||a.illegal)e.valid=0;
        }
        Totals *b=&e.original[scenario],*a=&e.candidate[scenario];
        percentile(a);percentile(b);
        double survival=a->moves/(b->moves+1e-9);
        double breakRate=(a->breaks+.1)/(a->moves+10.0) /
                         ((b->breaks+.1)/(b->moves+10.0));
        e.reward+=(scenario?1.5:1)*(logSurvival/count+.2*logScore/count);
        if(a->misses>b->misses)e.reward-=2*(a->misses-b->misses)/count;
        if(breakRate>1.25)e.reward-=.28*(breakRate-1.25);
        if(survival<.8)e.reward-=.9*(.8-survival);
        if(scenario>0&&survival<.97)e.reward-=.48*(.97-survival);
        if(a->p10<b->p10)e.reward-=.02*(b->p10-a->p10);
    }
    if(!e.valid)e.reward=-1000000;
    return e;
}
static void report(FILE *csv,const char *kind,int round,int epoch,int gamma,const Eval *e){
    for(int i=0;i<3;i++){
        const Totals *b=&e->original[i],*a=&e->candidate[i];
        double ratio=a->moves/(b->moves+1e-9),score=a->score/(b->score+1e-9);
        double oldRate=100*b->breaks/(b->moves+1e-9);
        double newRate=100*a->breaks/(a->moves+1e-9);
        printf("%s round=%d scenario=%d gamma=%d epoch=%d baseline=%.3f candidate=%.3f "
               "survival_ratio=%.4f proxy_ratio=%.4f p10=%d:%d combo_rate=%.3f:%.3f "
               "missed_clear=%.0f:%.0f illegal=%d:%d\n",
               kind,round,i,gamma,epoch,b->moves/b->n,a->moves/a->n,ratio,score,
               b->p10,a->p10,oldRate,newRate,b->misses,a->misses,b->illegal,a->illegal);
        if(csv)fprintf(csv,"%d,%s,%d,%d,%d,%.6f,%.6f,%.6f,%.6f,%d,%d,%.6f,%.6f,%.0f,%.0f,%d,%d\n",
              round,kind,epoch,i,gamma,b->moves/b->n,a->moves/a->n,ratio,score,
              b->p10,a->p10,oldRate,newRate,b->misses,a->misses,b->illegal,a->illegal);
    }
    fflush(stdout);
}
int main(int argc,char **argv){
    int n=argc>1?atoi(argv[1]):3;
    int diagnosis=argc>2?atoi(argv[2]):3;
    int final=argc>3?atoi(argv[3]):6;
    if(n<2||n>10||diagnosis<2||diagnosis>12||final<2||final>LIMIT)return 2;
    init_shapes();
    FILE *csv=fopen("v25_batch_metrics.csv","w");if(!csv){perror("v25_batch_metrics.csv");return 2;}
    fprintf(csv,"round,kind,epoch,scenario,gamma,base_moves,learned_moves,move_ratio,proxy_ratio,base_p10,learned_p10,base_combo_rate,learned_combo_rate,base_misses,learned_misses,base_illegal,learned_illegal\n");
    int gamma[6]={0},choices[]={-3,-2,-1,0,1,2,3};
    for(int round=1;round<=6;round++){
        printf("=== BATCH %d/6: FEATURE NUMBERS %d–%d ===\n",round,(round-1)*5+1,round*5);
        int selected=0;double best=-DBL_MAX;
        for(int j=0;j<7;j++){
            gamma[round-1]=choices[j];
            Eval a=evaluate(gamma,round,241+round,n);
            Eval b=evaluate(gamma,round,261+round,n);
            double reward=.5*(a.reward+b.reward);
            if(a.valid&&b.valid&&reward>best){best=reward;selected=choices[j];}
        }
        gamma[round-1]=0;
        Eval incumbent=evaluate(gamma,round,401+round,diagnosis);
        gamma[round-1]=selected;
        Eval candidate=evaluate(gamma,round,401+round,diagnosis);
        int accept=selected!=0&&candidate.valid &&
                   candidate.reward>=incumbent.reward+.008;
        for(int i=0;i<3;i++)
            if(candidate.candidate[i].moves<.83*incumbent.candidate[i].moves ||
               candidate.candidate[i].misses>incumbent.candidate[i].misses ||
               candidate.candidate[i].illegal>incumbent.candidate[i].illegal)accept=0;
        if(!accept){
            printf("BATCH_REJECT round=%d proposed=%d previous_fitness=%+.4f new_fitness=%+.4f\n",
                   round,selected,incumbent.reward,candidate.reward);
            selected=0;gamma[round-1]=0;candidate=incumbent;
        }
        report(csv,"DIAGNOSTIC",round,401+round,selected,&candidate);
        printf("BATCH_RESULT round=%d selected_weight=%d training_reward=%+.4f\n",round,selected,best);
        fflush(csv);
    }
    FILE *w=fopen("v25_weights.txt","w");if(!w)abort();
    for(int i=0;i<6;i++)fprintf(w,"%s%d",i?" ":"",gamma[i]);
    fputc('\n',w);fclose(w);
    int robust=1,tenx=1;
    for(int stage=0;stage<2;stage++){
        int epoch=891+17*stage;
        Eval e=evaluate(gamma,6,epoch,final);
        report(csv,"HOLDOFF",6,epoch,gamma[5],&e);
        for(int i=0;i<3;i++){
            const Totals *a=&e.candidate[i],*b=&e.original[i];
            double survival=a->moves/(b->moves+1e-9),score=a->score/(b->score+1e-9);
            if(!e.valid||survival<1.02||score<.98||a->p10<b->p10||
               a->breaks/(a->moves+1e-9)>b->breaks/(b->moves+1e-9)||
               a->misses>b->misses)robust=0;
            if(!e.valid||survival<10||score<10||a->p10<10*b->p10||
               a->breaks/(a->moves+1e-9)>b->breaks/(b->moves+1e-9)||
               a->misses>b->misses)tenx=0;
        }
    }
    printf("SIX_ROUND_GATE consistent_gain=%s ten_x=%s iphone_release=NO\n",
           robust?"PASS":"NOT_PROVEN",tenx?"PASS":"NOT_REACHED");
    fclose(csv);
    return 0;
}
