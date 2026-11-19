// Independent local two-objective solver. The reference website's proprietary
// engine is not included or called: this enumerates legal three-piece routes
// under the existing 8x8 row/column clear model.
#include "BLDualStrategy.h"
#include <string.h>
#include <limits.h>
// The fair-root hypothesis was falsified on shifted-heavy holdouts 1943/1944.
// Keep it available for controlled A/B studies but default to the better
// characterized original research traversal; never alter src/ v1.7 baseline.
#ifndef BL_RESEARCH_FAIR_ROOT
#define BL_RESEARCH_FAIR_ROOT 0
#endif
// Explore how two *unknown future* large shapes could coexist after a tray.
// This is a conservative geometric proxy, not knowledge of the next tray.
#ifndef BL_RESEARCH_PAIR_LOOKAHEAD
#define BL_RESEARCH_PAIR_LOOKAHEAD 1
#endif
// Three fixed next-tray rollouts overfit/shifted heavy on holdouts
// 1953/1954. Retain for explicit research A/B but default OFF.
// Independent v1.7 source and phone runtime remain untouched.
#ifndef BL_RESEARCH_NEXT_TRAY_ROLLOUT
#define BL_RESEARCH_NEXT_TRAY_ROLLOUT 0
#endif
// Iteration 36's minimax over the single rarest future hard piece improved
// normal survival, but heavy survival remained only ~1.12x on holdout.
// Restore as the last combo-safe characterized hypothesis for iteration 38.
#ifndef BL_RESEARCH_MIN_HARD_COVERAGE
#define BL_RESEARCH_MIN_HARD_COVERAGE 0
#endif
// Iteration 37 (rejected on shifted-heavy combo rate): reward the bottom-three hard-piece landing counts DURING
// every explored current-tray afterstate, before the top-four finalist filter.
// Unlike single-shape minimax, this trades off several hard shapes and can
// select a less immediately pretty board that preserves multiple exits.
#ifndef BL_RESEARCH_BOTTOM3_HARD_COVERAGE
#define BL_RESEARCH_BOTTOM3_HARD_COVERAGE 0
#endif
// Iteration 38: protect proactive combo rescue with a hard-future-pair
// Pareto condition. Only override a large immediate-utility shortfall if
// a fully legal combo-saving tray route does NOT lose pair feasibility.
// This rejects the iteration-37 bottom-three aggregate which worsened
// shifted-heavy combo breaks on holdout 1957. Research-only, not iPhone.
#ifndef BL_RESEARCH_PROACTIVE_PARETO
#define BL_RESEARCH_PROACTIVE_PARETO 0
#endif
// Iteration 39: preserve additional complete-tray finalists selected on
// risk-adjusted *hard-shape* coverage, in parallel to ordinary quality.
// The late future-pair ranking can now consider genuinely different legal
// afterstates that a top-four pure-quality pool would have discarded.
// No new future RNG access, no extra search nodes, no baseline changes.
#ifndef BL_RESEARCH_DUAL_POOL
#define BL_RESEARCH_DUAL_POOL 0
#endif
// Iteration 40: conservative *baseline-anchored feasibility shield*.
// On cramped boards, a lower-risk full three-piece plan from the completely
// unchanged v1.7 solver may replace a candidate plan ONLY if it does not
// worsen a known combo deadline. This is a research-only ensemble decision,
// not a modification to v1.7, not access to hidden game state.
#ifndef BL_RESEARCH_BASELINE_RISK_SHIELD
#define BL_RESEARCH_BASELINE_RISK_SHIELD 0
#endif
// Iteration 41: distributionally robust future-piece accessibility across
// all 28 public synthetic laboratory shape templates. Assess only already
// proven legal current-tray finalists, NOT hidden next-tray RNG samples.
#ifndef BL_RESEARCH_SHAPE_MIXTURE
#define BL_RESEARCH_SHAPE_MIXTURE 0
#endif
// Iteration 42: future single-piece LINE CLEAR accessibility, not just raw
// landing-space accessibility. This counts exactly legal immediate clears by
// representative next shapes from already completed current-tray boards;
// never samples or peeks at the game's hidden next-tray RNG.
#ifndef BL_RESEARCH_FUTURE_CLEAR_ACCESS
#define BL_RESEARCH_FUTURE_CLEAR_ACCESS 1
#endif
// Iteration 43: lower-tail hard-shape accessibility floor combined with
// exact one-piece clear access; unlike the rejected global minimax bonus,
// only penalize *zero* hard-shape landing on cramped completed trays.
#ifndef BL_RESEARCH_HARD_ZERO_FLOOR
#define BL_RESEARCH_HARD_ZERO_FLOOR 0
#endif
// Iteration 44: favor a single large contiguous empty region over multiple
// small disconnected pockets ONLY on relatively crowded afterstates. This
// complements the promising future single-piece clear-access feature and
// avoids speculative hidden future trays. Research-only late rerank.
#ifndef BL_RESEARCH_CONNECTED_FREE_REGION
#define BL_RESEARCH_CONNECTED_FREE_REGION 0
#endif
// Iteration 45: reward robust *two-orientation escape corridors* on crowded
// completed trays. This is not the rejected global hard-piece minimum: it
// rewards the weaker of horizontal/vertical five-bar capacities, only if
// BOTH orientations still have legal landings. No future RNG is consulted.
#ifndef BL_RESEARCH_ORTHOGONAL_ESCAPE
#define BL_RESEARCH_ORTHOGONAL_ESCAPE 0
#endif
// Iteration 46: reward *diversity of heavy-shape immediate clear routes* on
// crowded complete afterstates, rather than horizontal/vertical bar capacity.
// Only exact legal hypothetical one-piece placements; no next-tray RNG.
#ifndef BL_RESEARCH_HARD_CLEAR_DIVERSITY
#define BL_RESEARCH_HARD_CLEAR_DIVERSITY 0
#endif
// Iteration 47: lower-tail rescue feasibility: after an exactly legal
// hypothetical difficult-piece placement that clears a line, can the board
// still accommodate a second difficult piece? This measures recoverable
// TWO-step capacity, unlike iteration 46's count of immediate clear types.
// Only public shapes and exact board geometry; no hidden future RNG.
#ifndef BL_RESEARCH_HARD_RESCUE_CHAIN
#define BL_RESEARCH_HARD_RESCUE_CHAIN 0
#endif
// Iteration 48: lower-tail geometric rescue diversity across BOTH clear axes.
// The prior two-step chain rewarded any rescue but shifted-heavy survival
// remained near 1.1x. This instead scores the weaker of row-clearing and
// column-clearing hard-shape type diversity, using only exact legal placements.
// No next-tray RNG, no hidden piece order, no baseline or phone changes.
#ifndef BL_RESEARCH_BIAXIAL_HARD_CLEAR
#define BL_RESEARCH_BIAXIAL_HARD_CLEAR 0
#endif
#define BL_RESEARCH_REDUNDANT_HARD_LANDINGS 0
// Iteration 50: exact non-overlapping 3x3 landing pairs, not individual landing counts.
#define BL_RESEARCH_DISJOINT_SQUARE_PAIR 0
// Iteration 51: mobility coverage of free cells by diverse exact legal bulky footprints.
#define BL_RESEARCH_FREE_CELL_MOBILITY 0
// Iteration 52: reward accessible nearly-complete row/column repair lines, not bulky-footprint coverage.
#define BL_RESEARCH_LINE_REPAIR_POTENTIAL 0
// Iteration 53: exact single-piece repair feasibility; unlike union-cell reachability,
// every missing line cell must be filled by ONE legal placement.
#define BL_RESEARCH_EXACT_REPAIR 0
// Iteration 54: evaluate actual next-one-piece clears by how much 3x3 mobility they restore.
#define BL_RESEARCH_POST_CLEAR_SQUARE_RECOVERY 0
// Iteration 55: future legal small-piece clear must recover the weakest hard-shape access,
// not merely 3x3 square mobility; bounded public-geometry afterstate test only.
#define BL_RESEARCH_POST_CLEAR_HARD_RESCUE 0
// Iteration 56: reward hard-shape placements that preserve a DIFFERENT hard shape's legal fit.
// Exact deterministic one-piece hypothetical, no hidden future RNG.
#define BL_RESEARCH_HARD_CROSS_FIT 0
// Iteration 57: worst-family *immediate line-clear* continuation among hard shapes.
// Unlike cross-fit, a non-clearing first hard piece cannot earn a reward.
#define BL_RESEARCH_HARD_CLEAR_TAIL 0
// Iteration 58: penalize shared critical landing cells across all hard families.
// A single contested empty pocket is a brittle common-mode failure point.
#define BL_RESEARCH_HARD_LANDING_BOTTLENECK 0
// Iteration 61: reward independent 3x3 landing zones instead of counting
// overlapping placements as separate escape routes. Research-only.
#define BL_RESEARCH_INDEPENDENT_SQUARE_ZONES 0
// Iteration 65: exact three-hard-piece continuation across all six orders,
// rewarding real line clears rather than static square/bar co-location.
#define BL_RESEARCH_CROSS_FAMILY_DISJOINT 0
// Iteration 69: protect the two weakest of three hard-piece landing families
// only on crowded full-tray boards. Unlike exact sequential three-piece
// continuation, this bounded single-step lower-tail reserve neither
// presumes all three next pieces nor rewards one abundant orientation.
#define BL_RESEARCH_CROWDED_HARD_FLOOR 1
// Iteration 77: conditional two-piece hard-pair failure floor. The prior
// exact one-piece rescue improved no strict 10x gates; on crowded completed
// trays, penalize a board when NONE of three distinct hard-piece pair
// families has an exact legal two-piece continuation. Unlike static landing
// counts, this uses legal sequential placements and intervening line clears.
// The tested shapes are fixed public geometry, not future RNG observations.
#define BL_RESEARCH_CROWDED_PAIR_FAILURE 1
// Research-only eight-finalist ablation after failed six-finalist holdouts 78/79:
// retain two additional distinct exactly legal complete-tray afterstates.
// Hypothesis: shifted-heavy early deaths arise from a narrow candidate pool
// excluding rare irregular-shape escape routes before late reranking.
// Fixed deterministic observable geometry only; no hidden future RNG.
// No baseline, validated checkpoint, or live iPhone changes.
#define BL_FUTURE_TOP 8
// v2.2 RESEARCH-ONLY global trainable board-utility features. No iPhone
// source changed. Set to all-zero coefficients for the v1.7 baseline.
static int blTrainWeights[4]={0};
void BLSetTrainWeights(const int *w){
    for(int i=0;i<4;i++)blTrainWeights[i]=w[i];
}

typedef struct { uint64_t mask; unsigned char row,col; } BLPlace;
typedef struct {
    uint64_t board;
    int depth,used,lines,firstClear,misses,streak,broken,seenClear;
    BLMove moves[3];
} BLPath;
typedef struct {
    BLPlace placements[3][64];
    int counts[3],total;
    const BLPiece *pieces;
    BLComboContext initial;
    BLPath maxPath,comboPath,smartPath,smartComboPath;
    int maxQuality,comboQuality,smartQuality,smartComboQuality;
    int haveMax,haveCombo,haveSmart,haveSmartCombo;
    uint32_t budget,visited;
    int truncated;
    // Four diverse completed-tray afterstates per objective. Full search
    // still determines legal plans; future stress is only a late reranker.
    BLPath topAll[BL_FUTURE_TOP],topCombo[BL_FUTURE_TOP];
    int topAllQ[BL_FUTURE_TOP],topComboQ[BL_FUTURE_TOP];
    int topAllN,topComboN;
#if BL_RESEARCH_DUAL_POOL && BL_RESEARCH_MIN_HARD_COVERAGE
    // Separate risk-oriented finalist diversity; each stored path is an
    // already proven current-tray legal route, never an imaginary rollout.
    BLPath topRisk[BL_FUTURE_TOP],topComboRisk[BL_FUTURE_TOP];
    int topRiskQ[BL_FUTURE_TOP],topComboRiskQ[BL_FUTURE_TOP];
    int topRiskN,topComboRiskN;
#endif
} BLSearch;
#if BL_RESEARCH_BASELINE_RISK_SHIELD
// Only the research runner links the unchanged v1.7 strategy separately as
// v17.o, with BLFindTwoStrategies renamed at compile time. Never link this
// opt-in experiment into live v3.4 iPhone builds.
BLTwoStrategies BLV17FindTwoStrategies(uint64_t,const BLPiece[3],
                                      BLComboContext,uint32_t);
static int plan_keeps_existing_combo(const BLSolution *plan,
                                     BLComboContext combo) {
    if(!combo.known || combo.streak<=0)return 1;
    int misses=combo.misses>=0&&combo.misses<3?combo.misses:0;
    for(int i=0;i<plan->movesFound;++i){
        if(plan->moves[i].linesCleared>0)misses=0;
        else if(++misses>=3)return 0;
    }
    return 1;
}
#endif
static int bits(uint64_t v) {return __builtin_popcountll(v);}
static uint64_t clear_full(uint64_t after,int *lines) {
    uint64_t erase=0;
    *lines=0;
    for(int r=0;r<8;++r) {
        uint64_t m=UINT64_C(255)<<(r*8);
        if((after&m)==m) {erase|=m;++*lines;}
    }
    for(int c=0;c<8;++c) {
        uint64_t m=UINT64_C(0x0101010101010101)<<c;
        if((after&m)==m) {erase|=m;++*lines;}
    }
    return after&~erase;
}
static int fit(uint64_t board,uint64_t mask,int w,int h) {
    int n=0;
    for(int r=0;r<=8-h;++r)
        for(int c=0;c<=8-w;++c)
            n+=!(board&(mask<<(r*8+c)));
    return n;
}
// Runs only when a candidate ties or improves a primary objective. Favors
// useful empty landing areas instead of merely unfilled cell count.
static int future_quality(uint64_t board) {
    int free=64-bits(board);
    int square=fit(board,UINT64_C(0x70707),3,3);
    int hbar=fit(board,UINT64_C(0x1f),5,1);
    int vbar=fit(board,UINT64_C(0x0101010101),1,5);
    int rect1=fit(board,UINT64_C(0x707),3,2);
    int rect2=fit(board,UINT64_C(0x30303),2,3);
    int rough=0;
    for(int r=0;r<8;++r) {
        uint64_t row=(board>>(r*8))&255;
        for(int c=1;c<8;++c)
            rough+=((row>>(c-1))&1)!=((row>>c)&1);
    }
    // Avoid rewarding a large number of possible 1x5 locations at the cost of
    // entirely losing all 3x3 and 2x3 positions when the board gets cramped.
    int missing=(square==0)*2200+(hbar==0)*600+(vbar==0)*600;
    return free*85+square*72+hbar*15+vbar*15+
           (rect1+rect2)*28-rough*12-missing;
}
// A position can be mostly empty but still unable to accept a 3x3 square
// or a five-long bar. Add a quantified cost for disconnected pockets and
// reward *different* kinds of usable future piece footprints.
// A bounded robust proxy: prioritize the least-placeable of seven irregular
// future pieces. This sees only public board geometry, never hidden RNG.
static int worst_hard_coverage(uint64_t board) {
    static const struct {uint64_t mask;int w,h;} shapes[7]={
        {UINT64_C(0x40603),3,3},{UINT64_C(0x507),3,2},
        {UINT64_C(0x705),3,2},{UINT64_C(0x701),3,2},
        {UINT64_C(0x704),3,2},{UINT64_C(0x20303),3,3},
        {UINT64_C(0x20702),3,3}
    };
    int minimum=64;
    for(int i=0;i<7;++i){
        int n=fit(board,shapes[i].mask,shapes[i].w,shapes[i].h);
        if(n<minimum)minimum=n;
    }
    return minimum>6?6:minimum;
}
// Geometry-only mixture of synthetic normal/heavy/shifted-heavy shape FAMILIES.
// Pattern masks/width/height match learning/learn_combo_guarded.c's 28 templates.
// This does not sample or observe the next RNG state, and does NOT model real
// Block Blast spawn probabilities. A zero fit is an accessibility hazard.
int BLResearchShapeMixtureValue(uint64_t board) {
    static const struct {uint64_t mask;int w,h;} shape[28]={
        {UINT64_C(0x1),1,1}, /* 0 # */
        {UINT64_C(0x3),2,1}, /* 1 ## */
        {UINT64_C(0x7),3,1}, /* 2 ### */
        {UINT64_C(0xf),4,1}, /* 3 #### */
        {UINT64_C(0x1f),5,1}, /* 4 ##### */
        {UINT64_C(0x101),1,2}, /* 5 #/# */
        {UINT64_C(0x10101),1,3}, /* 6 #/#/# */
        {UINT64_C(0x1010101),1,4}, /* 7 #/#/#/# */
        {UINT64_C(0x101010101),1,5}, /* 8 #/#/#/#/# */
        {UINT64_C(0x303),2,2}, /* 9 ##/## */
        {UINT64_C(0x70707),3,3}, /* 10 ###/###/### */
        {UINT64_C(0x30303),2,3}, /* 11 ##/##/## */
        {UINT64_C(0x707),3,2}, /* 12 ###/### */
        {UINT64_C(0x603),3,2}, /* 13 ##./.## */
        {UINT64_C(0x306),3,2}, /* 14 .##/##. */
        {UINT64_C(0x301),2,2}, /* 15 #./## */
        {UINT64_C(0x302),2,2}, /* 16 .#/## */
        {UINT64_C(0x107),3,2}, /* 17 ###/#.. */
        {UINT64_C(0x407),3,2}, /* 18 ###/..# */
        {UINT64_C(0x207),3,2}, /* 19 ###/.#. */
        {UINT64_C(0x702),3,2}, /* 20 .#./### */
        {UINT64_C(0x40603),3,3}, /* 21 ##./.##/..# */
        {UINT64_C(0x507),3,2}, /* 22 ###/#.# */
        {UINT64_C(0x705),3,2}, /* 23 #.#/### */
        {UINT64_C(0x701),3,2}, /* 24 #../### */
        {UINT64_C(0x704),3,2}, /* 25 ..#/### */
        {UINT64_C(0x20303),3,3}, /* 26 ##./##./.#. */
        {UINT64_C(0x20702),3,3} /* 27 .#./###/.#. */
    };
    int capped=0,unplaceable=0,oneLanding=0;
    for(int i=0;i<28;++i) {
        // Robust proxy weight: balanced among general pieces, the heavy
        // shape family and the shifted-heavy family. All retain positive
        // weight; this is NOT fitted to withheld test seeds.
        int weight=2;
        if((i>=9&&i<=14)||i>=21)weight+=3;
        if(i==4||i==8||i==10||i==11||i>=21)weight+=3;
        int n=fit(board,shape[i].mask,shape[i].w,shape[i].h);
        capped+=weight*(n>6?6:n);
        if(n==0)unplaceable+=weight;
        else if(n==1)oneLanding+=weight;
    }
    // Reward broad coverage rather than just the single rarest hard shape.
    // Penalize unavailable types and fragile single-landing types. This is a
    // bounded afterstate score, never a guarantee about any future tray.
    return 5*capped-95*unplaceable-25*oneLanding;
}
// Bounded corridor balance on actual board geometry; zero when either
// orientation is unavailable, to avoid extrapolating from impossible moves.
static int BLResearchOrthogonalEscape(uint64_t board) {
    if(64-bits(board)>=40)return 0;
    int h=fit(board,UINT64_C(0x1f),5,1);
    int v=fit(board,UINT64_C(0x0101010101),1,5);
    int m=h<v?h:v;
    return m>0?((m>6?6:m)*95):0;
}
// Distinct from single-zero or clear-axis metrics: protect two landing options
// for every difficult shape, on crowded afterstates only. No hidden RNG.
static int BLResearchRedundantHardLandings(uint64_t board) {
 if(64-bits(board)>=34)return 0;
 static const struct {uint64_t m;int w,h;} shapes[7]={
 {0x40603,3,3},{0x507,3,2},{0x705,3,2},{0x701,3,2},
 {0x704,3,2},{0x20303,3,3},{0x20702,3,3}};
 int safe=0,fragile=0;
 for(int i=0;i<7;i++){
  int n=fit(board,shapes[i].m,shapes[i].w,shapes[i].h);
  safe+=(n>=2); fragile+=(n==1);
 }
 return 110*safe-125*fragile;
}
// Exact disjoint square capacity: two simultaneous 3x3 footprints offer
// resilience to multiple bulky future pieces, unlike redundant landings that
// may all overlap the same fragile pocket. No hidden future-tray sampling.
static int BLResearchDisjointSquarePair(uint64_t board) {
    if(64-bits(board)>=40)return 0;
    uint64_t slots[36];int n=0;
    for(int r=0;r<6;r++)for(int c=0;c<6;c++){
        uint64_t m=UINT64_C(0x70707)<<(r*8+c);
        if(!(board&m))slots[n++]=m;
    }
    int pairs=0;
    for(int i=0;i<n;i++)for(int j=i+1;j<n;j++)
        if(!(slots[i]&slots[j])){if(++pairs>=6)return 720;}
    return pairs*120;
}
// Count how many distinct empty cells remain reachable by legal 3x3,
// horizontal 5-bar and vertical 5-bar placements. Unlike counts of landing
// positions, this detects when all legal options reuse one narrow pocket.
// Only the observable afterstate is used; no future RNG or hidden pieces.
static int BLResearchFreeCellMobility(uint64_t board) {
    if(64-bits(board)>=40)return 0;
    const uint64_t patterns[3]={UINT64_C(0x70707),UINT64_C(0x1f),UINT64_C(0x0101010101)};
    const int widths[3]={3,5,1},heights[3]={3,1,5};
    uint64_t covered[3]={0,0,0};
    for(int k=0;k<3;k++)
        for(int r=0;r<=8-heights[k];r++)
            for(int c=0;c<=8-widths[k];c++) {
                uint64_t m=patterns[k]<<(r*8+c);
                if(!(board&m))covered[k]|=m;
            }
    uint64_t any=covered[0]|covered[1]|covered[2];
    uint64_t shared=(covered[0]&covered[1])|(covered[0]&covered[2])|(covered[1]&covered[2]);
    int unreachable=bits((~board)&(~any));
    return 28*bits(any)+16*bits(shared)-45*unreachable;
}
// Count disjoint 3x3 landing zones, not raw overlapping 3x3 fits.
// At most four zones on an 8x8 board. Deterministic and RNG-blind.
static int BLResearchIndependentSquareZones(uint64_t board) {
    if(64-bits(board)>=40)return 0;
    uint64_t placements[36];int n=0;
    for(int r=0;r<6;++r)for(int c=0;c<6;++c){
        uint64_t m=UINT64_C(0x70707)<<(8*r+c);
        if(!(board&m))placements[n++]=m;
    }
    int best=0;
    for(int i=0;i<n;++i){
        int count=1;uint64_t used=placements[i];
        for(int j=0;j<n;++j){
            if(!(used&placements[j])){used|=placements[j];++count;}
        }
        if(count>best)best=count;
    }
    // A single available zone is fragile, two independent zones provide
    // distinct space for consecutive bulky pieces.
    return best>=2?550*(best-1):best==1?-250:-650;
}
// Independent square-and-bar placements: a bulky square landing is only
// useful as a robust escape if a 5-bar can coexist without occupying its cells.
// Use the weaker bar orientation to avoid a one-axis false sense of safety.
// No future RNG, no scenario label, and no modification to legal tray search.
// Iteration 65: the prior square+bar pair reward missed lower-tail heavy
// failures (holdout 753 heavy p10 17 vs 18). Require an exact three-piece
// sequential escape with BOTH bar orientations and a square, including real
// row/column clears after each legal placement. Test each of the six orders.
// This is a deterministic afterstate geometry probe, not a hidden next tray.
// Bounded deterministic accessibility reserve; no hidden next-piece RNG.
static int BLResearchCrowdedHardFloor(uint64_t board) {
    // Iteration 75: the immediate irregular clear-count proxy still left
    // shifted-heavy survival at 1.17x on independent holdout 712.
    // Hypothesis: a clear is valuable only if its exact afterstate restores
    // a usable hard-piece landing, rather than just removing a row/column.
    // Public hypothetical geometry only; no hidden future RNG.
    // New lower-tail hypothesis: defer this costly speculative rescue penalty
    // until fewer than 28 cells are free. At 28..31 free cells, preserve
    // actual current-tray geometry ranking instead of penalizing boards
    // for hypothetical irregular next pieces. No hidden RNG or illegal moves.
    if(64-bits(board)>=28)return 0;
    static const struct {uint64_t mask;int w,h;} irregular[7]={
        {UINT64_C(0x40603),3,3},{UINT64_C(0x507),3,2},
        {UINT64_C(0x705),3,2},{UINT64_C(0x701),3,2},
        {UINT64_C(0x704),3,2},{UINT64_C(0x20303),3,3},
        {UINT64_C(0x20702),3,3}
    };
    const int initial_square=fit(board,UINT64_C(0x70707),3,3);
    const int initial_hbar=fit(board,UINT64_C(0x1f),5,1);
    const int initial_vbar=fit(board,UINT64_C(0x0101010101),1,5);
    int rescued_types=0, robust_types=0;
    for(int i=0;i<7;++i){
        int best=0;
        for(int r=0;r<=8-irregular[i].h;++r)
            for(int c=0;c<=8-irregular[i].w;++c){
                uint64_t m=irregular[i].mask<<(8*r+c);
                if(board&m)continue;
                int lines=0;
                uint64_t after=clear_full(board|m,&lines);
                if(!lines)continue;
                int sq=fit(after,UINT64_C(0x70707),3,3);
                int hb=fit(after,UINT64_C(0x1f),5,1);
                int vb=fit(after,UINT64_C(0x0101010101),1,5);
                int recovered=(sq>initial_square)+(hb>initial_hbar)+
                              (vb>initial_vbar);
                int available=(sq>0)+(hb>0)+(vb>0);
                int value=3*recovered+available;
                if(value>best)best=value;
            }
        if(best>0)++rescued_types;
        if(best>=5)++robust_types;
    }
    // Saturate per shape; reward multiple independent shape types whose
    // clears restore usable square/bar capacity. Never reward illegal moves.
    return 105*rescued_types+145*robust_types-
           (rescued_types==0?400:0);
}
static int BLResearchCrossFamilyDisjoint(uint64_t board) {
    if(64-bits(board)>=40)return 0;
    const uint64_t masks[3]={UINT64_C(0x70707),UINT64_C(0x1f),UINT64_C(0x0101010101)};
    const int w[3]={3,5,1},h[3]={3,1,5};
    int supported=0,clearing=0;
    for(int a=0;a<3;++a)for(int b=0;b<3;++b) {
        if(a==b)continue;
        int c=3-a-b,found=0,foundClear=0;
        for(int ar=0;ar<=8-h[a]&&!foundClear;++ar)
          for(int ac=0;ac<=8-w[a]&&!foundClear;++ac){
            uint64_t am=masks[a]<<(8*ar+ac);
            if(board&am)continue;
            int linesA=0;
            uint64_t afterA=clear_full(board|am,&linesA);
            for(int br=0;br<=8-h[b]&&!foundClear;++br)
              for(int bc=0;bc<=8-w[b]&&!foundClear;++bc){
                uint64_t bm=masks[b]<<(8*br+bc);
                if(afterA&bm)continue;
                int linesB=0;
                uint64_t afterB=clear_full(afterA|bm,&linesB);
                if(fit(afterB,masks[c],w[c],h[c])>0){
                    found=1;
                    if(linesA||linesB)foundClear=1;
                }
              }
          }
        supported+=found;
        clearing+=foundClear;
    }
    // A clear-producing continuation earns a premium because mere coexisting
    // empty footprints can still leave a brittle board after two moves.
    return 110*supported+100*clearing-(supported==0?500:0);
}
// Research-only: exact occupancy of rows/columns that are one or two cells
// from clearing. A potential clear must have its missing cells in a legal
// 2x2 or 3-long placement; merely counting near-full lines overvalues holes.
// This is a geometric late rerank of legal complete-tray afterstates, with
// no next-tray RNG, no alteration to the independent v1.7 baseline.
static int BLResearchLineRepairPotential(uint64_t board) {
    if(64-bits(board)>=40)return 0;
    uint64_t accessible=0;
    const uint64_t shapes[4]={UINT64_C(0x3),UINT64_C(0x101),UINT64_C(0x303),UINT64_C(0x7)};
    const int widths[4]={2,1,2,3},heights[4]={1,2,2,1};
    for(int k=0;k<4;k++)
        for(int r=0;r<=8-heights[k];r++)
            for(int c=0;c<=8-widths[k];c++){
                uint64_t m=shapes[k]<<(r*8+c);
                if(!(board&m))accessible|=m;
            }
    int score=0;
    for(int r=0;r<8;r++){
        uint64_t line=UINT64_C(255)<<(8*r);
        uint64_t holes=line&~board;
        int n=bits(holes);
        if(n>=1 && n<=3){
            int reachable=bits(holes&accessible);
            if(reachable==n)score+=(4-n)*125;
            else score-=90*(n-reachable);
        }
    }
    for(int c=0;c<8;c++){
        uint64_t line=UINT64_C(0x0101010101010101)<<c;
        uint64_t holes=line&~board;
        int n=bits(holes);
        if(n>=1 && n<=3){
            int reachable=bits(holes&accessible);
            if(reachable==n)score+=(4-n)*125;
            else score-=90*(n-reachable);
        }
    }
    return score;
}
// Bounded deterministic afterstate hypothesis: only credit a near-full line
// when a single legal small piece actually completes it. The prior union
// coverage method falsely credited holes reached by different placements.
// No future-tray RNG or game-specific unseen state is consulted.
static int BLResearchExactRepair(uint64_t board) {
    if(64-bits(board)>=40)return 0;
    const uint64_t shapes[6]={UINT64_C(0x1),UINT64_C(0x3),
        UINT64_C(0x101),UINT64_C(0x303),UINT64_C(0x7),UINT64_C(0x10101)};
    const int ws[6]={1,2,1,2,3,1},hs[6]={1,1,2,2,1,3};
    uint64_t lines[16];int missing[16],nlines=0;
    for(int r=0;r<8;r++){
        uint64_t holes=(UINT64_C(255)<<(8*r))&~board;
        int n=bits(holes);
        if(n>=1&&n<=3){lines[nlines]=holes;missing[nlines++]=n;}
    }
    for(int c=0;c<8;c++){
        uint64_t holes=(UINT64_C(0x0101010101010101)<<c)&~board;
        int n=bits(holes);
        if(n>=1&&n<=3){lines[nlines]=holes;missing[nlines++]=n;}
    }
    int score=0;
    for(int i=0;i<nlines;i++){
        int ways=0;
        for(int k=0;k<6;k++)for(int r=0;r<=8-hs[k];r++)
            for(int c=0;c<=8-ws[k];c++){
                uint64_t m=shapes[k]<<(8*r+c);
                if(!(m&board) && (m&lines[i])==lines[i])ways++;
            }
        // Saturate redundancy so an easy one-cell clear cannot overwhelm
        // preservation of several independent repairable lines.
        if(ways)score+=(4-missing[i])*110+(ways>3?3:ways)*35;
        else score-=140;
    }
    return score;
}
// Unlike near-full-line rewards, simulate each legal public small-piece clear
// and measure whether the resulting cleared board regains 3x3 capacity.
// No hidden future tray, RNG, or phone state is accessed.
static int BLResearchPostClearSquareRecovery(uint64_t board) {
    if(64-bits(board)>=40)return 0;
    const uint64_t masks[5]={UINT64_C(1),UINT64_C(3),UINT64_C(0x101),UINT64_C(7),UINT64_C(0x10101)};
    const int widths[5]={1,2,1,3,1},heights[5]={1,1,2,1,3};
    int initial=fit(board,UINT64_C(0x70707),3,3);
    int best=0,distinct=0;
    for(int k=0;k<5;k++)for(int r=0;r<=8-heights[k];r++)
        for(int c=0;c<=8-widths[k];c++){
            uint64_t m=masks[k]<<(8*r+c);
            if(board&m)continue;
            int cleared=0;
            uint64_t next=clear_full(board|m,&cleared);
            if(!cleared)continue;
            int after=fit(next,UINT64_C(0x70707),3,3);
            int gain=after-initial;
            if(gain>best)best=gain;
            if(gain>=2)distinct++;
        }
    if(best>8)best=8;
    if(distinct>4)distinct=4;
    return best*90+distinct*35;
}
// New hypothesis: rank completed trays by the strongest exact small-piece clear
// that restores accessibility for the *worst* of square, 5-horizontal and
// 5-vertical. Avoids rewarding 3x3-only gains while heavy bars remain trapped.
// Future shapes are public templates, not sampled hidden next-tray RNG.
// Cross-shape continuation: how many distinct hard-shape families have a legal
// first placement leaving a different family still placeable? This directly
// measures trap avoidance, not post-clear square recovery.
// Conservative lower-tail hard-shape clear accessibility: a cramped board
// scores only if EACH of square, horizontal bar, vertical bar can legally
// clear a line immediately. Exact public geometry; no future RNG.
static int BLResearchHardClearTail(uint64_t board) {
    if (64-bits(board)>=38) return 0;
    const uint64_t masks[3]={UINT64_C(0x70707),UINT64_C(0x1f),UINT64_C(0x0101010101)};
    const int widths[3]={3,5,1},heights[3]={3,1,5};
    int clearTypes=0,minimum=1000;
    for(int k=0;k<3;k++){
        int count=0;
        for(int r=0;r<=8-heights[k];r++)for(int c=0;c<=8-widths[k];c++){
            uint64_t placed=masks[k]<<(8*r+c);
            if(board&placed)continue;
            int lines=0;
            (void)clear_full(board|placed,&lines);
            if(lines>0)count++;
        }
        if(count>0)clearTypes++;
        if(count<minimum)minimum=count;
    }
    // Cap redundant landings, emphasize the weakest family over aggregate.
    return 180*clearTypes+90*(minimum>4?4:minimum);
}
// Exact legal hard-family footprints, without hidden next-tray information.
// A cell is a common-mode bottleneck when every legal landing of multiple
// difficult shape families requires it. Penalize only crowded boards.
static int BLResearchHardLandingBottleneck(uint64_t board) {
    if(64-bits(board)>=38)return 0;
    const uint64_t masks[3]={UINT64_C(0x70707),UINT64_C(0x1f),UINT64_C(0x0101010101)};
    const int widths[3]={3,5,1},heights[3]={3,1,5};
    uint64_t mandatory[3]={0,0,0};
    int available[3]={0,0,0};
    for(int k=0;k<3;++k){
        uint64_t common=~UINT64_C(0);
        for(int r=0;r<=8-heights[k];++r)
            for(int c=0;c<=8-widths[k];++c){
                uint64_t placed=masks[k]<<(8*r+c);
                if(board&placed)continue;
                common&=placed;
                ++available[k];
            }
        if(available[k])mandatory[k]=common;
    }
    int contested=0;
    for(int a=0;a<3;++a)for(int b=a+1;b<3;++b)
        if(available[a]&&available[b])contested+=bits(mandatory[a]&mandatory[b]);
    int zero=(available[0]==0)+(available[1]==0)+(available[2]==0);
    return -160*contested-240*zero;
}
static int BLResearchHardCrossFit(uint64_t board) {
    if(64-bits(board)>=38)return 0;
    const uint64_t masks[3]={UINT64_C(0x70707),UINT64_C(0x1f),UINT64_C(0x0101010101)};
    const int widths[3]={3,5,1},heights[3]={3,1,5};
    int supported=0;
    for(int first=0;first<3;++first){
        int viable=0;
        for(int r=0;r<=8-heights[first]&&!viable;++r)
            for(int c=0;c<=8-widths[first]&&!viable;++c){
                uint64_t placement=masks[first]<<(8*r+c);
                if(board&placement)continue;
                int lines=0;
                uint64_t after=clear_full(board|placement,&lines);
                for(int second=0;second<3;++second){
                    if(second==first)continue;
                    if(fit(after,masks[second],widths[second],heights[second])>0){
                        viable=1;break;
                    }
                }
            }
        supported+=viable;
    }
    return 420*supported;
}
static int BLResearchPostClearHardRescue(uint64_t board) {
    if(64-bits(board)>=40)return 0;
    const uint64_t small[5]={UINT64_C(1),UINT64_C(3),UINT64_C(0x101),UINT64_C(7),UINT64_C(0x10101)};
    const int w[5]={1,2,1,3,1},h[5]={1,1,2,1,3};
    const uint64_t hard[3]={UINT64_C(0x70707),UINT64_C(0x1f),UINT64_C(0x0101010101)};
    const int hw[3]={3,5,1},hh[3]={3,1,5};
    int before[3];
    for(int i=0;i<3;i++)before[i]=fit(board,hard[i],hw[i],hh[i]);
    int best=0,robust=0;
    for(int k=0;k<5;k++)for(int r=0;r<=8-h[k];r++)
      for(int c=0;c<=8-w[k];c++){
        uint64_t mask=small[k]<<(8*r+c);
        if(board&mask)continue;
        int lines=0;
        uint64_t next=clear_full(board|mask,&lines);
        if(!lines)continue;
        int minGain=100,restored=0;
        for(int i=0;i<3;i++){
          int after=fit(next,hard[i],hw[i],hh[i]);
          int gain=after-before[i];
          if(gain<minGain)minGain=gain;
          if(before[i]==0&&after>0)restored++;
        }
        int value=(minGain>0?(minGain>5?5:minGain)*110:0)+restored*95;
        if(value>best)best=value;
        if(minGain>0)robust++;
      }
    return best+(robust>3?3:robust)*35;
}
static int survival_value(uint64_t board) {
    int q=future_quality(board);
    int a=fit(board,UINT64_C(0x70707),3,3);
    int h=fit(board,UINT64_C(0x1f),5,1);
    int v=fit(board,UINT64_C(0x0101010101),1,5);
    int rect1=fit(board,UINT64_C(0x707),3,2);
    int rect2=fit(board,UINT64_C(0x30303),2,3);
    int empty=64-bits(board);
    if(!a)q-=empty<30?1200:350;
    if(!h)q-=empty<24?900:250;
    if(!v)q-=empty<24?900:250;
    q+=((a>8?8:a)+(h>8?8:h)+(v>8?8:v))*45;
    q+=((rect1>8?8:rect1)+(rect2>8?8:rect2))*30;
    // Remaining empty islands of size 1-2 cannot hold most useful pieces.
    uint64_t free=~board;
    const uint64_t edgeR=UINT64_C(0x7f7f7f7f7f7f7f7f);
    const uint64_t edgeL=UINT64_C(0xfefefefefefefefe);
    int regions=0,isolated=0;
    while(free) {
        uint64_t island=free&(~free+1),previous;
        do {
            previous=island;
            uint64_t near=((island&edgeR)<<1)|((island&edgeL)>>1)|
                          (island<<8)|(island>>8);
            island|=near&free;
        } while(island!=previous);
        free&=~island;
        ++regions;
        int area=bits(island);
        if(area<=2)isolated+=3-area;
    }
    q-=isolated*250;
    if(regions>1)q-=(regions-1)*100;
    // Features after a completed tray: 2x2 flexibility, nearly completed
    // rows/columns, shorter bars and L-shaped future landing zones. Only
    // the relative weights are trainable, not the move legality rules.
    if(blTrainWeights[0])
        q+=blTrainWeights[0]*fit(board,UINT64_C(0x303),2,2);
    if(blTrainWeights[1]) {
        int almost=0;
        for(int row=0;row<8;row++){
            int n=bits((board>>(row*8))&255);
            if(n==7)almost+=2;
            if(n==6)almost++;
        }
        for(int col=0;col<8;col++){
            int n=bits(board&(UINT64_C(0x0101010101010101)<<col));
            if(n==7)almost+=2;
            if(n==6)almost++;
        }
        q+=blTrainWeights[1]*almost;
    }
    if(blTrainWeights[2])
        q+=blTrainWeights[2]*(fit(board,UINT64_C(0xf),4,1)+
                              fit(board,UINT64_C(0x01010101),1,4));
    if(blTrainWeights[3])
        q+=blTrainWeights[3]*(fit(board,UINT64_C(0x10303),2,3)+
                              fit(board,UINT64_C(0x30301),2,3));
    // Research-only shifted-heavy survival hypothesis. The synthetic hard
    // generator favors irregular shapes 21..27 that are NOT represented by
    // the square/bar/rectangle footprint features above. Count their actual
    // legal landing zones after a full tray; no hidden future piece is used.
    // The independent v1.7 SMART SURVIVAL baseline is compiled separately
    // from src/BLDualStrategy.c and remains unchanged.
    static const struct {uint64_t mask; int width,height;} hard[7]={
        {UINT64_C(0x40603),3,3}, {UINT64_C(0x507),3,2},
        {UINT64_C(0x705),3,2},   {UINT64_C(0x701),3,2},
        {UINT64_C(0x704),3,2},   {UINT64_C(0x20303),3,3},
        {UINT64_C(0x20702),3,3}
    };
    int covered=0,cappedLandings=0;
#if BL_RESEARCH_BOTTOM3_HARD_COVERAGE
    int smallest[3]={64,64,64};
#endif
    for(int i=0;i<7;++i) {
        int n=fit(board,hard[i].mask,hard[i].width,hard[i].height);
        covered+=(n>0);
        cappedLandings+=(n>4?4:n);
#if BL_RESEARCH_BOTTOM3_HARD_COVERAGE
        // Maintain the exact three smallest landing counts (including 0).
        if(n<smallest[0]) {
            smallest[2]=smallest[1];smallest[1]=smallest[0];smallest[0]=n;
        } else if(n<smallest[1]) {
            smallest[2]=smallest[1];smallest[1]=n;
        } else if(n<smallest[2])smallest[2]=n;
#endif
    }
    q+=170*covered+18*cappedLandings;
    if(covered<3)q-=(3-covered)*350;
#if BL_RESEARCH_BOTTOM3_HARD_COVERAGE
    // Lower-tail CVaR-style coverage proxy: bottom 3 of 7, each capped so
    // excess placements of one easy shape cannot hide a zero-fit shape.
    // A double-zero on a crowded afterstate is especially vulnerable to
    // the next unknown heavy tray; no hidden RNG data is used here.
    int tail=0;
    for(int i=0;i<3;++i)tail+=(smallest[i]>6?6:smallest[i]);
    q+=135*tail;
    if(smallest[1]==0 && (64-bits(board))<28)q-=425;
#endif
    return q;
}
// Hard safety check when keeping a small clear would sacrifice the only
// future landing zone for a difficult shape on an already crowded board.
static int dangerously_worse(uint64_t comboBoard,uint64_t safeBoard) {
    int comboEmpty=64-bits(comboBoard);
    if(comboEmpty>=29)return 0; // spare room permits more flexible combos
    const uint64_t shapes[]={UINT64_C(0x70707),
                             UINT64_C(0x1f),
                             UINT64_C(0x0101010101)};
    const int ws[]={3,5,1},hs[]={3,1,5};
    for(int i=0;i<3;++i)
        if(fit(safeBoard,shapes[i],ws[i],hs[i])>0 &&
           fit(comboBoard,shapes[i],ws[i],hs[i])==0)return 1;
    return 0;
}
// True only when two hypothetical large next-tray pieces have disjoint
// legal landing zones *simultaneously*. This cannot guarantee a later
// 3-piece tray, because actual future pieces are unknown and clears could
// make additional orders feasible. A positive result certifies space for
// those two fixed shapes without relying on any hypothetical line clear.
static int pair_fits_now(uint64_t board,
                         uint64_t a,int wa,int ha,
                         uint64_t b,int wb,int hb) {
    for(int ra=0;ra<=8-ha;++ra)for(int ca=0;ca<=8-wa;++ca){
        uint64_t maskA=a<<(ra*8+ca);
        if(board&maskA)continue;
        for(int rb=0;rb<=8-hb;++rb)for(int cb=0;cb<=8-wb;++cb) {
            uint64_t maskB=b<<(rb*8+cb);
            if(!((board|maskA)&maskB))return 1;
        }
    }
    return 0;
}
// Measure the opportunity to clear an entire line when ONE hypothetical
// subsequent shape is available. Unlike the failed three-piece rollout, this
// asks a factual geometric question about a single placement, without any
// guess about order or probabilities of unseen future trays. Bounded to
// 10 representative shapes and at most three clearable sites per shape.
// A large count of empty cells may be unusable if split into separate
// islands. This exact 4-connected empty-space penalty is intentionally
// bounded, and only applied when fewer than 40 cells remain empty. It
// supplements (rather than replaces) legal shape fits / line-clear access.
int BLResearchFreeFragmentationPenalty(uint64_t board) {
    uint64_t free=~board;
    int total=bits(free);
    if(total>=40 || total==0)return 0;
    const uint64_t edgeR=UINT64_C(0x7f7f7f7f7f7f7f7f);
    const uint64_t edgeL=UINT64_C(0xfefefefefefefefe);
    int largest=0;
    while(free) {
        uint64_t island=free&(~free+UINT64_C(1)),old;
        do {
            old=island;
            uint64_t near=((island&edgeR)<<1)|((island&edgeL)>>1)|
                          (island<<8)|(island>>8);
            island|=near&free;
        } while(island!=old);
        int area=bits(island);
        if(area>largest)largest=area;
        free&=~island;
    }
    int fragmented=total-largest;
    if(fragmented>16)fragmented=16;
    return -100*fragmented;
}
int BLResearchFutureClearAccess(uint64_t board) {
    static const struct {uint64_t mask;int w,h;} sample[]={
        {UINT64_C(0x70707),3,3},
        {UINT64_C(0x1f),5,1},{UINT64_C(0x0101010101),1,5},
        {UINT64_C(0x303),2,2},{UINT64_C(0x707),3,2},
        {UINT64_C(0x40603),3,3},{UINT64_C(0x507),3,2},
        {UINT64_C(0x705),3,2},{UINT64_C(0x20303),3,3},
        {UINT64_C(0x20702),3,3}
    };
    int score=0;
    for(unsigned i=0;i<sizeof(sample)/sizeof(sample[0]);++i) {
        int clearing=0;
        for(int r=0;r<=8-sample[i].h && clearing<3;++r) {
            for(int c=0;c<=8-sample[i].w && clearing<3;++c) {
                uint64_t placed=sample[i].mask<<(8*r+c);
                if(placed&board)continue;
                int lines=0;
                (void)clear_full(board|placed,&lines);
                if(lines>0)++clearing;
            }
        }
        // One available future clear is valuable; a third redundant landing
        // must not dominate the entire original whole-tray evaluation.
        if(clearing>0)score+=160+35*clearing;
    }
    return score;
}
// Count distinct difficult shape templates with an immediately legal
// line-clearing placement, not multiple redundant placements of one shape.
static int BLResearchHardClearDiversity(uint64_t board) {
    if(64-bits(board)>=40)return 0;
    static const struct {uint64_t mask;int w,h;} hard[]={
        {UINT64_C(0x40603),3,3},{UINT64_C(0x507),3,2},
        {UINT64_C(0x705),3,2},{UINT64_C(0x701),3,2},
        {UINT64_C(0x704),3,2},{UINT64_C(0x20303),3,3},
        {UINT64_C(0x20702),3,3}
    };
    int distinct=0;
    for(unsigned i=0;i<sizeof(hard)/sizeof(hard[0]);++i) {
        int found=0;
        for(int r=0;r<=8-hard[i].h && !found;++r)
            for(int c=0;c<=8-hard[i].w;++c) {
                uint64_t placed=hard[i].mask<<(8*r+c);
                if(placed&board)continue;
                int lines=0;
                (void)clear_full(board|placed,&lines);
                if(lines>0){found=1;break;}
            }
        distinct+=found;
    }
    return distinct*125;
}
// At most seven hard templates x 64 exact placements; no speculative rollout.
static int BLResearchBiaxialHardClear(uint64_t board) {
    if(64-bits(board)>=40)return 0;
    static const struct {uint64_t mask;int w,h;} hard[]={
        {UINT64_C(0x40603),3,3},{UINT64_C(0x507),3,2},
        {UINT64_C(0x705),3,2},{UINT64_C(0x701),3,2},
        {UINT64_C(0x704),3,2},{UINT64_C(0x20303),3,3},
        {UINT64_C(0x20702),3,3}
    };
    int rowTypes=0,colTypes=0;
    for(int i=0;i<7;++i) {
        int row=0,col=0;
        for(int r=0;r<=8-hard[i].h && !(row&&col);++r)
            for(int c=0;c<=8-hard[i].w && !(row&&col);++c) {
                uint64_t placed=hard[i].mask<<(8*r+c);
                if(placed&board)continue;
                uint64_t after=board|placed;
                for(int k=0;k<8;++k) {
                    if(((after>>(8*k))&255)==255)row=1;
                    if((after&(UINT64_C(0x0101010101010101)<<k))==
                        (UINT64_C(0x0101010101010101)<<k))col=1;
                }
            }
        rowTypes+=row;colTypes+=col;
    }
    int weaker=rowTypes<colTypes?rowTypes:colTypes;
    return 180*weaker+30*(rowTypes+colTypes);
}
// Bounded two-step rescue: count distinct hard first-piece types that can
// clear immediately AND leave a legal landing for a different hard type.
// Cap each type to its first 12 legal clear-producing placements, and never
// infer the next piece or treat this as a real future-tray simulation.
static int BLResearchHardRescueChain(uint64_t board) {
    if(64-bits(board)>=40)return 0;
    static const struct {uint64_t mask;int w,h;} hard[]={
        {UINT64_C(0x40603),3,3},{UINT64_C(0x507),3,2},
        {UINT64_C(0x705),3,2},{UINT64_C(0x20303),3,3},
        {UINT64_C(0x20702),3,3}
    };
    int rescued=0;
    for(int i=0;i<5;++i) {
        int success=0,checked=0;
        for(int r=0;r<=8-hard[i].h && !success;++r)
            for(int c=0;c<=8-hard[i].w && !success;++c) {
                uint64_t placed=hard[i].mask<<(r*8+c);
                if(placed&board)continue;
                int lines=0;
                uint64_t next=clear_full(board|placed,&lines);
                if(!lines)continue;
                if(++checked>12)break;
                for(int j=0;j<5;++j)if(j!=i &&
                    fit(next,hard[j].mask,hard[j].w,hard[j].h)>0) {
                    success=1;break;
                }
            }
        rescued+=success;
    }
    return rescued*155;
}
// A fixed suite of hard PIECE PAIRS, not samples from the game's hidden RNG.
// Includes two 3x3 squares, square/bar, square/irregular, crossing bars,
// and two irregular shapes. Used only for bounded afterstate reranking.
int BLResearchPairCoverage(uint64_t board) {
    int n=0;
    n+=pair_fits_now(board,UINT64_C(0x70707),3,3,
                           UINT64_C(0x70707),3,3);
    n+=pair_fits_now(board,UINT64_C(0x70707),3,3,
                           UINT64_C(0x1f),5,1);
    n+=pair_fits_now(board,UINT64_C(0x70707),3,3,
                           UINT64_C(0x0101010101),1,5);
    n+=pair_fits_now(board,UINT64_C(0x70707),3,3,
                           UINT64_C(0x40603),3,3);
    n+=pair_fits_now(board,UINT64_C(0x1f),5,1,
                           UINT64_C(0x0101010101),1,5);
    n+=pair_fits_now(board,UINT64_C(0x20303),3,3,
                           UINT64_C(0x20702),3,3);
    return n;
}
// Conservative second-tray stress probes. These fixed synthetic trays
// approximate easy, heavy and shifted-heavy challenges; they are NOT the
// actual game's unknown upcoming pieces. Each probe checks all three pieces
// with exact 8x8 placements and simultaneous line clearing. Exhaustion of
// the 192-node search budget returns UNKNOWN, never a false impossibility.
typedef struct {uint64_t mask;int w,h;} BLFutureShape;
static const BLFutureShape futureStressTrays[3][3]={
    {{UINT64_C(0x70707),3,3},{UINT64_C(0x1f),5,1},
     {UINT64_C(0x303),2,2}},
    {{UINT64_C(0x70707),3,3},{UINT64_C(0x70707),3,3},
     {UINT64_C(0x707),3,2}},
    {{UINT64_C(0x40603),3,3},{UINT64_C(0x20303),3,3},
     {UINT64_C(0x20702),3,3}}
};
static int future_tray_dfs(uint64_t board,int used,int sample,int *budget){
    if(used==7)return 1;
    if(*budget<=0)return -1;
    --*budget;
    int unknown=0;
    for(int p=0;p<3;++p){
        if(used&(1<<p))continue;
        const BLFutureShape *shape=&futureStressTrays[sample][p];
        for(int row=0;row<=8-shape->h;++row)
            for(int col=0;col<=8-shape->w;++col) {
                uint64_t mask=shape->mask<<(row*8+col);
                if(board&mask)continue;
                int lines=0;
                uint64_t next=clear_full(board|mask,&lines);
                int result=future_tray_dfs(next,used|(1<<p),sample,budget);
                if(result==1)return 1;
                if(result<0)unknown=1;
                // Truly bound compute: a recursion-budget exhaustion must
                // stop OTHER siblings too, not continue iterating millions
                // of alternatives that all immediately return UNKNOWN.
                if(*budget<=0)return -1;
            }
    }
    return unknown?-1:0;
}
// Bonus only for proven feasible future stress trays; a fully searched
// impossibility gets a modest penalty. Truncated/unknown probes add zero.
// This estimates resilience, NOT the game's future RNG or actual score.
int BLResearchSampledFutureBonus(uint64_t board){
    int bonus=0;
    for(int sample=0;sample<3;++sample){
        int nodes=192;
        int result=future_tray_dfs(board,0,sample,&nodes);
        if(result==1)bonus+=700;
        else if(result==0)bonus-=250;
    }
    return bonus;
}
#if BL_RESEARCH_PAIR_LOOKAHEAD
static void record_afterstate(BLPath pool[BL_FUTURE_TOP],
                              int score[BL_FUTURE_TOP],int *count,
                              const BLPath *path,int quality) {
    // Do not fill all finalist slots with nearly identical boards that
    // cannot explore different hard-piece landing arrangements.
    // Research-only next hypothesis after failed 318/319: the exact-only
    // deduplicator still admits eight high-quality boards with no irregular
    // escape footprints. Give crowded finalist ADMISSION (not final move
    // selection) a bounded bonus for two different exact legal irregular
    // shape landing families. This uses only public board geometry and
    // never observes future trays or changes the v1.7 comparator.
    if (64-bits(path->board)<24) {
        int irregularA=fit(path->board,UINT64_C(0x40603),3,3);
        int irregularB=fit(path->board,UINT64_C(0x20303),3,3);
        if(irregularA>3)irregularA=3;
        if(irregularB>3)irregularB=3;
        quality+=90*(irregularA+irregularB);
    }
    for(int i=0;i<*count;++i) {
        // Research-only shifted-heavy hypothesis: a one- or two-cell
        // difference on a crowded board can open a distinct irregular
        // landing or line-clear route. Preserve those legally reached
        // afterstates for the late deterministic reranker; retain the
        // established near-duplicate collapse on spacious boards.
        // No future RNG, baseline, checkpoint, or iPhone source change.
        const int crowded=(64-bits(path->board)<32);
        if(bits(pool[i].board^path->board)<=(crowded?0:2)) {
            if(quality>score[i]){pool[i]=*path;score[i]=quality;}
            return;
        }
    }
    if(*count<BL_FUTURE_TOP) {
        pool[*count]=*path;
        score[*count]=quality;
        ++*count;
        return;
    }
    int worst=0;
    for(int i=1;i<BL_FUTURE_TOP;++i)
        if(score[i]<score[worst])worst=i;
    if(quality>score[worst]) {
        pool[worst]=*path;
        score[worst]=quality;
    }
}
#if BL_RESEARCH_DUAL_POOL && BL_RESEARCH_MIN_HARD_COVERAGE
// Proven bound on minimax bonus: 450 * min(6, placements) <= 2700.
// Skip expensive seven-shape counting when a new afterstate cannot enter a
// full risk pool, regardless of its actual hard-shape coverage. This is an
// exact safe shortlist prune, not a search-node-budget or scoring change.
static int risk_pool_might_improve(const int score[BL_FUTURE_TOP],
                                   int n,int baseQ) {
    if(n<BL_FUTURE_TOP)return 1;
    int worst=score[0];
    for(int i=1;i<BL_FUTURE_TOP;++i)
        if(score[i]<worst)worst=score[i];
    return baseQ+2700>worst;
}
#endif
#if BL_RESEARCH_CROWDED_PAIR_FAILURE
static int BLResearchCrowdedPairFailure(uint64_t board) {
    if(64-bits(board)>=30)return 0;
    const int squareBar=pair_fits_now(board,UINT64_C(0x70707),3,3,
                                           UINT64_C(0x1f),5,1);
    const int irregularSquare=pair_fits_now(board,UINT64_C(0x40603),3,3,
                                                  UINT64_C(0x70707),3,3);
    const int irregularPair=pair_fits_now(board,UINT64_C(0x20303),3,3,
                                                UINT64_C(0x20702),3,3);
    // Iteration 79: the zero-only floor left heavy/shifted-heavy mean
    // survival at 1.17-1.43x. A single feasible pair is also a brittle
    // bottleneck: check the orthogonal bar and a distinct shifted irregular
    // pairing, and require at least two *families* of sequential escape.
    // Exact public geometry only; no hidden future-tray RNG.
    const int squareVertical=pair_fits_now(board,UINT64_C(0x70707),3,3,
                                                 UINT64_C(0x0101010101),1,5);
    const int shiftedPair=pair_fits_now(board,UINT64_C(0x507),3,2,
                                             UINT64_C(0x705),3,2);
    const int familyBar=(squareBar||squareVertical);
    const int familyIrregular=(irregularSquare||irregularPair||shiftedPair);
    const int feasible=familyBar+familyIrregular;
    // Do not reward easy boards; distinguish no escape from a single
    // fragile family while avoiding counting correlated landings as distinct.
    // Iteration 81: shifted-heavy p10 regressed on holdout 782 despite
    // pair-family diversity. An available two-piece route may rely on a
    // single narrow shifted-shape landing. Require redundancy of distinct
    // EXACT legal shifted-shape starts before trusting that escape family.
    // This is deterministic public geometry, not next-tray prediction.
    const int shiftedA=fit(board,UINT64_C(0x507),3,2);
    const int shiftedB=fit(board,UINT64_C(0x705),3,2);
    const int shiftedC=fit(board,UINT64_C(0x20303),3,3);
    const int shiftedTypes=(shiftedA>0)+(shiftedB>0)+(shiftedC>0);
    const int shiftedFragile=shiftedTypes<2;
    // Iteration 84 hypothesis (UNTESTED): the no-pair floor already punishes
    // boards with no two-piece escape; stacking another 650 penalty when ONE
    // shifted piece can still land may discard recoverable afterstates.
    // Apply the extra lower-tail penalty only to a genuine shifted-family
    // zero-fit dead end, not to the broader one-type bottleneck. This changes
    // the activation logic, not the coefficients, and uses only public
    // current-board geometry. Test on fresh independent paired seeds.
    const int shiftedDeadEnd=(shiftedA==0 && shiftedB==0 && shiftedC==0);
    (void)shiftedFragile; // retained for explicit future A/B diagnostics
    // Iteration 86: the 350/351/352 paired holdouts exposed shifted-heavy
    // lower-tail regression (p10 18 vs 19 on 352) despite no illegal moves.
    // Distinguish a genuinely stranded zero-fit board from one with an exact
    // one-piece line clear available to reopen shifted-family space. The
    // previous unconditional dead-end penalty can discard a recoverable
    // position. Only public hypothetical geometry is used; no future RNG.
    int hasShiftedRescue=0;
    if(shiftedDeadEnd && feasible==0) {
        const uint64_t rescueMasks[4]={UINT64_C(1),UINT64_C(3),
                                       UINT64_C(0x101),UINT64_C(0x303)};
        const int rescueW[4]={1,2,1,2},rescueH[4]={1,1,2,2};
        for(int k=0;k<4 && !hasShiftedRescue;++k)
          for(int r=0;r<=8-rescueH[k] && !hasShiftedRescue;++r)
            for(int c=0;c<=8-rescueW[k] && !hasShiftedRescue;++c) {
                uint64_t m=rescueMasks[k]<<(8*r+c);
                if(board&m)continue;
                int lines=0;
                uint64_t after=clear_full(board|m,&lines);
                // Iteration 88: a rescue that restores only ONE shifted family
                // can still leave a brittle lower-tail board. Require two
                // distinct families after the same exactly legal clear.
                // No future tray or RNG access; this is research-only.
                if(lines) {
                    int restored=(fit(after,UINT64_C(0x507),3,2)>0)
                                +(fit(after,UINT64_C(0x705),3,2)>0)
                                +(fit(after,UINT64_C(0x20303),3,3)>0);
                    if(restored>=2)hasShiftedRescue=1;
                }
            }
    }
    return (feasible==0?-1050:(feasible==1?-420:0))
           -((shiftedDeadEnd && feasible==0 && !hasShiftedRescue)?650:0);
}
#endif
static void rerank_afterstates(BLSearch *ctx) {
    int best=INT_MIN;
    for(int i=0;i<ctx->topAllN;++i) {
        int utility=ctx->topAllQ[i]
#if BL_RESEARCH_MIN_HARD_COVERAGE
            + 450*worst_hard_coverage(ctx->topAll[i].board)
#endif
            +550*
            BLResearchPairCoverage(ctx->topAll[i].board)
            + ((bits(ctx->topAll[i].board)>36 &&
                (fit(ctx->topAll[i].board,UINT64_C(0x70707),3,3)==0 ||
                 fit(ctx->topAll[i].board,UINT64_C(0x1f),5,1)==0 ||
                 fit(ctx->topAll[i].board,UINT64_C(0x0101010101),1,5)==0)) ? -700 : 0)
#if BL_RESEARCH_SHAPE_MIXTURE
            + BLResearchShapeMixtureValue(ctx->topAll[i].board)
#endif
            ;
#if BL_RESEARCH_NEXT_TRAY_ROLLOUT
        utility+=BLResearchSampledFutureBonus(ctx->topAll[i].board);
#endif
#if BL_RESEARCH_HARD_ZERO_FLOOR
        // Zero-fit floor is conditional on scarce free space, rather than
        // applying a global reward that dominated earlier experiments.
        if(64-bits(ctx->topAll[i].board)<28 &&
           worst_hard_coverage(ctx->topAll[i].board)==0)
            utility-=850;
#endif
#if BL_RESEARCH_FUTURE_CLEAR_ACCESS
        utility+=BLResearchFutureClearAccess(ctx->topAll[i].board);
#endif
#if BL_RESEARCH_CONNECTED_FREE_REGION
        utility+=BLResearchFreeFragmentationPenalty(ctx->topAll[i].board);
#endif
#if BL_RESEARCH_ORTHOGONAL_ESCAPE
        utility+=BLResearchOrthogonalEscape(ctx->topAll[i].board);
#endif
#if BL_RESEARCH_HARD_CLEAR_DIVERSITY
        utility+=BLResearchHardClearDiversity(ctx->topAll[i].board);
#endif
#if BL_RESEARCH_BIAXIAL_HARD_CLEAR
        utility+=BLResearchBiaxialHardClear(ctx->topAll[i].board);
#endif
#if BL_RESEARCH_REDUNDANT_HARD_LANDINGS
        utility+=BLResearchRedundantHardLandings(ctx->topAll[i].board);
#endif
#if BL_RESEARCH_LINE_REPAIR_POTENTIAL
        utility+=BLResearchLineRepairPotential(ctx->topAll[i].board);
#endif
#if BL_RESEARCH_EXACT_REPAIR
        utility+=BLResearchExactRepair(ctx->topAll[i].board);
#endif
#if BL_RESEARCH_POST_CLEAR_HARD_RESCUE
        utility+=BLResearchPostClearHardRescue(ctx->topAll[i].board);
#endif
#if BL_RESEARCH_INDEPENDENT_SQUARE_ZONES
        utility+=BLResearchIndependentSquareZones(ctx->topAll[i].board);
#endif
#if BL_RESEARCH_CROWDED_HARD_FLOOR
        utility+=BLResearchCrowdedHardFloor(ctx->topAll[i].board);
#endif
#if BL_RESEARCH_CROWDED_PAIR_FAILURE
        utility+=BLResearchCrowdedPairFailure(ctx->topAll[i].board);
#endif
#if BL_RESEARCH_CROSS_FAMILY_DISJOINT
        utility+=BLResearchCrossFamilyDisjoint(ctx->topAll[i].board);
#endif
#if BL_RESEARCH_HARD_LANDING_BOTTLENECK
        utility+=BLResearchHardLandingBottleneck(ctx->topAll[i].board);
#endif
#if BL_RESEARCH_HARD_CLEAR_TAIL
        utility+=BLResearchHardClearTail(ctx->topAll[i].board);
#endif
#if BL_RESEARCH_POST_CLEAR_SQUARE_RECOVERY
        utility+=BLResearchPostClearSquareRecovery(ctx->topAll[i].board);
#endif
#if BL_RESEARCH_DISJOINT_SQUARE_PAIR
        utility+=BLResearchDisjointSquarePair(ctx->topAll[i].board);
#endif
#if BL_RESEARCH_HARD_RESCUE_CHAIN
        utility+=BLResearchHardRescueChain(ctx->topAll[i].board);
#endif
        if(utility>best) {
            best=utility;
            ctx->smartPath=ctx->topAll[i];
            ctx->smartQuality=utility;
            ctx->haveSmart=1;
        }
    }
#if BL_RESEARCH_DUAL_POOL && BL_RESEARCH_MIN_HARD_COVERAGE
    // Crucial difference from iteration 36: rare-shape-aware finalists
    // compete with ordinary finalists, not only AFTER quality truncation.
    for(int i=0;i<ctx->topRiskN;++i) {
        int utility=ctx->topRiskQ[i]+550*
            BLResearchPairCoverage(ctx->topRisk[i].board);
#if BL_RESEARCH_NEXT_TRAY_ROLLOUT
        utility+=BLResearchSampledFutureBonus(ctx->topRisk[i].board);
#endif
        if(utility>best) {
            best=utility;
            ctx->smartPath=ctx->topRisk[i];
            ctx->smartQuality=utility;
            ctx->haveSmart=1;
        }
    }
#endif
    best=INT_MIN;
    for(int i=0;i<ctx->topComboN;++i) {
        int utility=ctx->topComboQ[i]
#if BL_RESEARCH_MIN_HARD_COVERAGE
            + 450*worst_hard_coverage(ctx->topCombo[i].board)
#endif
            +550*
            BLResearchPairCoverage(ctx->topCombo[i].board)
            + ((bits(ctx->topCombo[i].board)>36 &&
                (fit(ctx->topCombo[i].board,UINT64_C(0x70707),3,3)==0 ||
                 fit(ctx->topCombo[i].board,UINT64_C(0x1f),5,1)==0 ||
                 fit(ctx->topCombo[i].board,UINT64_C(0x0101010101),1,5)==0)) ? -700 : 0)
#if BL_RESEARCH_SHAPE_MIXTURE
            + BLResearchShapeMixtureValue(ctx->topCombo[i].board)
#endif
            ;
#if BL_RESEARCH_NEXT_TRAY_ROLLOUT
        utility+=BLResearchSampledFutureBonus(ctx->topCombo[i].board);
#endif
#if BL_RESEARCH_HARD_ZERO_FLOOR
        // Zero-fit floor is conditional on scarce free space, rather than
        // applying a global reward that dominated earlier experiments.
        if(64-bits(ctx->topCombo[i].board)<28 &&
           worst_hard_coverage(ctx->topCombo[i].board)==0)
            utility-=850;
#endif
#if BL_RESEARCH_FUTURE_CLEAR_ACCESS
        utility+=BLResearchFutureClearAccess(ctx->topCombo[i].board);
#endif
#if BL_RESEARCH_CONNECTED_FREE_REGION
        utility+=BLResearchFreeFragmentationPenalty(ctx->topCombo[i].board);
#endif
#if BL_RESEARCH_ORTHOGONAL_ESCAPE
        utility+=BLResearchOrthogonalEscape(ctx->topCombo[i].board);
#endif
#if BL_RESEARCH_HARD_CLEAR_DIVERSITY
        utility+=BLResearchHardClearDiversity(ctx->topCombo[i].board);
#endif
#if BL_RESEARCH_BIAXIAL_HARD_CLEAR
        utility+=BLResearchBiaxialHardClear(ctx->topCombo[i].board);
#endif
#if BL_RESEARCH_REDUNDANT_HARD_LANDINGS
        utility+=BLResearchRedundantHardLandings(ctx->topCombo[i].board);
#endif
#if BL_RESEARCH_LINE_REPAIR_POTENTIAL
        utility+=BLResearchLineRepairPotential(ctx->topCombo[i].board);
#endif
#if BL_RESEARCH_EXACT_REPAIR
        utility+=BLResearchExactRepair(ctx->topCombo[i].board);
#endif
#if BL_RESEARCH_POST_CLEAR_HARD_RESCUE
        utility+=BLResearchPostClearHardRescue(ctx->topCombo[i].board);
#endif
#if BL_RESEARCH_INDEPENDENT_SQUARE_ZONES
        utility+=BLResearchIndependentSquareZones(ctx->topCombo[i].board);
#endif
#if BL_RESEARCH_CROWDED_HARD_FLOOR
        utility+=BLResearchCrowdedHardFloor(ctx->topCombo[i].board);
#endif
#if BL_RESEARCH_CROWDED_PAIR_FAILURE
        utility+=BLResearchCrowdedPairFailure(ctx->topCombo[i].board);
#endif
#if BL_RESEARCH_CROSS_FAMILY_DISJOINT
        utility+=BLResearchCrossFamilyDisjoint(ctx->topCombo[i].board);
#endif
#if BL_RESEARCH_HARD_LANDING_BOTTLENECK
        utility+=BLResearchHardLandingBottleneck(ctx->topCombo[i].board);
#endif
#if BL_RESEARCH_HARD_CLEAR_TAIL
        utility+=BLResearchHardClearTail(ctx->topCombo[i].board);
#endif
#if BL_RESEARCH_POST_CLEAR_SQUARE_RECOVERY
        utility+=BLResearchPostClearSquareRecovery(ctx->topCombo[i].board);
#endif
#if BL_RESEARCH_DISJOINT_SQUARE_PAIR
        utility+=BLResearchDisjointSquarePair(ctx->topCombo[i].board);
#endif
#if BL_RESEARCH_HARD_RESCUE_CHAIN
        utility+=BLResearchHardRescueChain(ctx->topCombo[i].board);
#endif
        if(utility>best) {
            best=utility;
            ctx->smartComboPath=ctx->topCombo[i];
            ctx->smartComboQuality=utility;
            ctx->haveSmartCombo=1;
        }
    }
#if BL_RESEARCH_DUAL_POOL && BL_RESEARCH_MIN_HARD_COVERAGE
    for(int i=0;i<ctx->topComboRiskN;++i) {
        int utility=ctx->topComboRiskQ[i]+550*
            BLResearchPairCoverage(ctx->topComboRisk[i].board);
#if BL_RESEARCH_NEXT_TRAY_ROLLOUT
        utility+=BLResearchSampledFutureBonus(ctx->topComboRisk[i].board);
#endif
        if(utility>best) {
            best=utility;
            ctx->smartComboPath=ctx->topComboRisk[i];
            ctx->smartComboQuality=utility;
            ctx->haveSmartCombo=1;
        }
    }
#endif
}
#endif // BL_RESEARCH_PAIR_LOOKAHEAD
static int preserved(const BLSearch *ctx,const BLPath *path) {
    if(ctx->initial.known && ctx->initial.streak>0)
        return !path->broken && path->streak>0;
    return path->seenClear;
}
static int earlier(int n) {return n? n:4;}
static void save_if_best(BLSearch *ctx,const BLPath *path) {
    int quality;
    int betterMax=!ctx->haveMax ||
        path->depth>ctx->maxPath.depth ||
        (path->depth==ctx->maxPath.depth && path->lines>ctx->maxPath.lines);
    int maybeMax=ctx->haveMax && path->depth==ctx->maxPath.depth &&
                 path->lines==ctx->maxPath.lines;
    int comboOK=preserved(ctx,path);
    int oldComboOK=ctx->haveCombo && preserved(ctx,&ctx->comboPath);
    int betterCombo=!ctx->haveCombo ||
        path->depth>ctx->comboPath.depth ||
        (path->depth==ctx->comboPath.depth && comboOK>oldComboOK) ||
        (path->depth==ctx->comboPath.depth && comboOK==oldComboOK &&
         earlier(path->firstClear)<earlier(ctx->comboPath.firstClear));
    int maybeCombo=ctx->haveCombo &&
         path->depth==ctx->comboPath.depth && comboOK==oldComboOK &&
         earlier(path->firstClear)==earlier(ctx->comboPath.firstClear);
    if(maybeCombo && path->lines>ctx->comboPath.lines)betterCombo=1;
    // SMART SURVIVAL always considers the best *whole-tray* positions.
    // A single-line combo is enough. Large clears do not get automatic
    // priority. Score only proven legal completed sequences, never futures
    // that require knowing the game's hidden next pieces.
    if(path->depth>=ctx->smartPath.depth) {
        int sq=survival_value(path->board);
#if BL_RESEARCH_PAIR_LOOKAHEAD
        if(path->depth==ctx->total) {
            record_afterstate(ctx->topAll,ctx->topAllQ,&ctx->topAllN,path,sq);
            if(comboOK)
                record_afterstate(ctx->topCombo,ctx->topComboQ,
                                  &ctx->topComboN,path,sq);
#if BL_RESEARCH_DUAL_POOL && BL_RESEARCH_MIN_HARD_COVERAGE
            // Rerank utility can now discover lower-base-quality boards that
            // retain actual landing locations for the least-placeable hard
            // shape. Keep this pool separate to avoid overwriting safe base
            // quality finalists or forcing all policy decisions to change.
            int tryMain=risk_pool_might_improve(ctx->topRiskQ,
                                                   ctx->topRiskN,sq);
            int tryCombo=comboOK &&
                risk_pool_might_improve(ctx->topComboRiskQ,
                                        ctx->topComboRiskN,sq);
            if(tryMain || tryCombo) {
                int riskQ=sq+450*worst_hard_coverage(path->board);
                if(tryMain)
                    record_afterstate(ctx->topRisk,ctx->topRiskQ,
                                      &ctx->topRiskN,path,riskQ);
                if(tryCombo)
                    record_afterstate(ctx->topComboRisk,ctx->topComboRiskQ,
                                      &ctx->topComboRiskN,path,riskQ);
            }
#endif
        }
#endif
        if(!ctx->haveSmart || path->depth>ctx->smartPath.depth ||
           (path->depth==ctx->smartPath.depth &&
            (sq>ctx->smartQuality ||
             (sq==ctx->smartQuality && path->lines>ctx->smartPath.lines)))) {
            ctx->smartPath=*path;
            ctx->smartQuality=sq;
            ctx->haveSmart=1;
        }
        if(comboOK &&
           (!ctx->haveSmartCombo ||
            path->depth>ctx->smartComboPath.depth ||
            (path->depth==ctx->smartComboPath.depth &&
             (sq>ctx->smartComboQuality ||
              (sq==ctx->smartComboQuality &&
               earlier(path->firstClear)<earlier(ctx->smartComboPath.firstClear)))))) {
            ctx->smartComboPath=*path;
            ctx->smartComboQuality=sq;
            ctx->haveSmartCombo=1;
        }
    }
    if(!betterMax&&!maybeMax&&!betterCombo&&!maybeCombo)return;
    quality=future_quality(path->board);
    if(maybeMax && (quality>ctx->maxQuality ||
                    (quality==ctx->maxQuality &&
                     earlier(path->firstClear)<earlier(ctx->maxPath.firstClear))))
        betterMax=1;
    if(maybeCombo && path->lines==ctx->comboPath.lines &&
        quality>ctx->comboQuality)
        betterCombo=1;
    if(betterMax) {
        ctx->maxPath=*path;ctx->maxQuality=quality;ctx->haveMax=1;
    }
    if(betterCombo) {
        ctx->comboPath=*path;ctx->comboQuality=quality;ctx->haveCombo=1;
    }
}
static void search(BLSearch *ctx,BLPath *path) {
    if(ctx->visited>=ctx->budget) {ctx->truncated=1;return;}
    ++ctx->visited;
    if(path->depth==ctx->total) {save_if_best(ctx,path);return;}
    int any=0;
    for(int p=0;p<3;++p) {
        if(ctx->pieces[p].count<=0 || (path->used&(1<<p)))continue;
        for(int j=0;j<ctx->counts[p];++j) {
            const BLPlace *pos=&ctx->placements[p][j];
            if(path->board&pos->mask)continue;
            any=1;
            int lines=0;
            BLPath next=*path;
            next.board=clear_full(path->board|pos->mask,&lines);
            next.used|=1<<p;
            next.moves[path->depth]=(BLMove) {
                .pieceIndex=p,.row=pos->row,.col=pos->col,
                .linesCleared=lines,.before=path->board,.after=next.board
            };
            ++next.depth;
            next.lines+=lines;
            if(lines>0) {
                if(!next.firstClear)next.firstClear=next.depth;
                next.seenClear=1;
                next.streak=next.streak<63?next.streak+1:63;
                next.misses=0;
            } else if(next.streak>0) {
                ++next.misses;
                if(next.misses>=3) {
                    if(ctx->initial.known && ctx->initial.streak>0)next.broken=1;
                    next.streak=0;next.misses=0;
                }
            }
            search(ctx,&next);
            if(ctx->truncated)return;
        }
    }
    if(!any)save_if_best(ctx,path);
}
#if BL_RESEARCH_FAIR_ROOT
// Research-only search fairness: the depth-first baseline spends its full
// node budget on the first indexed piece/landing location on busy boards.
// Allocate the SAME total node budget across legal first actions, so all
// three visible piece orders can receive some consideration. Search still
// requires exact 8x8 legality and never predicts the next unknown tray.
static void search_fair_first(BLSearch *ctx,const BLPath *start) {
    int roots=0;
    for(int p=0;p<3;++p) {
        if(ctx->pieces[p].count<=0)continue;
        for(int j=0;j<ctx->counts[p];++j)
            if(!(start->board&ctx->placements[p][j].mask))++roots;
    }
    if(roots<2) {
        BLPath one=*start;
        search(ctx,&one);
        return;
    }
    const uint32_t fullBudget=ctx->budget;
    int remaining=roots,anyTruncated=0;
    for(int p=0;p<3;++p) {
        if(ctx->pieces[p].count<=0)continue;
        for(int j=0;j<ctx->counts[p];++j) {
            const BLPlace *pos=&ctx->placements[p][j];
            if(start->board&pos->mask)continue;
            if(ctx->visited>=fullBudget){anyTruncated=1;goto done;}
            uint32_t quota=(fullBudget-ctx->visited)/(uint32_t)remaining;
            if(quota==0)quota=1;
            ctx->budget=ctx->visited+quota;
            ctx->truncated=0;
            BLPath next=*start;
            int lines=0;
            next.board=clear_full(start->board|pos->mask,&lines);
            next.used|=1<<p;
            next.moves[0]=(BLMove){
                .pieceIndex=p,.row=pos->row,.col=pos->col,
                .linesCleared=lines,.before=start->board,.after=next.board
            };
            next.depth=1;
            next.lines+=lines;
            if(lines>0) {
                next.firstClear=1;
                next.seenClear=1;
                next.streak=next.streak<63?next.streak+1:63;
                next.misses=0;
            } else if(next.streak>0) {
                ++next.misses;
                if(next.misses>=3) {
                    if(ctx->initial.known&&ctx->initial.streak>0)
                        next.broken=1;
                    next.streak=0;next.misses=0;
                }
            }
            search(ctx,&next);
            anyTruncated|=ctx->truncated;
            --remaining;
        }
    }
done:
    ctx->budget=fullBudget;
    ctx->truncated=anyTruncated;
}
#endif // BL_RESEARCH_FAIR_ROOT
// Last-chance combo rescue. The usual bounded depth-first search sometimes
// never visits a full-tray sequence beginning with a clearing move. When the
// observed combo has two misses, enumerate legal immediate clears separately
// and demand an exactly legal completion for every remaining visible piece.
// Sample at most 12 complete routes per clearing root and 30000 nodes total.
// This uses NO knowledge of future trays. It is research-only and all survival
// and combo gates remain in force.
static void urgent_finish(const BLSearch *ctx,const BLPath *path,
                          BLPath *best,int *bestQuality,
                          int *rootCompletions,uint32_t *nodes) {
    if(*nodes>=30000 || *rootCompletions>=12)return;
    ++*nodes;
    if(path->depth==ctx->total) {
        int q=survival_value(path->board);
        if(q>*bestQuality){*best=*path;*bestQuality=q;}
        ++*rootCompletions;
        return;
    }
    for(int p=0;p<3;++p) {
        if(ctx->pieces[p].count<=0 || (path->used&(1<<p)))continue;
        for(int j=0;j<ctx->counts[p];++j) {
            const BLPlace *pos=&ctx->placements[p][j];
            if(path->board&pos->mask)continue;
            int lines=0;
            BLPath next=*path;
            next.board=clear_full(path->board|pos->mask,&lines);
            next.moves[path->depth]=(BLMove){
                .pieceIndex=p,.row=pos->row,.col=pos->col,
                .linesCleared=lines,.before=path->board,.after=next.board
            };
            next.used|=1<<p;
            ++next.depth;
            next.lines+=lines;
            if(lines>0) {
                if(!next.firstClear)next.firstClear=next.depth;
                next.seenClear=1;
                next.streak=next.streak<63?next.streak+1:63;
                next.misses=0;
            } else if(next.streak>0) {
                ++next.misses;
                if(next.misses>=3) {
                    next.broken=1;
                    next.misses=0;
                    next.streak=0;
                }
            }
            urgent_finish(ctx,&next,best,bestQuality,rootCompletions,nodes);
            if(*nodes>=30000 || *rootCompletions>=12)return;
        }
    }
}
static int find_urgent_complete_clear(const BLSearch *ctx,
                                      const BLPath *start,BLPath *result) {
    int bestQ=INT_MIN;
    uint32_t nodes=0;
    for(int p=0;p<3;++p) {
        if(ctx->pieces[p].count<=0)continue;
        for(int j=0;j<ctx->counts[p];++j) {
            const BLPlace *pos=&ctx->placements[p][j];
            if(start->board&pos->mask)continue;
            int cleared=0;
            uint64_t after=clear_full(start->board|pos->mask,&cleared);
            if(!cleared)continue; // An urgent save MUST clear on move one.
            BLPath root=*start;
            root.board=after;
            root.used|=1<<p;
            root.moves[0]=(BLMove){
                .pieceIndex=p,.row=pos->row,.col=pos->col,
                .linesCleared=cleared,.before=start->board,.after=after
            };
            root.depth=1;
            root.lines=cleared;
            root.firstClear=1;
            root.seenClear=1;
            root.streak=root.streak<63?root.streak+1:63;
            root.misses=0;
            int completions=0;
            urgent_finish(ctx,&root,result,&bestQ,&completions,&nodes);
            if(nodes>=30000) return bestQ!=INT_MIN;
        }
    }
    return bestQ!=INT_MIN;
}
static void pack(BLSolution *to,const BLPath *from,int approximate) {
    memset(to,0,sizeof(*to));
    to->movesFound=from->depth;
    to->totalLines=from->lines;
    to->approximate=approximate;
    for(int i=0;i<from->depth;++i)to->moves[i]=from->moves[i];
}
BLTwoStrategies BLFindTwoStrategies(uint64_t board,const BLPiece pieces[3],
                                    BLComboContext combo,uint32_t nodeBudget) {
    BLTwoStrategies out={0};
    if(!pieces)return out;
    BLSearch ctx={0};
    ctx.pieces=pieces;
    ctx.initial=combo;
    ctx.budget=nodeBudget?nodeBudget:180000;
    if(ctx.budget>500000)ctx.budget=500000;
    for(int p=0;p<3;++p) {
        const BLPiece *shape=&pieces[p];
        if(shape->count<0||shape->count>25)return out;
        if(!shape->count)continue;
        int width=0,height=0;
        uint64_t local=0;
        for(int i=0;i<shape->count;++i) {
            int x=shape->x[i],y=shape->y[i];
            if(x<0||x>=8||y<0||y>=8)return out;
            uint64_t bit=UINT64_C(1)<<(y*8+x);
            if(local&bit)return out;
            local|=bit;
            if(x+1>width)width=x+1;
            if(y+1>height)height=y+1;
        }
        ++ctx.total;
        for(int r=0;r<=8-height;++r)
            for(int c=0;c<=8-width;++c) {
                uint64_t m=0;
                for(int i=0;i<shape->count;++i)
                    m|=UINT64_C(1)<<((r+shape->y[i])*8+c+shape->x[i]);
                BLPlace *q=&ctx.placements[p][ctx.counts[p]++];
                q->mask=m;q->row=(unsigned char)r;q->col=(unsigned char)c;
            }
    }
    if(!ctx.total)return out;
    BLPath start={0};start.board=board;
    if(combo.known && combo.streak>0) {
        start.streak=combo.streak>63?63:combo.streak;
        start.misses=(combo.misses>=0&&combo.misses<3)?combo.misses:0;
    }
#if BL_RESEARCH_FAIR_ROOT
    search_fair_first(&ctx,&start);
#else
    search(&ctx,&start);
#endif
#if BL_RESEARCH_PAIR_LOOKAHEAD
    // The expensive two-shape co-fit assessment runs for at most eight
    // finalists, never for every leaf of the 180k-node current-tray search.
    rerank_afterstates(&ctx);
#endif
    out.exploredNodes=ctx.visited;
    out.exhaustive=!ctx.truncated;
    if(ctx.haveMax) {
        pack(&out.maxLines,&ctx.maxPath,ctx.truncated);
        out.maxFirstClear=ctx.maxPath.firstClear;
        out.maxComboPreserved=preserved(&ctx,&ctx.maxPath);
    }
    if(ctx.haveCombo) {
        pack(&out.keepCombo,&ctx.comboPath,ctx.truncated);
        out.comboFirstClear=ctx.comboPath.firstClear;
        out.keepComboPreserved=preserved(&ctx,&ctx.comboPath);
    }
    // Require all visible pieces to fit. Preserve an ongoing small combo
    // when doing so leaves a nearly-as-safe board, even if that means one
    // clearing line now instead of five lines in the final placement.
    if(ctx.haveSmart) {
        BLPath chosen=ctx.smartPath;
        int chosenQ=ctx.smartQuality;
        if(ctx.haveSmartCombo &&
           ctx.smartComboPath.depth==ctx.smartPath.depth) {
            // Research-only hypothesis: trained policies may under-select safe combo
            // routes. Preserve baseline behavior when weights are zero.
            int tuned=(blTrainWeights[0]||blTrainWeights[1]||
                       blTrainWeights[2]||blTrainWeights[3]);
            // A witnessed streak with one non-clearing placement already
            // consumed has only two moves left to produce a clearing line.
            // Give a proven *complete legal* combo route a modestly larger
            // utility tolerance one step BEFORE the urgent last-chance
            // rescue. Still reject routes that destroy crucial large-piece
            // landing zones on crowded boards; all benchmark gates remain.
            int proactive=ctx.initial.known && ctx.initial.streak>0 &&
                          ctx.initial.misses==1 &&
                          ctx.smartComboPath.depth==ctx.total;
            int gap=tuned?(proactive?6250:5000):4700;
            int sameSafety=ctx.smartComboQuality>=ctx.smartQuality-gap;
            // Last-chance observed combo: a complete legal three-piece
            // route that clears immediately should prevent an avoidable
            // deadline miss. Keep all safety/performance gates in testing.
            int urgent=ctx.initial.known && ctx.initial.streak>0 &&
                       ctx.initial.misses==2;
            int urgentComplete=urgent &&
                ctx.smartComboPath.depth==ctx.total;
            int noCriticalShapeLoss=
                !dangerously_worse(ctx.smartComboPath.board,ctx.smartPath.board);
#if BL_RESEARCH_PROACTIVE_PARETO
            // Pareto-style combo feasibility: with one miss already spent,
            // accept a proven combo-preserving 3-piece route even outside
            // the usual score gap, but only if hard future shape PAIRS
            // remain at least as feasible as the generic best route.
            // If not, retain the ordinary survival/quality tradeoff.
            int proactivePareto=proactive && noCriticalShapeLoss &&
                BLResearchPairCoverage(ctx.smartComboPath.board)>=
                BLResearchPairCoverage(ctx.smartPath.board);
#else
            int proactivePareto=0;
#endif
            if(urgentComplete || proactivePareto ||
               (sameSafety && noCriticalShapeLoss)) {
                chosen=ctx.smartComboPath;
                chosenQ=ctx.smartComboQuality;
            } else {
                out.smartComboRejectedForSafety=1;
            }
        }
        pack(&out.smartSurvival,&chosen,ctx.truncated);
        out.smartComboPreserved=preserved(&ctx,&chosen);
        out.smartFutureQuality=chosenQ;
    }
    // Guard against budget-based pruning that misses a complete path.
    if(ctx.truncated && (out.maxLines.movesFound<ctx.total ||
                         out.keepCombo.movesFound<ctx.total)) {
        BLSolution fallback=BLSolveWithCombo(board,pieces,combo);
        if(fallback.movesFound>out.maxLines.movesFound)out.maxLines=fallback;
        if(fallback.movesFound>out.keepCombo.movesFound)out.keepCombo=fallback;
        if(fallback.movesFound>out.smartSurvival.movesFound) {
            out.smartSurvival=fallback;
            out.smartComboPreserved=0;
            out.smartFutureQuality=survival_value(
                fallback.movesFound?fallback.moves[fallback.movesFound-1].after:board);
        }
    }
    // A bounded 180k-node search can miss an available last-chance clear
    // even when a valid all-three-piece route exists. Run the independent
    // exact-legal rescue only in that failure case; never replace an already
    // immediate-clearing plan. Keep the baseline binary fully unchanged.
    if(combo.known && combo.streak>0 && combo.misses==2 &&
       (!out.smartSurvival.movesFound ||
        out.smartSurvival.moves[0].linesCleared==0)) {
        BLPath rescue={0};
        if(find_urgent_complete_clear(&ctx,&start,&rescue)) {
            pack(&out.smartSurvival,&rescue,1);
            out.smartComboPreserved=1;
            out.smartFutureQuality=survival_value(rescue.board);
        }
    }
#if BL_RESEARCH_BASELINE_RISK_SHIELD
    // A conservative board-risk exception, not a general copy of baseline:
    // only expensive-query v1.7 on a crowded board or imminent combo. Both
    // candidate plans were constructed from exact legal current pieces.
    if(out.smartSurvival.movesFound>0) {
        BLSolution *cand=&out.smartSurvival;
        uint64_t after=cand->moves[cand->movesFound-1].after;
        if(bits(after)>32 || (combo.known&&combo.streak>0&&combo.misses>=1)) {
            BLTwoStrategies old=BLV17FindTwoStrategies(board,pieces,combo,
                                                        ctx.budget);
            BLSolution ref=old.smartSurvival;
            if(ref.movesFound>=cand->movesFound && ref.movesFound>0) {
                int oldKeeps=plan_keeps_existing_combo(&ref,combo);
                int candKeeps=plan_keeps_existing_combo(cand,combo);
                if(oldKeeps>=candKeeps) {
                    uint64_t oldAfter=ref.moves[ref.movesFound-1].after;
                    int oldQ=survival_value(oldAfter)
                        +450*worst_hard_coverage(oldAfter)
                        +550*BLResearchPairCoverage(oldAfter);
                    int candQ=survival_value(after)
                        +450*worst_hard_coverage(after)
                        +550*BLResearchPairCoverage(after);
                    if(ref.movesFound>cand->movesFound ||
                       oldKeeps>candKeeps || oldQ>candQ+1000) {
                        out.smartSurvival=ref;
                        out.smartComboPreserved=oldKeeps;
                        out.smartFutureQuality=oldQ;
                    }
                }
            }
        }
    }
#endif
    return out;
}
