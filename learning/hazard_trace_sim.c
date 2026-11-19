// Research ONLY: legal completed-tray afterstates and three-tray v1.7 continuations.
// Synthetic simulator, not the real game's piece distribution or scoring.
// Build with v17.o/solver.o as in the existing bounded research workflow.
#define MAXMOVES 4096
#define main learning_lab_unused_main
#include "learn_combo_guarded.c"
#undef main
#include "../src/BLDualStrategy.h"
BLTwoStrategies BLV17FindTwoStrategies(uint64_t,const BLPiece[3],BLComboContext,uint32_t);

static void trace_convert(const int from[3], BLPiece to[3]) {
    memset(to,0,sizeof(BLPiece)*3);
    for(int i=0;i<3;i++) if(from[i]>=0) {
        Shape *p=&shapes[from[i]];to[i].count=p->n;
        for(int j=0;j<p->n;j++){to[i].x[j]=p->dx[j];to[i].y[j]=p->dy[j];}
    }
}
static void trace_new_tray(State *g,int scenario) {
    new_tray(g,scenario>0);
    if(scenario==2){
        const int hard[]={4,8,10,11,21,22,23,24,25,26,27};
        for(int p=0;p<3;p++)if(random32()%4!=0)
            g->p[p]=hard[random32()%(sizeof(hard)/sizeof(hard[0]))];
    }
}
static uint32_t trace_seed(int epoch,int scenario,int game) {
    return (uint32_t)(163000000u+20000000u*(uint32_t)scenario+
            (uint32_t)epoch*1000000u+(uint32_t)game*13717u);
}
// Exactly one validated legal v1.7 move, preserving the simulator's combo state.
static int trace_play(State *g) {
    BLPiece three[3];trace_convert(g->p,three);
    BLComboContext combo={.known=g->known,.streak=g->streak,.misses=g->misses};
    BLTwoStrategies strategies=BLV17FindTwoStrategies(g->b,three,combo,180000);
    BLSolution plan=strategies.smartSurvival;
    if(!plan.movesFound)return 0;
    BLMove m=plan.moves[0];
    if(m.pieceIndex<0||m.pieceIndex>2||g->p[m.pieceIndex]<0||
       m.row<0||m.row>7||m.col<0||m.col>7||m.before!=g->b) return -1;
    Shape *piece=&shapes[g->p[m.pieceIndex]];
    U64 mask=piece->placed[m.row*8+m.col];
    int cleared=0;
    U64 next=place(g->b,mask,&cleared);
    if(!mask||(mask&g->b)||next!=m.after||cleared!=m.linesCleared)return -1;
    g->b=next;
    g->p[m.pieceIndex]=-1;
    if(cleared){g->known=1;g->streak=g->streak<63?g->streak+1:63;g->misses=0;}
    else if(g->known&&++g->misses>=3){g->streak=0;g->misses=0;}
    return 1;
}
static int trace_one_tray(State *g,int scenario) {
    trace_new_tray(g,scenario);
    for(int k=0;k<3;k++){int ok=trace_play(g);if(ok!=1)return ok;}
    if(g->p[0]!=-1||g->p[1]!=-1||g->p[2]!=-1)return -1;
    return 1;
}
static void trace_regions(U64 b,int *largest,int *pockets) {
    U64 seen=0;int components=0;
    *largest=0;
    for(int start=0;start<64;start++){
        U64 first=UINT64_C(1)<<start;
        if((b|seen)&first)continue;
        components++;int queue[64],head=0,tail=0,count=0;
        queue[tail++]=start;seen|=first;
        while(head<tail){
            int i=queue[head++],r=i/8,c=i%8;count++;
            const int nbr[4]={i-8,i+8,i-1,i+1};
            for(int k=0;k<4;k++) {
                if((k==0&&r==0)||(k==1&&r==7)||
                   (k==2&&c==0)||(k==3&&c==7))continue;
                U64 bit=UINT64_C(1)<<nbr[k];
                if(!((b|seen)&bit)){seen|=bit;queue[tail++]=nbr[k];}
            }
        }
        if(count>*largest)*largest=count;
    }
    *pockets=components>0?components-1:0;
}
static int trace_hard_zero(U64 board){
    const int hard[]={4,8,10,11,21,22,23,24,25,26,27};
    for(unsigned k=0;k<sizeof(hard)/sizeof(hard[0]);k++)
        if(options_on(board,hard[k])>0)return 0;
    return 1;
}
static int trace_future_clear(U64 board){
    int n=0;
    for(int j=0;j<N_SHAPES;j++){
        Shape *p=&shapes[j];
        for(int r=0;r<=8-p->h;r++)for(int c=0;c<=8-p->w;c++){
            U64 mask=p->placed[r*8+c];
            if(board&mask)continue;
            int lines=0;(void)place(board,mask,&lines);
            if(lines&&++n>=32)return 32;
        }
    }
    return n;
}
static int trace_label(State after,int scenario,int completed[3]){
    uint32_t old_rng=rng;
    for(int t=0;t<3;t++){
        int result=trace_one_tray(&after,scenario);
        if(result<0){rng=old_rng;return -1;}
        completed[t]=(result==1);
        if(result==0)break;
    }
    rng=old_rng;
    return 0;
}
static int trace_emit(FILE *out,const State *g,int epoch,int scenario,
                      int game,int tray,int finalist,const char *split) {
    int largest=0,pockets=0,completed[3]={0,0,0};
    trace_regions(g->b,&largest,&pockets);
    if(trace_label(*g,scenario,completed)<0)return -1;
    static const char *names[]={"normal","heavy","shifted-heavy"};
    fprintf(out,"{\"epoch\":%d,\"scenario\":\"%s\",\"game\":%d,\"tray\":%d,"
        "\"finalist\":%d,\"split\":\"%s\",\"legal\":true,\"features\":{"
        "\"occupied\":%d,\"square_fits\":%d,\"hbar_fits\":%d,"
        "\"vbar_fits\":%d,\"hard_zero\":%d,\"largest_free\":%d,"
        "\"pockets\":%d,\"combo_streak\":%d,\"future_clear\":%d},"
        "\"continuation\":[",epoch,names[scenario],game,tray,finalist,split,
        pop(g->b),countfit(g->b,0),countfit(g->b,1),countfit(g->b,2),
        trace_hard_zero(g->b),largest,pockets,g->streak,trace_future_clear(g->b));
    for(int i=0;i<3;i++){
        if(i)fputc(',',out);
        fputs(completed[i]?"{\"completed\":true}":"{\"completed\":false}",out);
        if(!completed[i])break;
    }
    fputs("]}\n",out);
    return 0;
}
// Replay ONLY a complete three-piece solution certified by the solver.
// Verify every move and the resulting combo state against the simulator.
static int trace_replay(const State *before,const BLSolution *plan,State *after) {
    if(plan->movesFound!=3)return 0;
    *after=*before;
    for(int i=0;i<3;i++){
        BLMove m=plan->moves[i];
        if(m.pieceIndex<0||m.pieceIndex>2||after->p[m.pieceIndex]<0||
           m.row<0||m.row>7||m.col<0||m.col>7||m.before!=after->b)return -1;
        Shape *shape=&shapes[after->p[m.pieceIndex]];
        U64 mask=shape->placed[m.row*8+m.col];
        int cleared=0;U64 next=place(after->b,mask,&cleared);
        if(!mask||(mask&after->b)||next!=m.after||
           cleared!=m.linesCleared)return -1;
        after->b=next;after->p[m.pieceIndex]=-1;
        if(cleared){after->known=1;
            after->streak=after->streak<63?after->streak+1:63;
            after->misses=0;
        }else if(after->known&&++after->misses>=3){
            after->streak=0;after->misses=0;
        }
    }
    return after->p[0]==-1&&after->p[1]==-1&&after->p[2]==-1?1:-1;
}
static int trace_same_after(const State *a,const State *b) {
    return a->b==b->b && a->known==b->known &&
           a->misses==b->misses && a->streak==b->streak;
}
static int trace_batch(FILE *out,int epoch,const char *split,int games) {
    for(int scenario=0;scenario<3;scenario++)for(int game=0;game<games;game++){
        seed_rng(trace_seed(epoch,scenario,game));State g={0};
        // Prove counterfactual legal completed-tray finalists from the same
        // observable three-piece tray. Run the actual reference policy on its
        // original trajectory; alternative labels use identical future RNG
        // starts, restored after each continuation. This is NOT online play.
        for(int tray=0;tray<80;tray++) {
            trace_new_tray(&g,scenario);
            State before=g;
            const int sample=(tray<4 || tray%2==1);
            BLTwoStrategies strategies={0};
            if(sample){
                BLPiece three[3];trace_convert(before.p,three);
                BLComboContext combo={.known=before.known,
                    .streak=before.streak,.misses=before.misses};
                strategies=BLV17FindTwoStrategies(before.b,three,combo,180000);
            }
            int status=1;
            for(int move=0;move<3;move++) {
                status=trace_play(&g);
                if(status!=1)break;
            }
            if(status<0)return -1;
            if(status==0)break;
            if(!sample)continue;
            // The dynamically receding v1.7 reference path is finalist 0.
            // Alternative finalists are exact legal paths supplied by v1.7's
            // independent MaxLines/KeepCombo/SmartSurvival objectives.
            if(trace_emit(out,&g,epoch,scenario,game,tray,0,split)<0)return -1;
            const BLSolution *alternatives[3]={
                &strategies.maxLines,&strategies.keepCombo,
                &strategies.smartSurvival};
            State accepted[3];int acceptedN=0;
            for(int j=0;j<3;j++){
                State alt={0};
                int replay=trace_replay(&before,alternatives[j],&alt);
                if(replay<0)return -1;
                if(replay==0||trace_same_after(&alt,&g))continue;
                int duplicate=0;
                for(int k=0;k<acceptedN;k++)
                    if(trace_same_after(&alt,&accepted[k]))duplicate=1;
                if(duplicate)continue;
                accepted[acceptedN++]=alt;
                if(trace_emit(out,&alt,epoch,scenario,game,tray,
                              acceptedN,split)<0)return -1;
            }
        }
    }
    return 0;
}
int main(int argc,char **argv) {
    if(argc!=5){fprintf(stderr,"Usage: %s train_epoch validation_epoch games output.jsonl\n",argv[0]);return 2;}
    char *end=NULL;
    long train=strtol(argv[1],&end,10);if(*end)return 2;
    long valid=strtol(argv[2],&end,10);if(*end)return 2;
    long games=strtol(argv[3],&end,10);if(*end)return 2;
    if(train<17||train>2000||valid<17||valid>2000||train==valid||
       games<1||games>20)return 2;
    FILE *out=fopen(argv[4],"w");if(!out){perror(argv[4]);return 2;}
    init_shapes();
    int ok=trace_batch(out,(int)train,"train",(int)games)==0 &&
           trace_batch(out,(int)valid,"validation",(int)games)==0;
    if(fclose(out)!=0)ok=0;
    if(!ok){fprintf(stderr,"Hazard trace generation aborted: illegal move or I/O failure\n");return 3;}
    puts("HAZARD_TRACE_SIM=COMPLETE synthetic legal afterstates, v1.7 continuation labels; no hidden RNG at inference");
    return 0;
}
