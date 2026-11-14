// Independent legality and board-transition property checks.
// This test deliberately does not call the solver's internal clear_lines().
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/BlockBlastSolver.h"

static uint32_t seed=0x9e3779b9u;
static uint32_t rnd(void) {
    uint32_t x=seed; x^=x<<13; x^=x>>17; x^=x<<5;
    return seed=x;
}
static const char *patterns[]={
    "#", "##", "###", "####", "#####", "#/#", "#/#/#", "#/#/#/#",
    "##/##", "###/###", "##/##/##", "###/###/###",
    "##./.##", ".##/##.", "#./##", ".#/##", ".#./###",
    "###/#.#", "#.#/###", "###/..#", "##./.##/..#"
};
static BLPiece piece(int shape) {
    BLPiece p={0};const char *a=patterns[shape];int x=0,y=0;
    for(;*a;++a) {
        if(*a=='/') {++y;x=0;continue;}
        if(*a=='#') {p.x[p.count]=x;p.y[p.count]=y;++p.count;}
        ++x;
    }
    return p;
}
static int legal(uint64_t b,const BLPiece *p,int row,int col) {
    if(row<0||col<0||row>=8||col>=8)return 0;
    for(int j=0;j<p->count;++j) {
        int x=col+p->x[j],y=row+p->y[j];
        if(x<0||y<0||x>=8||y>=8||(b&(UINT64_C(1)<<(y*8+x))))return 0;
    }
    return 1;
}
static uint64_t independent_transition(uint64_t b,const BLPiece *p,int row,int col,int *clears) {
    for(int j=0;j<p->count;++j)b|=UINT64_C(1)<<((row+p->y[j])*8+col+p->x[j]);
    uint64_t cleared=0;*clears=0;
    for(int r=0;r<8;++r) {
        int full=1;
        for(int c=0;c<8;++c) if(!(b&(UINT64_C(1)<<(r*8+c))))full=0;
        if(full) {++*clears;for(int c=0;c<8;++c)cleared|=UINT64_C(1)<<(r*8+c);}
    }
    for(int c=0;c<8;++c) {
        int full=1;
        for(int r=0;r<8;++r) if(!(b&(UINT64_C(1)<<(r*8+c))))full=0;
        if(full) {++*clears;for(int r=0;r<8;++r)cleared|=UINT64_C(1)<<(r*8+c);}
    }
    return b&~cleared;
}

static int budget;
// Independent exhaustive feasibility oracle, stops if search is too expensive.
static int exists_full(uint64_t board,const BLPiece tray[3],int used,int goal,int depth) {
    if(depth==goal)return 1;
    if(++budget>40000)return -1;
    int unknown=0;
    for(int p=0;p<3;++p) {
        if(!tray[p].count || (used&(1<<p)))continue;
        for(int row=0;row<8;++row)for(int col=0;col<8;++col) {
            if(!legal(board,&tray[p],row,col))continue;
            int clears=0;
            uint64_t next=independent_transition(board,&tray[p],row,col,&clears);
            int answer=exists_full(next,tray,used|(1<<p),goal,depth+1);
            if(answer==1)return 1;
            if(answer<0)unknown=1;
        }
    }
    return unknown?-1:0;
}
int main(void) {
    int checked=0,solvable=0,inconclusive=0;
    // Chosen independently from the original test seed.
    seed=0x7f4a7c15u;
    for(int iteration=0;iteration<450;++iteration) {
        uint64_t board=0;BLPiece tray[3]={0};
        int occupied=13+(int)(rnd()%38);
        for(int j=0;j<occupied;++j)
            board|=UINT64_C(1)<<(rnd()%64);
        int total=0;
        for(int p=0;p<3;++p) {
            tray[p]=piece((int)(rnd()%(sizeof(patterns)/sizeof(patterns[0]))));
            if((rnd()%11)==0)tray[p].count=0;
            total+=tray[p].count>0;
        }
        budget=0;
        int possible=exists_full(board,tray,0,total,0);
        if(possible<0){++inconclusive;continue;}
        BLSolution got=BLSolve(board,tray);
        assert(got.movesFound>=0&&got.movesFound<=total);
        if(possible==1) {
            ++solvable;
            if(got.movesFound!=total) {
                fprintf(stderr,"Lost full-tray plan: seed-case=%d occupied=%d expected=%d got=%d\n",
                        iteration,__builtin_popcountll(board),total,got.movesFound);
                return 1;
            }
        }
        ++checked;
    }
    printf("exhaustive feasibility comparison: checked=%d solvable=%d inconclusive=%d\n",
           checked,solvable,inconclusive);
    assert(checked>120 && solvable>20);
    return 0;
}
