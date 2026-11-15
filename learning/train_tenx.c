// v2.2 Offline self-play: evolutionary tuning of the COMPLETE THREE-PIECE
// SMART SURVIVAL search. Not an iPhone runtime module.
#define MAXMOVES 1400
#define main legacy_training_main
#include "learn_combo_guarded.c"
#undef main
#include "../src/BLDualStrategy.h"
void BLSetTrainWeights(const int w[4]);
BLTwoStrategies BLV17FindTwoStrategies(uint64_t,const BLPiece[3],BLComboContext,uint32_t);

static const int zero[4]={0};
static void convert(const int src[3],BLPiece dest[3]){
    memset(dest,0,3*sizeof(BLPiece));
    for(int i=0;i<3;i++)if(src[i]>=0){
        Shape *p=&shapes[src[i]];dest[i].count=p->n;
        for(int k=0;k<p->n;k++){dest[i].x[k]=p->dx[k];dest[i].y[k]=p->dy[k];}
    }
}
static void tray_scenario(State *g,int scenario){
    new_tray(g,scenario>0);
    if(scenario==2){
        const int hard[]={4,8,10,11,21,22,23,24,25,26,27};
        for(int p=0;p<3;p++)if(random32()%4!=0)
            g->p[p]=hard[random32()%(sizeof(hard)/sizeof(hard[0]))];
    }
}
static Result episode_complete(uint32_t seed,int scenario,const int weight[4],int original){
    seed_rng(seed);State g={0};tray_scenario(&g,scenario);
    BLSetTrainWeights(weight);Result out={0};
    for(int turn=0;turn<MAXMOVES;turn++){
        BLPiece shapes3[3];convert(g.p,shapes3);
        BLComboContext combo={.known=g.known,.streak=g.streak,.misses=g.misses};
        BLTwoStrategies all=original
            ?BLV17FindTwoStrategies(g.b,shapes3,combo,180000)
            :BLFindTwoStrategies(g.b,shapes3,combo,180000);
        BLSolution plan=all.smartSurvival;
        if(!plan.movesFound)break;
        BLMove m=plan.moves[0];
        if(m.pieceIndex<0||m.pieceIndex>2||g.p[m.pieceIndex]<0||
           m.row<0||m.row>7||m.col<0||m.col>7||m.before!=g.b){out.illegal++;break;}
        Shape *piece=&shapes[g.p[m.pieceIndex]];
        U64 mask=piece->placed[m.row*8+m.col];
        int cleared=0;U64 next=place(g.b,mask,&cleared);
        if(!mask||mask&g.b||next!=m.after||cleared!=m.linesCleared){
            out.illegal++;break;
        }
        if(g.known&&g.streak>0&&g.misses==2&&cleared==0&&
           safe_clear_now(g.b,g.p))++out.skipped;
        g.b=next;g.p[m.pieceIndex]=-1;++out.moves;
        if(cleared){++out.clears;g.known=1;
            g.streak=g.streak<63?g.streak+1:63;g.misses=0;
        }else if(g.known&&++g.misses>=3){
            ++out.breaks;g.streak=0;g.misses=0;
        }
        int multiplier=g.streak>8?8:g.streak;
        out.proxy+=piece->n+10.0*cleared*cleared*(multiplier?multiplier:1);
        if(g.p[0]<0&&g.p[1]<0&&g.p[2]<0)tray_scenario(&g,scenario);
    }
    return out;
}
typedef struct {int n,illegal,p10;double moves,score,breaks,skips;} TenXStats;
static uint32_t seed_for(int scenario,int game,int epoch){
    return (uint32_t)(163000000u+20000000u*scenario+
                      (uint32_t)epoch*1000000u+(uint32_t)game*13717u);
}
static int compare_int(const void *a,const void *b){
    int x=*(const int*)a,y=*(const int*)b;return x<y?-1:x>y;
}
static TenXStats measure(const int w[4],int n,int epoch,int scenario,int original){
    TenXStats s={.n=n};
    int *lengths=calloc(n,sizeof(int));if(!lengths)abort();
    for(int i=0;i<n;i++){
        Result r=episode_complete(seed_for(scenario,i,epoch),scenario,w,original);
        s.moves+=r.moves;s.score+=r.proxy;s.breaks+=r.breaks;
        s.skips+=r.skipped;s.illegal+=r.illegal;lengths[i]=r.moves;
    }
    qsort(lengths,n,sizeof(int),compare_int);
    s.p10=lengths[(n-1)/10];free(lengths);return s;
}
static double tenx_fitness(TenXStats s,TenXStats baseline){
    // Balanced across generators: reward lower-tail robustness and penalize
    // combo deadlines instead of allowing the easy generator to dominate.
    double survival=s.moves/(baseline.moves>0?baseline.moves:1);
    double score=s.score/(baseline.score>0?baseline.score:1);
    double tail=(double)s.p10/(baseline.p10>0?baseline.p10:1);
    double breaks=100.0*s.breaks/(s.moves>0?s.moves:1);
    double oldbreaks=100.0*baseline.breaks/(baseline.moves>0?baseline.moves:1);
    return 50.0*survival+30.0*score+20.0*tail
        -40.0*fmax(0.0,breaks-oldbreaks)
        -25.0*fmax(0.0,s.skips-baseline.skips)/s.n
        -500.0*s.illegal;
}
static void metrics(const char *tag,TenXStats s){
    printf("%s games=%d avg_moves=%.3f p10=%d proxy=%.1f "
           "breaks_per_100=%.3f avoidable_combo_misses=%.0f illegal=%d\n",
           tag,s.n,s.moves/s.n,s.p10,s.score/s.n,
           100*s.breaks/(s.moves?s.moves:1),s.skips,s.illegal);
}
static int train(int argc,char **argv){
    int rounds=argc>2?atoi(argv[2]):4,seeds=argc>3?atoi(argv[3]):4;
    if(rounds<1||rounds>40||seeds<1||seeds>30)return 2;
    init_shapes();seed_rng(7238191u);
    int w[4]={0},start=0;
    FILE *f=fopen("tenx_checkpoint.txt","r");
    if(f){
        int n=fscanf(f,"%d %d %d %d %d",&start,&w[0],&w[1],&w[2],&w[3]);
        fclose(f);
        if(n!=5||start<0||start>2000)return 2;
        printf("Resuming saved search weights at epoch %d\n",start);
    }
    for(int epoch=start;epoch<start+rounds;epoch++){
        double best=-1e100;int chosen[4]={0};
        // Provisional disjoint training epoch namespace; historical ledger
        // must be audited before claiming these seeds are never used.
        const int training_seed_epoch=1700+epoch;
        TenXStats reference[3];
        for(int scenario=0;scenario<3;scenario++)
            reference[scenario]=measure(zero,seeds,training_seed_epoch,scenario,1);
        for(int attempt=0;attempt<7;attempt++){
            int trial[4];memcpy(trial,w,sizeof(trial));
            if(attempt){
                int idx=(int)(random32()%4),delta[]={-120,-60,-30,30,60,120};
                trial[idx]+=delta[random32()%6];
                if(trial[idx]>250)trial[idx]=250;
                if(trial[idx]<-250)trial[idx]=-250;
            }
            double score=0;
            for(int scenario=0;scenario<3;scenario++){
                TenXStats s=measure(trial,seeds,training_seed_epoch,scenario,0);
                score+=tenx_fitness(s,reference[scenario]);
            }
            if(score>best){best=score;memcpy(chosen,trial,sizeof(chosen));}
        }
        memcpy(w,chosen,sizeof(w));
        FILE *checkpoint=fopen("tenx_checkpoint.pending","w");if(!checkpoint)abort();
        fprintf(checkpoint,"%d %d %d %d %d\n",epoch+1,w[0],w[1],w[2],w[3]);
        fclose(checkpoint);
        if(rename("tenx_checkpoint.pending","tenx_checkpoint.txt"))abort();
        printf("epoch=%d weights=%d,%d,%d,%d training_fitness=%.2f\n",
               epoch+1,w[0],w[1],w[2],w[3],best);fflush(stdout);
    }
    f=fopen("tenx_candidate_weights.txt","w");if(!f)abort();
    fprintf(f,"%d %d %d %d\n",w[0],w[1],w[2],w[3]);fclose(f);
    return 0;
}
static int test(int argc,char **argv){
    int n=argc>2?atoi(argv[2]):24,epoch=argc>4?atoi(argv[4]):92;
    if(n<10||n>1000||epoch<0||epoch>2000)return 2;
    FILE *f=fopen(argc>3?argv[3]:"tenx_candidate_weights.txt","r");
    if(!f)return 2;
    int w[4]={0},got=fscanf(f,"%d %d %d %d",&w[0],&w[1],&w[2],&w[3]);
    fclose(f);if(got!=4)return 2;
    init_shapes();int passed=1;
    printf("SYNTHETIC TEN_X TEST: %d unseen games/scenario, epoch=%d, "
           "max_moves=%d. NOT the real game's scoring.\n",n,epoch,MAXMOVES);
    for(int scenario=0;scenario<3;scenario++){
        TenXStats old=measure(zero,n,epoch,scenario,1);
        TenXStats challenger=measure(w,n,epoch,scenario,0);
        metrics(scenario==0?"v17_normal":scenario==1?"v17_stress":"v17_shifted",old);
        metrics(scenario==0?"candidate_normal":scenario==1?"candidate_stress":"candidate_shifted",challenger);
        if(old.moves<=0||old.score<=0||old.p10<=0||old.illegal||challenger.illegal){
            fprintf(stderr,"Invalid baseline/result. Fail closed.\n");return 3;
        }
        double survival=challenger.moves/old.moves;
        double points=challenger.score/old.score;
        printf("scenario=%d survival_ratio=%.4f score_ratio=%.4f p10_ratio=%.4f\n",
               scenario,survival,points,(double)challenger.p10/old.p10);
        if(!isfinite(survival)||!isfinite(points)||
           survival<10.0||points<10.0||challenger.p10<10*old.p10||
           challenger.breaks/(challenger.moves+1e-9)>
             old.breaks/(old.moves+1e-9)||challenger.skips>old.skips)passed=0;
    }
    printf("TEN_X_VERDICT=%s\n",passed?"ALL HELDOUT GATES PASSED":"NOT REACHED - NO DEPLOY");
    return 0;
}
int main(int argc,char **argv){
    if(argc<2){fprintf(stderr,"Usage: %s train [rounds] [seeds] | "
            "test [games] [weights.txt] [withheld_epoch]\n",argv[0]);return 2;}
    if(strcmp(argv[1],"train")==0)return train(argc,argv);
    if(strcmp(argv[1],"test")==0)return test(argc,argv);
    return 2;
}
