/* BlockBlastAI v2.3 -- independent paired-seed offline research.
 * Train ONLY; never update a live .dylib based on in-sample reward.
 * cc -O3 -std=c11 -I. learning/train_paired_v23.c
 *   learning/BLTrainableStrategy.c (plus frozen v1.7 solver) ...
 */
#define BL_TENX_EMBEDDED
#define BL_TENX_NODE_BUDGET 18000
#include "train_tenx.c"

typedef struct {
    double moveLog[3],scoreRatio[3],comboRate[3],reward;
    int skipped,illegal,catastrophes,episodes;
} PairedEval;

static PairedEval evaluate_pair(const int weights[4],int count,int epoch,int verbose){
    PairedEval x={0};
    for(int scenario=0;scenario<3;scenario++){
        double logGain=0,score=0,combo=0;
        for(int j=0;j<count;j++){
            uint32_t seed=seed_for(scenario,j,epoch);
            Result original=episode_complete(seed,scenario,zero,1);
            Result candidate=episode_complete(seed,scenario,weights,0);
            x.episodes+=2;
            x.illegal+=original.illegal+candidate.illegal;
            if(candidate.skipped>original.skipped)x.skipped+=candidate.skipped-original.skipped;
            double ratio=(candidate.moves+3.0)/(original.moves+3.0);
            double scoreRatio=(candidate.proxy+30.0)/(original.proxy+30.0);
            logGain+=log(ratio);score+=scoreRatio;
            double before=(original.breaks+0.5)/(original.moves+20.0);
            double after=(candidate.breaks+0.5)/(candidate.moves+20.0);
            combo+=after-before;
            if(ratio<0.70)x.catastrophes++;
        }
        x.moveLog[scenario]=logGain/count;
        x.scoreRatio[scenario]=score/count;
        x.comboRate[scenario]=combo/count;
        x.reward+=(scenario?1.15:1.0)*(x.moveLog[scenario]+0.24*log(x.scoreRatio[scenario]));
        if(x.moveLog[scenario]<0)x.reward+=2.8*x.moveLog[scenario];
        if(x.comboRate[scenario]>0)x.reward-=25.0*x.comboRate[scenario];
    }
    x.reward-=0.08*x.catastrophes;
    x.reward-=0.5*x.skipped;
    if(x.illegal)x.reward-=1000;
    if(verbose) {
        printf("PAIRED weights=%d,%d,%d,%d epoch=%d games_per_scenario=%d "
               "reward=%+.4f catastrophic=%d avoidable_misses=%d illegal=%d\n",
               weights[0],weights[1],weights[2],weights[3],
               epoch,count,x.reward,x.catastrophes,x.skipped,x.illegal);
        for(int i=0;i<3;i++)printf(" scenario=%d mean_log_survival=%+.4f "
                "avg_score_ratio=%.4f combo_break_rate_delta=%+.5f\n",
                i,x.moveLog[i],x.scoreRatio[i],x.comboRate[i]);
    }
    return x;
}
static uint32_t mutate_v23(uint32_t *s) {
    *s^=*s<<13;*s^=*s>>17;*s^=*s<<5;return *s;
}
int main(int argc,char **argv){
    int rounds=argc>1?atoi(argv[1]):3;
    int candidates=argc>2?atoi(argv[2]):8;
    int samples=argc>3?atoi(argv[3]):4;
    if(rounds<1||rounds>30||candidates<1||candidates>25||samples<2||samples>25)return 2;
    init_shapes();
    int weights[4]={0};
    FILE *prior=fopen("tenx_candidate_weights.txt","r");
    if(prior) {
        int w[4]={0};
        if(fscanf(prior,"%d %d %d %d",w,w+1,w+2,w+3)==4){
            PairedEval old=evaluate_pair(w,samples,606,0);
            PairedEval baseline=evaluate_pair(zero,samples,606,0);
            if(old.reward>baseline.reward+0.005)memcpy(weights,w,sizeof(weights));
            printf("PRIOR candidate=%+.4f baseline=%+.4f chosen=%s\n",
                    old.reward,baseline.reward,
                    old.reward>baseline.reward+0.005?"candidate":"baseline");
        }
        fclose(prior);
    }
    uint32_t rng=0x6e7b28f1u;
    int episodes=0;
    for(int epoch=0;epoch<rounds;epoch++){
        const int batch=610+epoch;
        PairedEval live=evaluate_pair(weights,samples,batch,0);
        PairedEval anchor=evaluate_pair(weights,samples,606,0);
        episodes+=live.episodes+anchor.episodes;
        double best=.65*live.reward+.35*anchor.reward;
        int next[4];memcpy(next,weights,sizeof(next));
        for(int attempt=0;attempt<candidates;attempt++){
            int proposal[4];memcpy(proposal,weights,sizeof(proposal));
            const int d[]={-160,-100,-60,-30,30,60,100,160};
            int idx=mutate_v23(&rng)%4;
            proposal[idx]+=d[mutate_v23(&rng)%8];
            if(proposal[idx]>300)proposal[idx]=300;
            if(proposal[idx]<-300)proposal[idx]=-300;
            if(attempt%3==2){
                int other=(idx+1+mutate_v23(&rng)%3)%4;
                proposal[other]+=(mutate_v23(&rng)%2)?30:-30;
                if(proposal[other]>300)proposal[other]=300;
                if(proposal[other]<-300)proposal[other]=-300;
            }
            PairedEval a=evaluate_pair(proposal,samples,batch,0);
            PairedEval b=evaluate_pair(proposal,samples,606,0);
            episodes+=a.episodes+b.episodes;
            double score=.65*a.reward+.35*b.reward;
            if(score>best+.0005){best=score;memcpy(next,proposal,sizeof(next));}
        }
        memcpy(weights,next,sizeof(weights));
        printf("ROUND %d/%d model=%d,%d,%d,%d paired_train_score=%+.4f episodes=%d\n",
               epoch+1,rounds,weights[0],weights[1],weights[2],weights[3],best,episodes);
        fflush(stdout);
        FILE *f=fopen("v23_paired_checkpoint.tmp","w");if(!f)abort();
        fprintf(f,"%d %d %d %d\n",weights[0],weights[1],weights[2],weights[3]);
        fclose(f);
        if(rename("v23_paired_checkpoint.tmp","v23_paired_checkpoint.txt"))abort();
    }
    puts("TRAINING FINISHED. Independent holdout follows. NOT APPROVED FOR IPHONE.");
    evaluate_pair(weights,12,851,1);
    return 0;
}
