// Regression using the exact grid and three tray pieces shown in the
// user's screenshot contrasted with blockblastsolverss.com's two approaches.
// Independent 8x8 transition oracle; not the website's proprietary engine.
#include "../src/BLDualStrategy.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static BLPiece square3(void) {
    BLPiece p={0};
    for(int r=0;r<3;++r)for(int c=0;c<3;++c) {
        int n=p.count++;p.x[n]=c;p.y[n]=r;
    }
    return p;
}
static BLPiece rightL(void) {
    BLPiece p={0};p.count=4;
    p.x[0]=1;p.y[0]=0;
    p.x[1]=1;p.y[1]=1;
    p.x[2]=0;p.y[2]=2;
    p.x[3]=1;p.y[3]=2;return p;
}
static uint64_t screenshot_board(void) {
    uint64_t b=0;
    for(int r=0;r<5;++r)for(int c=3;c<=4;++c)
        b|=UINT64_C(1)<<(r*8+c);
    for(int r=5;r<=6;++r)b|=UINT64_C(1)<<(r*8+6);
    return b;
}
static int check(const BLSolution *sol,const BLPiece tray[3],uint64_t board) {
    int used=0,clearedTotal=0;
    assert(sol->movesFound>=0&&sol->movesFound<=3);
    for(int i=0;i<sol->movesFound;++i) {
        BLMove m=sol->moves[i];
        assert(m.pieceIndex>=0&&m.pieceIndex<3);
        assert(!(used&(1<<m.pieceIndex)));
        assert(m.before==board);
        uint64_t placed=board;
        const BLPiece *p=&tray[m.pieceIndex];
        for(int j=0;j<p->count;++j) {
            int x=m.col+p->x[j],y=m.row+p->y[j];
            assert(x>=0&&x<8&&y>=0&&y<8);
            uint64_t bit=UINT64_C(1)<<(8*y+x);
            assert(!(placed&bit));placed|=bit;
        }
        int lines=0;uint64_t erased=0;
        for(int r=0;r<8;++r) {
            uint64_t mask=UINT64_C(255)<<(8*r);
            if((placed&mask)==mask) {erased|=mask;++lines;}
        }
        for(int c=0;c<8;++c) {
            uint64_t mask=UINT64_C(0x0101010101010101)<<c;
            if((placed&mask)==mask) {erased|=mask;++lines;}
        }
        board=placed&~erased;
        assert(board==m.after);
        assert(lines==m.linesCleared);
        clearedTotal+=lines;
        used|=1<<m.pieceIndex;
    }
    assert(clearedTotal==sol->totalLines);
    return clearedTotal;
}
static uint32_t seed=0x8e6b726a;
static uint32_t rnd(void) {
    uint32_t x=seed;x^=x<<13;x^=x>>17;x^=x<<5;return seed=x;
}
int main(void) {
    BLPiece tray[3]={square3(),rightL(),square3()};
    uint64_t board=screenshot_board();
    BLComboContext due={.streak=37,.misses=2,.known=1};
    BLTwoStrategies options=BLFindTwoStrategies(board,tray,due,500000);
    assert(options.exhaustive);
    assert(options.maxLines.movesFound==3);
    assert(options.keepCombo.movesFound==3);
    assert(check(&options.maxLines,tray,board)==5);
    assert(check(&options.keepCombo,tray,board)==2);
    assert(options.maxFirstClear==3);
    assert(options.comboFirstClear==1);
    assert(!options.maxComboPreserved);
    assert(options.keepComboPreserved);
    printf("Screenshot reproduced: max clears=%d, first clear step=%d; combo clears=%d, first clear step=%d; explored=%u\n",
           options.maxLines.totalLines,options.maxFirstClear,
           options.keepCombo.totalLines,options.comboFirstClear,options.exploredNodes);
    // Test hundreds of random partially occupied boards; approximate plans
    // still must be legal and must not invent lines or place a piece twice.
    for(int i=0;i<160;++i) {
        uint64_t b=0;
        for(int k=0;k<i%48;++k)b|=UINT64_C(1)<<(rnd()%64);
        BLPiece p[3]={square3(),rightL(),square3()};
        if(i%4==0)p[0].count=0;
        if(i%5==0)p[2].count=0;
        BLTwoStrategies result=BLFindTwoStrategies(b,p,due,6000);
        check(&result.maxLines,p,b);
        check(&result.keepCombo,p,b);
        assert(result.exploredNodes<=6000);
    }
    puts("Dual-strategy screenshot and 160 randomized legality tests passed.");
    return 0;
}
