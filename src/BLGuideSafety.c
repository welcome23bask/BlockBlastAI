#include "BLGuideSafety.h"
#include <stdint.h>
#include <string.h>

static int normalized_connected(const BLPiece *p){
    if(!p || p->count<=0 || p->count>25)return 0;
    unsigned char cells[5][5]={{0}},seen[5][5]={{0}};
    int minx=5,miny=5;
    for(int i=0;i<p->count;++i){
        int x=p->x[i],y=p->y[i];
        if(x<0||y<0||x>=5||y>=5||cells[y][x])return 0;
        cells[y][x]=1;if(x<minx)minx=x;if(y<miny)miny=y;
    }
    if(minx!=0 || miny!=0)return 0;
    int q[25]={0},head=0,tail=0,total=0;
    q[tail++]=p->x[0]+5*p->y[0];
    seen[p->y[0]][p->x[0]]=1;
    while(head<tail){
        int n=q[head++],x=n%5,y=n/5;
        ++total;
        const int dx[4]={1,-1,0,0},dy[4]={0,0,1,-1};
        for(int i=0;i<4;++i){
            int nx=x+dx[i],ny=y+dy[i];
            if(nx<0||nx>=5||ny<0||ny>=5||!cells[ny][nx]||seen[ny][nx])continue;
            q[tail++]=nx+ny*5;seen[ny][nx]=1;
        }
    }
    return total==p->count;
}

int BLGuideValidateSolution(uint64_t board,const BLPiece pieces[3],
                            const BLSolution *solution){
    if(!pieces||!solution || solution->movesFound<1 ||
       solution->movesFound>3)return 0;
    unsigned used[3]={0};
    uint64_t current=board;
    for(int m=0;m<solution->movesFound;++m){
        const BLMove *step=&solution->moves[m];
        int idx=step->pieceIndex;
        if(idx<0||idx>2||used[idx]++)return 0;
        const BLPiece *piece=&pieces[idx];
        if(!normalized_connected(piece) || step->before!=current ||
           step->row<0 || step->col<0 || step->row>=8 || step->col>=8)return 0;
        uint64_t placement=0;
        for(int i=0;i<piece->count;++i){
            int x=step->col+piece->x[i];
            int y=step->row+piece->y[i];
            if(x<0||x>=8||y<0||y>=8)return 0;
            uint64_t bit=UINT64_C(1)<<(y*8+x);
            if((current|placement)&bit)return 0; // illegal overlap
            placement|=bit;
        }
        uint64_t withPiece=current|placement,clear=0;
        int lines=0;
        for(int row=0;row<8;++row){
            uint64_t rowMask=UINT64_C(255)<<(8*row);
            if((withPiece&rowMask)==rowMask){clear|=rowMask;++lines;}
        }
        for(int col=0;col<8;++col){
            uint64_t columnMask=UINT64_C(0x0101010101010101)<<col;
            if((withPiece&columnMask)==columnMask){clear|=columnMask;++lines;}
        }
        current=withPiece&~clear;
        if(step->after!=current || step->linesCleared!=lines)return 0;
    }
    return 1;
}
