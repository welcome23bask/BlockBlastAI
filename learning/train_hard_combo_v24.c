/* BlockBlastAI v2.4 -- hard-piece combo-safe paired offline research.
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
    int weakTail[3]; 
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
            if(ratio<0.70){x.catastrophes++;x.weakTail[scenario]++;}
            // A big-piece scenario with an early death is more damaging
            // than a lucky long normal-piece run is valuable.
            if(scenario>0 && ratio<0.90)x.reward-=0.025;
            if(candidate.moves<original.moves/2 && original.moves>10)
                x.reward-=0.10;
        }
        x.moveLog[scenario]=logGain/count;
        x.scoreRatio[scenario]=score/count;
        x.comboRate[scenario]=combo/count;
        // Heavy and shifted-piece survival are the bottleneck. The
        // objective is balanced across distributions (not merely average).
        const double weight=scenario?1.7:1.0;
        x.reward+=weight*(x.moveLog[scenario]+0.18*log(x.scoreRatio[scenario]));
        if(x.moveLog[scenario]<0)x.reward+=(scenario?5.0:3.0)*x.moveLog[scenario];
        if(x.comboRate[scenario]>0)x.reward-=38.0*x.comboRate[scenario];
        x.reward-=(scenario?0.10:0.04)*x.weakTail[scenario]/count;
    }
    x.reward-=0.08*x.catastrophes;
    x.reward-=1.5*x.skipped;
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
static uint32_t mutate_v24(uint32_t *s) {
    *s^=*s<<13;*s^=*s>>17;*s^=*s<<5;return *s;
}
int main(int argc,char **argv){
    int rounds=argc>1?atoi(argv[1]):4;
    int candidates=argc>2?atoi(argv[2]):10;
    int samples=argc>3?atoi(argv[3]):6;
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
    uint32_t rng=0x6e7b28f1u ^ 0x0000aa31u;
    int episodes=0;
    for(int epoch=0;epoch<rounds;epoch++){
        const int batch=710+epoch;
        PairedEval live=evaluate_pair(weights,samples,batch,0);
        PairedEval anchor=evaluate_pair(weights,samples,706,0);
        // Separate 720-series guard seeds are used only for candidate
        // acceptance, NOT the final 940+ holdout epochs.
        PairedEval guard=evaluate_pair(weights,samples,720,0);
        episodes+=live.episodes+anchor.episodes+guard.episodes;
        double best=.45*live.reward+.25*anchor.reward+.30*guard.reward;
        int next[4];memcpy(next,weights,sizeof(next));
        for(int attempt=0;attempt<candidates;attempt++){
            int proposal[4];memcpy(proposal,weights,sizeof(proposal));
            const int d[]={-120,-80,-50,-25,25,50,80,120};
            int idx=mutate_v24(&rng)%4;
            proposal[idx]+=d[mutate_v24(&rng)%8];
            if(proposal[idx]>300)proposal[idx]=300;
            if(proposal[idx]<-300)proposal[idx]=-300;
            if(attempt%3==2){
                int other=(idx+1+mutate_v24(&rng)%3)%4;
                proposal[other]+=(mutate_v24(&rng)%2)?30:-30;
                if(proposal[other]>300)proposal[other]=300;
                if(proposal[other]<-300)proposal[other]=-300;
            }
            PairedEval a=evaluate_pair(proposal,samples,batch,0);
            PairedEval b=evaluate_pair(proposal,samples,706,0);
            PairedEval c=evaluate_pair(proposal,samples,720,0);
            episodes+=a.episodes+b.episodes+c.episodes;
            double score=.45*a.reward+.25*b.reward+.30*c.reward;
            // Require the separate 720-series guard group to be robust:
            // no loss beyond modest sample noise on either hard generator,
            // and no avoidable combo missed on the guard samples.
            int robust=c.illegal==0 && c.skipped==0 &&
                c.moveLog[1]>-0.04 && c.moveLog[2]>-0.04 &&
                c.comboRate[1]<=0.002 && c.comboRate[2]<=0.002;
            if(robust && score>best+.0005){
                best=score;memcpy(next,proposal,sizeof(next));
            }
        }
        memcpy(weights,next,sizeof(weights));
        printf("ROUND %d/%d model=%d,%d,%d,%d balanced_train_score=%+.4f episodes=%d\n",
               epoch+1,rounds,weights[0],weights[1],weights[2],weights[3],best,episodes);
        fflush(stdout);
        FILE *f=fopen("v24_paired_checkpoint.tmp","w");if(!f)abort();
        fprintf(f,"%d %d %d %d\n",weights[0],weights[1],weights[2],weights[3]);
        fclose(f);
        if(rename("v24_paired_checkpoint.tmp","v24_paired_checkpoint.txt"))abort();
    }
    puts("TRAINING FINISHED. Independent holdout follows. NOT APPROVED FOR IPHONE.");
    evaluate_pair(weights,12,851,1);
    return 0;
}
