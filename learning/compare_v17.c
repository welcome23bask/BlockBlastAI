/* Run exactly the SAME synthetic piece stream through the current
   production v1.7 SMART SURVIVAL planner and the trained one-step learner.
   This is an apples-to-apples SIMULATOR comparison, not real app gameplay. */
#define main learning_main
#include "learn_combo_guarded.c"
#undef main
#include "../src/BLDualStrategy.h"

static void converted(const int source[3],BLPiece result[3]) {
    memset(result,0,3*sizeof(BLPiece));
    for(int i=0;i<3;i++)if(source[i]>=0) {
        Shape *src=&shapes[source[i]];
        result[i].count=src->n;
        for(int j=0;j<src->n;j++) {
            result[i].x[j]=src->dx[j];
            result[i].y[j]=src->dy[j];
        }
    }
}
static Result baseline_episode_v17(uint32_t seed,int stress){
    seed_rng(seed);State g={0};new_tray(&g,stress);Result out={0};
    for(int k=0;k<MAXMOVES;k++){
        BLPiece pieces[3]={0};converted(g.p,pieces);
        BLComboContext context={.streak=g.streak,.misses=g.misses,.known=g.known};
        BLTwoStrategies solution=BLFindTwoStrategies(g.b,pieces,context,180000);
        BLSolution sol=solution.smartSurvival;
        if(sol.movesFound<=0)break;
        BLMove m=sol.moves[0];
        if(m.pieceIndex<0||m.pieceIndex>2||
           g.p[m.pieceIndex]<0||m.before!=g.b ||
           m.row<0||m.row>7||m.col<0||m.col>7){
            out.illegal++;break;
        }
        Shape *piece=&shapes[g.p[m.pieceIndex]];
        U64 mask=piece->placed[m.row*8+m.col];
        if(!mask|| (mask & g.b)){out.illegal++;break;}
        int cleared=0;
        U64 after=place(g.b,mask,&cleared);
        if(after!=m.after||cleared!=m.linesCleared){out.illegal++;break;}
        if(g.known&&g.streak>0&&g.misses==2&&cleared==0 &&
           safe_clear_now(g.b,g.p))out.skipped++;
        g.b=after;g.p[m.pieceIndex]=-1;
        ++out.moves;
        if(cleared>0){
            ++out.clears;g.known=1;g.streak=g.streak<63?g.streak+1:63;g.misses=0;
        }else if(g.known){
            ++g.misses;
            if(g.misses>=3){++out.breaks;g.streak=0;g.misses=0;}
        }
        int mult=g.streak>8?8:g.streak;
        out.proxy+=piece->n+10.0*cleared*cleared*(mult?mult:1);
        if(g.p[0]<0&&g.p[1]<0&&g.p[2]<0)new_tray(&g,stress);
    }
    return out;
}
int main(int argc,char **argv){
    int normal=argc>1?intarg(argv[1],1,200):24;
    int stress=argc>2?intarg(argv[2],1,200):24;
    init_shapes();
    // This build is intentionally only a reference policy comparison; the
    // learned weights come from the train workflow artifact separately.
    // Parse the 12 weights printed by the trainer's machine-readable JSON.
    const char *filename=argc>3?argv[3]:"learned_combo_weights.json";
    FILE *in=fopen(filename,"r");
    if(!in){perror(filename);return 2;}
    char bytes[8192]={0};size_t n=fread(bytes,1,sizeof(bytes)-1,in);fclose(in);
    bytes[n]=0;
    char *start=strstr(bytes,"\"weights\":");
    if(!start){fprintf(stderr,"No weights key in model file\n");return 2;}
    start=strchr(start,'[');
    if(!start)return 2;
    ++start;
    double learned[F]={0};
    for(int j=0;j<F;j++){
        char *end=NULL;learned[j]=strtod(start,&end);
        if(start==end||!isfinite(learned[j]))return 2;
        start=end;while(*start==' '||*start==',')++start;
    }
    printf("Comparing SMART SURVIVAL with offline trained policy; "
           "all decisions independently checked for legal board transitions.\n");
    for(int scenario=0;scenario<2;scenario++){
        int games=scenario?stress:normal;
        long oldMoves=0,newMoves=0,oldBreaks=0,newBreaks=0,oldSkips=0,newSkips=0;
        double oldScore=0,newScore=0;
        int *oldSorted=malloc(games*sizeof(int)),*newSorted=malloc(games*sizeof(int));
        for(int i=0;i<games;i++) {
            uint32_t seed=(scenario?145000000u:135000000u)+i*3137u;
            Result old=baseline_episode_v17(seed,scenario);
            guardActiveCombo=1;
            Result fresh=episode(learned,seed,scenario);
            if(old.illegal||fresh.illegal){
                fprintf(stderr,"Illegal move in pair %d: %d / %d\n",
                        i,old.illegal,fresh.illegal);return 1;
            }
            oldMoves+=old.moves;newMoves+=fresh.moves;
            oldBreaks+=old.breaks;newBreaks+=fresh.breaks;
            oldSkips+=old.skipped;newSkips+=fresh.skipped;
            oldScore+=old.proxy;newScore+=fresh.proxy;
            oldSorted[i]=old.moves;newSorted[i]=fresh.moves;
        }
        qsort(oldSorted,games,sizeof(int),intcmp);
        qsort(newSorted,games,sizeof(int),intcmp);
        printf("%s: games=%d v17_avg_moves=%.3f trained_avg_moves=%.3f "
               "v17_p10=%d trained_p10=%d "
               "v17_score_proxy=%.1f trained_score_proxy=%.1f "
               "v17_breaks_per_100_moves=%.2f trained_breaks_per_100_moves=%.2f "
               "v17_avoidable_deadline_skips=%ld trained_avoidable_deadline_skips=%ld\n",
               scenario?"LARGE_PIECES":"MIXED_PIECES",games,
               (double)oldMoves/games,(double)newMoves/games,
               oldSorted[(games-1)/10],newSorted[(games-1)/10],
               oldScore/games,newScore/games,
               100.0*oldBreaks/(oldMoves?oldMoves:1),
               100.0*newBreaks/(newMoves?newMoves:1),
               oldSkips,newSkips);
        free(oldSorted);free(newSorted);
    }
    return 0;
}
