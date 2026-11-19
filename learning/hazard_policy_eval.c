// Research ONLY: paired v1.7 vs learned-risk reranking of verified legal
// three-piece plans. No iPhone integration, no knowledge of next-tray RNG.
#define BL_HAZARD_TRACE_LIBRARY 1
#include "hazard_trace_sim.c"

typedef struct { double w[10],scale[9]; } RiskModel;
static int read_values(const char *line,double *v,int count) {
    const char *p=line;
    for(int i=0;i<count;i++){
        char *end=NULL;v[i]=strtod(p,&end);
        if(end==p||!isfinite(v[i]))return 0;p=end;
    }
    while(*p==' '||*p=='\t'||*p=='\r'||*p=='\n')p++;
    return !*p;
}
static int load_model(const char *path,RiskModel *m) {
    FILE *f=fopen(path,"r");if(!f)return 0;
    char line[1024];int n=0;
    while(fgets(line,sizeof(line),f)){
        char *p=line;while(*p==' '||*p=='\t')p++;
        if(*p=='#'||*p=='\n'||*p=='\r'||!*p)continue;
        if(n==0&&!read_values(p,m->w,10)){fclose(f);return 0;}
        if(n==1&&!read_values(p,m->scale,9)){fclose(f);return 0;}
        if(++n==2)break;
    }
    fclose(f);
    if(n!=2)return 0;
    for(int i=0;i<9;i++)if(m->scale[i]<=0)return 0;
    return 1;
}
static double model_hazard(const State *g,const RiskModel *m) {
    int largest=0,pockets=0;trace_regions(g->b,&largest,&pockets);
    int blocked=trace_hard_zero(g->b);
    // Historical calibrated model scale=7 used Boolean hard_zero.
    // Corrected scale=11 uses actual count of blocked hard templates.
    double hz=(m->scale[4]==7.)?(blocked==11?1.:0.):(double)blocked;
    double x[9]={pop(g->b),countfit(g->b,0),countfit(g->b,1),
        countfit(g->b,2),hz,largest,pockets,g->streak,trace_future_clear(g->b)};
    double z=m->w[0];
    for(int j=0;j<9;j++)z+=m->w[j+1]*x[j]/m->scale[j];
    if(z>35.)z=35.;
    if(z< -35.)z= -35.;
    return 1./(1.+exp(-z));
}
typedef struct { int moves,breaks,deadlines,illegal,capped;double proxy; } Game;
static int apply_move(State *g,BLMove move,Game *result) {
    if(move.pieceIndex<0||move.pieceIndex>2||g->p[move.pieceIndex]<0||
       move.row<0||move.row>=8||move.col<0||move.col>=8||
       move.before!=g->b){result->illegal++;return -1;}
    Shape *shape=&shapes[g->p[move.pieceIndex]];
    if(move.row>8-shape->h||move.col>8-shape->w){result->illegal++;return -1;}
    U64 mask=shape->placed[move.row*8+move.col];int cleared=0;
    U64 next=place(g->b,mask,&cleared);
    if(!mask||(mask&g->b)||next!=move.after||cleared!=move.linesCleared){
        result->illegal++;return -1;
    }
    if(g->known&&g->streak>0&&g->misses==2&&cleared==0&&
       safe_clear_now(g->b,g->p))result->deadlines++;
    g->b=next;g->p[move.pieceIndex]=-1;result->moves++;
    if(cleared){g->known=1;g->streak=g->streak<63?g->streak+1:63;g->misses=0;}
    else if(g->known&&++g->misses>=3){g->streak=0;g->misses=0;result->breaks++;}
    int streak=g->streak>8?8:g->streak;
    result->proxy+=shape->n+10.*cleared*cleared*(streak?streak:1);
    return 1;
}
static int reference_plan(const State *start,BLMove result[3],State *after){
    State s=*start;Game stats={0};
    for(int k=0;k<3;k++){
        BLPiece pieces[3];trace_convert(s.p,pieces);
        BLComboContext combo={.known=s.known,.streak=s.streak,.misses=s.misses};
        BLTwoStrategies options=BLV17FindTwoStrategies(s.b,pieces,combo,180000);
        if(options.smartSurvival.movesFound<1){*after=s;return k;}
        result[k]=options.smartSurvival.moves[0];
        if(apply_move(&s,result[k],&stats)<0)return -1;
    }
    *after=s;return 3;
}
static int play_tray(State *g,int use_model,const RiskModel *model,Game *out){
    BLMove ref[3]={0};State ref_after={0};
    int n=reference_plan(g,ref,&ref_after);
    if(n<0){out->illegal++;return -1;}
    if(use_model){
        BLPiece pieces[3];trace_convert(g->p,pieces);
        BLComboContext combo={.known=g->known,.streak=g->streak,.misses=g->misses};
        BLTwoStrategies options=BLV17FindTwoStrategies(g->b,pieces,combo,180000);
        const BLSolution *candidate[3]={&options.maxLines,
            &options.keepCombo,&options.smartSurvival};
        double best=n==3?model_hazard(&ref_after,model):2.;
        int selected=-1;
        for(int k=0;k<3;k++){
            State after={0};int valid=trace_replay(g,candidate[k],&after);
            if(valid<0){out->illegal++;return -1;}
            if(valid!=1)continue;
            // Reject routes worse for an already witnessed combo.
            if(n==3&&g->known&&g->streak>0&&
               (after.streak<ref_after.streak||after.misses>ref_after.misses))
                continue;
            double risk=model_hazard(&after,model);
            if((n<3||risk+0.015<best)){best=risk;selected=k;}
        }
        if(selected>=0){
            const BLSolution *win=candidate[selected];
            for(int k=0;k<3;k++)
                if(apply_move(g,win->moves[k],out)<0)return -1;
            return 3;
        }
    }
    for(int k=0;k<n;k++)if(apply_move(g,ref[k],out)<0)return -1;
    return n;
}
static Game game(uint32_t seed,int generator,int use_model,const RiskModel *model){
    seed_rng(seed);State s={0};Game result={0};
    while(result.moves<4096){
        trace_new_tray(&s,generator);
        int status=play_tray(&s,use_model,model,&result);
        if(status<3)break;
    }
    if(result.moves>=4096)result.capped=1;
    return result;
}
typedef struct{
    double moves,proxy;int lengths[30],breaks,deadlines,illegal,capped;
} Summary;
static void add_game(Summary *s,Game g,int index){
    s->moves+=g.moves;s->proxy+=g.proxy;s->lengths[index]=g.moves;
    s->breaks+=g.breaks;s->deadlines+=g.deadlines;
    s->illegal+=g.illegal;s->capped+=g.capped;
}
static double ratio(double a,double b){return b>0?a/b:0.;}
int main(int argc,char **argv){
    if(argc!=5){fprintf(stderr,"Usage: %s model.txt epoch1 epoch2 30\n",argv[0]);return 2;}
    RiskModel model={0};
    if(!load_model(argv[1],&model)){fprintf(stderr,"Invalid research model\n");return 2;}
    char *end=NULL;
    long a=strtol(argv[2],&end,10);if(*end)return 2;
    long b=strtol(argv[3],&end,10);if(*end)return 2;
    long games=strtol(argv[4],&end,10);if(*end)return 2;
    if(a<17||a>2000||b<17||b>2000||a==b||games!=30)return 2;
    init_shapes();int pass_all=1;
    const long epochs[2]={a,b};const char *names[3]={"normal","heavy","shifted-heavy"};
    for(int i=0;i<2;i++)for(int gen=0;gen<3;gen++){
        Summary baseline={0},candidate={0};
        for(int j=0;j<30;j++){
            uint32_t seed=trace_seed((int)epochs[i],gen,j);
            add_game(&baseline,game(seed,gen,0,&model),j);
            add_game(&candidate,game(seed,gen,1,&model),j);
        }
        qsort(baseline.lengths,30,sizeof(int),intcmp);
        qsort(candidate.lengths,30,sizeof(int),intcmp);
        double survival=ratio(candidate.moves,baseline.moves);
        double score=ratio(candidate.proxy,baseline.proxy);
        double p10=ratio(candidate.lengths[2],baseline.lengths[2]);
        double break_ref=ratio(baseline.breaks,baseline.moves);
        double break_cand=ratio(candidate.breaks,candidate.moves);
        int passed=survival>=10.&&score>=10.&&p10>=10.&&
            !baseline.illegal&&!candidate.illegal&&!baseline.capped&&
            !candidate.capped&&break_cand<=break_ref+1e-12&&
            candidate.deadlines<=baseline.deadlines;
        if(!passed)pass_all=0;
        printf("HAZARD_PAIRED epoch=%ld generator=%s games=30 "
               "survival_ratio=%.5f estimated_score_ratio=%.5f p10_ratio=%.5f "
               "illegal_ref=%d illegal_candidate=%d capped_ref=%d capped_candidate=%d "
               "combo_breaks_per_move_ref=%.6f combo_breaks_per_move_candidate=%.6f "
               "avoidable_deadlines_ref=%d avoidable_deadlines_candidate=%d "
               "strict_gate=%s\n",epochs[i],names[gen],survival,score,p10,
               baseline.illegal,candidate.illegal,baseline.capped,candidate.capped,
               break_ref,break_cand,baseline.deadlines,candidate.deadlines,
               passed?"PASS":"FAIL");
        fflush(stdout);
    }
    printf("HAZARD_PAIRED_FINAL=%s - NO DEPLOY; proxy score is NOT actual game score\n",
           pass_all?"REPLICATED_STRICT_10X":"NOT_REACHED");
    return pass_all?0:1;
}
