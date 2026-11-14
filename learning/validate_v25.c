/* Final verification on unseen seeds, using 18,000-node full-tray search.
 * This process NEVER tunes model weights; research-only, no iPhone deploy.
 */
#define BL_TENX_EMBEDDED
#ifndef BL_TENX_NODE_BUDGET
#define BL_TENX_NODE_BUDGET 18000
#endif
#ifndef MAXMOVES
#define MAXMOVES 1400
#endif
#include "train_tenx.c"
void BLSetV25Weights(const int *six,int stage);
#include <math.h>
#define CAP 20
static int icmp25(const void*a,const void*b){return *(const int*)a-*(const int*)b;}
int main(int argc,char **argv){
    int count=argc>1?atoi(argv[1]):14;
    int epoch=argc>2?atoi(argv[2]):1121;
    if(count<2||count>CAP||epoch<1000||epoch>2000)return 2;
    init_shapes();
    int gamma[6]={0};
    FILE *f=fopen("v25_weights.txt","r");if(!f)return 2;
    int ok=fscanf(f,"%d %d %d %d %d %d",
        &gamma[0],&gamma[1],&gamma[2],&gamma[3],&gamma[4],&gamma[5]);
    fclose(f);if(ok!=6)return 2;
    BLSetV25Weights(gamma,6);
    const int zero[4]={0},candidate[4]={120,120,0,-20};
    printf("V25_FINAL epoch=%d games=%d cap=%d nodes=%d weights=%d,%d,%d,%d,%d,%d\n",
        epoch,count,MAXMOVES,BL_TENX_NODE_BUDGET,
        gamma[0],gamma[1],gamma[2],gamma[3],gamma[4],gamma[5]);
    int robust=1,tenx=1;
    for(int scenario=0;scenario<3;scenario++){
        double bm=0,cm=0,bs=0,cs=0,bb=0,cb=0,missB=0,missC=0;
        int invalidB=0,invalidC=0,bLengths[CAP],cLengths[CAP];
        for(int i=0;i<count;i++){
            uint32_t seed=seed_for(scenario,i,epoch);
            Result b=episode_complete(seed,scenario,zero,1);
            Result a=episode_complete(seed,scenario,candidate,0);
            bm+=b.moves;cm+=a.moves;bs+=b.proxy;cs+=a.proxy;
            bb+=b.breaks;cb+=a.breaks;missB+=b.skipped;missC+=a.skipped;
            invalidB+=b.illegal;invalidC+=a.illegal;
            bLengths[i]=b.moves;cLengths[i]=a.moves;
        }
        qsort(bLengths,count,sizeof(int),icmp25);
        qsort(cLengths,count,sizeof(int),icmp25);
        double moveRatio=cm/(bm+1e-9),scoreRatio=cs/(bs+1e-9);
        double before=100*bb/(bm+1e-9),after=100*cb/(cm+1e-9);
        int p10b=bLengths[(count-1)/10],p10c=cLengths[(count-1)/10];
        printf("WITHHELD scenario=%d baseline_avg=%.4f candidate_avg=%.4f "
               "survival_ratio=%.5f score_ratio=%.5f baseline_p10=%d candidate_p10=%d "
               "combo_breaks_per100=%.4f:%.4f avoidable_deadline_misses=%.0f:%.0f invalid=%d:%d\n",
               scenario,bm/count,cm/count,moveRatio,scoreRatio,p10b,p10c,
               before,after,missB,missC,invalidB,invalidC);
        if(moveRatio<1.02||scoreRatio<.98||p10c<p10b||
           after>before||missC>missB||invalidB||invalidC)robust=0;
        if(moveRatio<10||scoreRatio<10||p10c<10*p10b||
           after>before||missC>missB||invalidB||invalidC)tenx=0;
    }
    printf("VERDICT robust=%s ten_x=%s iphone_release=NO\n",
           robust?"PASS":"FAIL",tenx?"PASS":"FAIL");
    return 0;
}
