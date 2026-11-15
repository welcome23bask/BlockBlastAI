#include "BLFuture100.h"
#include "BLGuideSafety.h"
#include <stdint.h>
#include <string.h>

typedef struct { uint64_t mask; unsigned char w,h,count; } BLShapeProbe;
static const BLShapeProbe kProbes[100]={
    { UINT64_C(0x101010101), 1, 5, 5 }, /* 1 observed */
    { UINT64_C(0x707), 3, 2, 6 }, /* 2 observed */
    { UINT64_C(0x1f), 5, 1, 5 }, /* 3 observed */
    { UINT64_C(0x1010101), 1, 4, 4 }, /* 4 observed */
    { UINT64_C(0x30303), 2, 3, 6 }, /* 5 observed */
    { UINT64_C(0x70707), 3, 3, 9 }, /* 6 observed */
    { UINT64_C(0x303), 2, 2, 4 }, /* 7 observed */
    { UINT64_C(0xf), 4, 1, 4 }, /* 8 observed */
    { UINT64_C(0x30202), 2, 3, 4 }, /* 9 observed */
    { UINT64_C(0x70101), 3, 3, 5 }, /* 10 observed */
    { UINT64_C(0x702), 3, 2, 4 }, /* 11 observed */
    { UINT64_C(0x306), 3, 2, 4 }, /* 12 observed */
    { UINT64_C(0x30101), 2, 3, 4 }, /* 13 observed */
    { UINT64_C(0x10103), 2, 3, 4 }, /* 14 observed */
    { UINT64_C(0x207), 3, 2, 4 }, /* 15 observed */
    { UINT64_C(0x10107), 3, 3, 5 }, /* 16 observed */
    { UINT64_C(0x40407), 3, 3, 5 }, /* 17 observed */
    { UINT64_C(0x20301), 2, 3, 4 }, /* 18 observed */
    { UINT64_C(0x70404), 3, 3, 5 }, /* 19 observed */
    { UINT64_C(0x407), 3, 2, 4 }, /* 20 observed */
    { UINT64_C(0x107), 3, 2, 4 }, /* 21 observed */
    { UINT64_C(0x20203), 2, 3, 4 }, /* 22 observed */
    { UINT64_C(0x704), 3, 2, 4 }, /* 23 observed */
    { UINT64_C(0x301), 2, 2, 3 }, /* 24 observed */
    { UINT64_C(0x20302), 2, 3, 4 }, /* 25 observed */
    { UINT64_C(0x103), 2, 2, 3 }, /* 26 observed */
    { UINT64_C(0x701), 3, 2, 4 }, /* 27 observed */
    { UINT64_C(0x203), 2, 2, 3 }, /* 28 observed */
    { UINT64_C(0x10302), 2, 3, 4 }, /* 29 observed */
    { UINT64_C(0x603), 3, 2, 4 }, /* 30 observed */
    { UINT64_C(0x7), 3, 1, 3 }, /* 31 observed */
    { UINT64_C(0x101), 1, 2, 2 }, /* 32 observed */
    { UINT64_C(0x10301), 2, 3, 4 }, /* 33 observed */
    { UINT64_C(0x3), 2, 1, 2 }, /* 34 observed */
    { UINT64_C(0x10101), 1, 3, 3 }, /* 35 observed */
    { UINT64_C(0x102), 2, 2, 2 }, /* 36 */
    { UINT64_C(0x201), 2, 2, 2 }, /* 37 */
    { UINT64_C(0x1), 1, 1, 1 }, /* 38 */
    { UINT64_C(0x302), 2, 2, 3 }, /* 39 */
    { UINT64_C(0x20101), 2, 3, 3 }, /* 40 */
    { UINT64_C(0x403), 3, 2, 3 }, /* 41 */
    { UINT64_C(0x10201), 2, 3, 3 }, /* 42 */
    { UINT64_C(0x20201), 2, 3, 3 }, /* 43 */
    { UINT64_C(0x601), 3, 2, 3 }, /* 44 */
    { UINT64_C(0x205), 3, 2, 3 }, /* 45 */
    { UINT64_C(0x10102), 2, 3, 3 }, /* 46 */
    { UINT64_C(0x20102), 2, 3, 3 }, /* 47 */
    { UINT64_C(0x502), 3, 2, 3 }, /* 48 */
    { UINT64_C(0x10202), 2, 3, 3 }, /* 49 */
    { UINT64_C(0x106), 3, 2, 3 }, /* 50 */
    { UINT64_C(0x304), 3, 2, 3 }, /* 51 */
    { UINT64_C(0x40201), 3, 3, 3 }, /* 52 */
    { UINT64_C(0x10204), 3, 3, 3 }, /* 53 */
    { UINT64_C(0x20103), 2, 3, 4 }, /* 54 */
    { UINT64_C(0x503), 3, 2, 4 }, /* 55 */
    { UINT64_C(0x10203), 2, 3, 4 }, /* 56 */
    { UINT64_C(0x30201), 2, 3, 4 }, /* 57 */
    { UINT64_C(0x305), 3, 2, 4 }, /* 58 */
    { UINT64_C(0x605), 3, 2, 4 }, /* 59 */
    { UINT64_C(0x30102), 2, 3, 4 }, /* 60 */
    { UINT64_C(0x506), 3, 2, 4 }, /* 61 */
    { UINT64_C(0x2010101), 2, 4, 4 }, /* 62 */
    { UINT64_C(0x1020101), 2, 4, 4 }, /* 63 */
    { UINT64_C(0x2020101), 2, 4, 4 }, /* 64 */
    { UINT64_C(0x807), 4, 2, 4 }, /* 65 */
    { UINT64_C(0xc03), 4, 2, 4 }, /* 66 */
    { UINT64_C(0x40b), 4, 2, 4 }, /* 67 */
    { UINT64_C(0x1010201), 2, 4, 4 }, /* 68 */
    { UINT64_C(0x2010201), 2, 4, 4 }, /* 69 */
    { UINT64_C(0x1020201), 2, 4, 4 }, /* 70 */
    { UINT64_C(0x30301), 2, 3, 5 }, /* 71 */
    { UINT64_C(0x30103), 2, 3, 5 }, /* 72 */
    { UINT64_C(0x10303), 2, 3, 5 }, /* 73 */
    { UINT64_C(0x20303), 2, 3, 5 }, /* 74 */
    { UINT64_C(0x703), 3, 2, 5 }, /* 75 */
    { UINT64_C(0x30203), 2, 3, 5 }, /* 76 */
    { UINT64_C(0x307), 3, 2, 5 }, /* 77 */
    { UINT64_C(0x507), 3, 2, 5 }, /* 78 */
    { UINT64_C(0x607), 3, 2, 5 }, /* 79 */
    { UINT64_C(0x705), 3, 2, 5 }, /* 80 */
    { UINT64_C(0x30302), 2, 3, 5 }, /* 81 */
    { UINT64_C(0x706), 3, 2, 5 }, /* 82 */
    { UINT64_C(0x3010101), 2, 4, 5 }, /* 83 */
    { UINT64_C(0x1030101), 2, 4, 5 }, /* 84 */
    { UINT64_C(0x2030101), 2, 4, 5 }, /* 85 */
    { UINT64_C(0x1010301), 2, 4, 5 }, /* 86 */
    { UINT64_C(0x2010301), 2, 4, 5 }, /* 87 */
    { UINT64_C(0x1020301), 2, 4, 5 }, /* 88 */
    { UINT64_C(0x2020301), 2, 4, 5 }, /* 89 */
    { UINT64_C(0xf01), 4, 2, 5 }, /* 90 */
    { UINT64_C(0x3020101), 2, 4, 5 }, /* 91 */
    { UINT64_C(0x1010103), 2, 4, 5 }, /* 92 */
    { UINT64_C(0x2010103), 2, 4, 5 }, /* 93 */
    { UINT64_C(0x1020103), 2, 4, 5 }, /* 94 */
    { UINT64_C(0x2020103), 2, 4, 5 }, /* 95 */
    { UINT64_C(0xd03), 4, 2, 5 }, /* 96 */
    { UINT64_C(0x3030101), 2, 4, 6 }, /* 97 */
    { UINT64_C(0x3010301), 2, 4, 6 }, /* 98 */
    { UINT64_C(0x1030301), 2, 4, 6 }, /* 99 */
    { UINT64_C(0x2030301), 2, 4, 6 }, /* 100 */
};
int BLFuture100ShapeCount(void) {return 100;}
int BLFuture100DescribeShape(int i,uint64_t *mask,int *width,
                              int *height,int *cells) {
    if(i<0||i>=100)return 0;
    if(mask)*mask=kProbes[i].mask;
    if(width)*width=kProbes[i].w;
    if(height)*height=kProbes[i].h;
    if(cells)*cells=kProbes[i].count;
    return 1;
}
static int possible(uint64_t board,uint64_t mask,int w,int h) {
    int count=0;
    for(int r=0;r<=8-h;++r)for(int c=0;c<=8-w;++c)
        count+=((board & (mask<<(r*8+c)))==0);
    return count;
}
BLFuture100Quality BLFuture100Measure(uint64_t board) {
    BLFuture100Quality out={0};
    for(int i=0;i<100;++i) {
        const BLShapeProbe *s=&kProbes[i];
        int n=possible(board,s->mask,s->w,s->h);
        if(n)++out.shapeAccess;
        out.cappedMobility+=n<4?n:4;
        if(i<35) {
            if(n)++out.observedAccess;
            out.observedMobility+=n<4?n:4;
        }
    }
    // Exact 4-neighbor free-cell regions reveal unusable 1-2 square traps
    // that average shape accessibility alone can fail to penalize.
    uint64_t freeCells=~board;
    const uint64_t notRightEdge=UINT64_C(0x7f7f7f7f7f7f7f7f);
    const uint64_t notLeftEdge=UINT64_C(0xfefefefefefefefe);
    while(freeCells) {
        uint64_t region=freeCells & (~freeCells+UINT64_C(1));
        for(;;) {
            uint64_t neighbors=((region&notRightEdge)<<1) |
                               ((region&notLeftEdge)>>1) |
                               (region<<8) | (region>>8);
            uint64_t expanded=region|(neighbors&freeCells);
            if(expanded==region)break;
            region=expanded;
        }
        int area=__builtin_popcountll(region);
        ++out.emptyRegions;
        if(area<=2)out.tinyEmptyCells+=area;
        if(area>out.largestEmptyRegion)out.largestEmptyRegion=area;
        freeCells&=~region;
    }
    static const struct {uint64_t mask;int w,h;} needed[]={
        {UINT64_C(0x70707),3,3},{UINT64_C(0x1f),5,1},
        {UINT64_C(0x101010101),1,5},{UINT64_C(0x707),3,2},
        {UINT64_C(0x30303),2,3}
    };
    for(int i=0;i<5;++i)
        if(possible(board,needed[i].mask,needed[i].w,needed[i].h))
            out.criticalAccess|=1<<i;
    return out;
}
static int actual_lines(const BLSolution *s) {
    int sum=0;
    for(int i=0;i<s->movesFound;++i)sum+=s->moves[i].linesCleared;
    return sum;
}
static int first_clear(const BLSolution *s) {
    for(int i=0;i<s->movesFound;++i)if(s->moves[i].linesCleared>0)
        return i+1;
    return 0;
}
static int total_pieces(const BLPiece tray[3]) {
    int total=0;
    for(int i=0;i<3;++i)total+=tray[i].count>0;
    return total;
}
static int keeps_deadline(const BLSolution *s,int latest) {
    int c=first_clear(s);
    return c>0 && c<=latest;
}
static int worthwhile(uint64_t board,const BLPiece tray[3],
                      BLComboContext combo,const BLSolution *base,
                      BLFuture100Quality before,
                      const BLSolution *candidate,
                      BLFuture100Quality *afterOut) {
    int total=total_pieces(tray);
    if(!candidate||candidate->movesFound!=total ||
       base->movesFound!=total ||
       actual_lines(candidate)<actual_lines(base) ||
       !BLGuideValidateSolution(board,tray,candidate))return 0;
    int lastTry=3;
    if(combo.known&&combo.streak>0 && combo.misses>=0&&combo.misses<=2)
        lastTry-=combo.misses;
    int bClear=keeps_deadline(base,lastTry);
    int cClear=keeps_deadline(candidate,lastTry);
    if(bClear&&!cClear)return 0;
    if(bClear && first_clear(candidate)>first_clear(base))return 0;
    uint64_t afterBoard=candidate->moves[total-1].after;
    BLFuture100Quality after=BLFuture100Measure(afterBoard);
    // Never sacrifice the last known critical shape type while chasing an
    // unverified future shape distribution.
    if((before.criticalAccess&~after.criticalAccess)!=0)return 0;
    // The first 35 probes are patterns observed in actual user screenshots.
    // Do not trade away their usability for speculative rare future shapes.
    if(after.observedAccess<before.observedAccess ||
       after.observedMobility<before.observedMobility)return 0;
    // A synthetic mobility win on hypothetical shapes is not enough to
    // justify changing a real move when the immediate line reward ties.
    // In that case demand a material gain for shapes in the user's saved
    // tray samples, or don't disturb the known-good Smart Survival route.
    if(actual_lines(candidate)==actual_lines(base) &&
       after.observedAccess<=before.observedAccess &&
       after.observedMobility<before.observedMobility+8)return 0;
    // Small disconnected empty pockets cannot hold most possible pieces.
    // Reject a candidate that increases the number of stranded cells.
    if(after.tinyEmptyCells>before.tinyEmptyCells)return 0;
    if(after.emptyRegions>before.emptyRegions &&
       after.largestEmptyRegion<before.largestEmptyRegion)return 0;
    // Require a real diversity gain, not just a tiny heuristic fluctuation.
    if(after.shapeAccess < before.shapeAccess+3)return 0;
    if(after.cappedMobility < before.cappedMobility+12)return 0;
    *afterOut=after;
    return 1;
}
BLSolution BLFuture100GuardedChoice(uint64_t board,const BLPiece tray[3],
                                     BLComboContext combo,
                                     const BLSolution *baseline,
                                     const BLSolution *candidateA,
                                     const BLSolution *candidateB,
                                     int *upgraded) {
    BLSolution blank={0};
    if(upgraded)*upgraded=0;
    if(!tray||!baseline)return blank;
    if(!BLGuideValidateSolution(board,tray,baseline))return *baseline;
    int total=total_pieces(tray);
    if(total<1||total>3||baseline->movesFound!=total)return *baseline;
    BLFuture100Quality best=BLFuture100Measure(baseline->moves[total-1].after);
    BLSolution result=*baseline;
    const BLSolution *options[2]={candidateA,candidateB};
    for(int i=0;i<2;++i) {
        BLFuture100Quality candidateQuality={0};
        if(worthwhile(board,tray,combo,&result,best,options[i],
                     &candidateQuality)) {
            result=*options[i];best=candidateQuality;
            if(upgraded)*upgraded=1;
        }
    }
    return result;
}
