/* Thirty independently measurable after-tray board signals, introduced and
 * tested in six batches of five. All disabled when their group weight is zero.
 * This file is RESEARCH ONLY. No live iPhone scanner or UI is modified.
 */
#ifndef BL_V25_FEATURES_H
#define BL_V25_FEATURES_H
#include <stdint.h>
#include <string.h>
// Only batch 4's five topology features passed the v2.5 selector.
static const int v25_weights[6]={0,0,0,1,0,0};
static const int v25_groups_enabled=6;
static int v25_min(int n,int max){return n<max?n:max;}
static uint64_t v25_rectmask(int w,int h) {
    uint64_t m=0;
    for(int r=0;r<h;r++)for(int c=0;c<w;c++)m|=UINT64_C(1)<<(8*r+c);
    return m;
}
static int v25_rectfit(uint64_t b,int w,int h){return fit(b,v25_rectmask(w,h),w,h);}
static int v25_fitmask(uint64_t b,uint64_t m,int w,int h){return fit(b,m,w,h);}
static int v25_linecount(uint64_t b,int row,int threshold) {
    int n=0;
    for(int i=0;i<8;i++){
        int count=row?bits((b>>(8*i))&255):bits(b&(UINT64_C(0x0101010101010101)<<i));
        n+=count==threshold;
    }
    return n;
}
static int v25_cross(uint64_t b){
    int count=0;
    for(int r=0;r<8;r++)if(bits((b>>(8*r))&255)==7)
      for(int c=0;c<8;c++)if(bits(b&(UINT64_C(0x0101010101010101)<<c))==7 &&
        !(b&(UINT64_C(1)<<(8*r+c))))count++;
    return count;
}
static int v25_free_run(uint64_t b,int length,int vertical){
    int n=0;
    for(int r=0;r<8;r++)for(int c=0;c<8;c++){
        if(vertical && r>8-length)continue;
        if(!vertical && c>8-length)continue;
        uint64_t mask=0;
        for(int k=0;k<length;k++)mask|=UINT64_C(1)<<(8*(r+(vertical?k:0))+c+(vertical?0:k));
        n+=!(b&mask);
    }
    return n;
}
static void v25_regions(uint64_t board,int *nregions,int *largest,int *tiny){
    const uint64_t edgeR=UINT64_C(0x7f7f7f7f7f7f7f7f);
    const uint64_t edgeL=UINT64_C(0xfefefefefefefefe);
    uint64_t free=~board;*nregions=0;*largest=0;*tiny=0;
    while(free){
        uint64_t island=free&(~free+1),old;
        do{
            old=island;
            uint64_t near=((island&edgeR)<<1)|((island&edgeL)>>1)|
                          (island<<8)|(island>>8);
            island|=near&free;
        }while(island!=old);
        free&=~island;
        int size=bits(island);++*nregions;
        if(size>*largest)*largest=size;
        if(size<=3)++*tiny;
    }
}
static int v25_3neighbor(uint64_t b){
    int n=0;
    for(int r=0;r<8;r++)for(int c=0;c<8;c++){
        int i=8*r+c;if(b&(UINT64_C(1)<<i))continue;
        int neighbors=0;
        if(r>0)neighbors+=!(b&(UINT64_C(1)<<(i-8)));
        if(r<7)neighbors+=!(b&(UINT64_C(1)<<(i+8)));
        if(c>0)neighbors+=!(b&(UINT64_C(1)<<(i-1)));
        if(c<7)neighbors+=!(b&(UINT64_C(1)<<(i+1)));
        n+=neighbors>=3;
    }
    return n;
}
static int v25_var_occupancy(uint64_t b,int rows){
    int sum=bits(b),total=0;
    for(int i=0;i<8;i++) {
        int v=rows?bits((b>>(8*i))&255):bits(b&(UINT64_C(0x0101010101010101)<<i));
        int diff=8*v-sum;
        total+=diff*diff;
    }
    return total/64;
}
static int v25_shape_diversity(uint64_t b){
    const int ws[]={3,5,1,2,3},hs[]={3,1,5,2,2};
    int n=0;for(int i=0;i<5;i++)n+=v25_rectfit(b,ws[i],hs[i])>0;
    return n;
}
static int v25_pair3x3(uint64_t b){
    uint64_t m=v25_rectmask(3,3);
    uint64_t placements[36];int n=0;
    for(int r=0;r<=5;r++)for(int c=0;c<=5;c++){
        uint64_t p=m<<(8*r+c);
        if(!(p&b))placements[n++]=p;
    }
    for(int i=0;i<n;i++)for(int j=i+1;j<n;j++)
        if(!(placements[i]&placements[j]))return 1;
    return 0;
}
/* Batch 1: emergency access for large and long placements.
 * 1 4x4 square, 2 horizontal 4x2, 3 vertical 2x4, 4 1x6, 5 6x1. */
static int v25_group1(uint64_t b){
    int s1=v25_rectfit(b,4,4),s2=v25_rectfit(b,4,2),
        s3=v25_rectfit(b,2,4),s4=v25_rectfit(b,6,1),s5=v25_rectfit(b,1,6);
    return 16*v25_min(s1,6)+6*v25_min(s2,12)+6*v25_min(s3,12)+
           10*v25_min(s4,9)+10*v25_min(s5,9);
}
/* Batch 2: irregular-piece landing diversity.
 * 6 T tetromino, 7 S zigzag, 8 Z mirror, 9 J corner, 10 L corner. */
static int v25_group2(uint64_t b){
    int a=v25_fitmask(b,UINT64_C(0x702),3,2); // .#. / ###
    int c=v25_fitmask(b,UINT64_C(0x603),3,2); // ##. / .##
    int d=v25_fitmask(b,UINT64_C(0x306),3,2); // .## / ##.
    int e=v25_fitmask(b,UINT64_C(0x10103),2,3); // ## / #. / #.
    int f=v25_fitmask(b,UINT64_C(0x20203),2,3); // ## / .# / .#
    return 8*(v25_min(a,9)+v25_min(c,9)+v25_min(d,9)+
              v25_min(e,9)+v25_min(f,9));
}
/* Batch 3: exact near-clear affordances by orientation and cross.
 * 11 row missing 1, 12 column missing 1, 13 row missing 2,
 * 14 column missing 2, 15 same-cell row-and-column intersection. */
static int v25_group3(uint64_t b){
    return 18*v25_linecount(b,1,7)+18*v25_linecount(b,0,7)+
           7*v25_linecount(b,1,6)+7*v25_linecount(b,0,6)+
           35*v25_cross(b);
}
/* Batch 4: true empty-space topology.
 * 16 free regions, 17 largest region, 18 tiny empty pockets,
 * 19 four-cell horizontal free runs, 20 four-cell vertical free runs. */
static int v25_group4(uint64_t b){
    int regions,largest,tiny;v25_regions(b,&regions,&largest,&tiny);
    int hr=v25_free_run(b,4,0),vr=v25_free_run(b,4,1);
    return -38*(regions>0?regions-1:0)+4*largest-40*tiny+
           8*v25_min(hr,15)+8*v25_min(vr,15);
}
/* Batch 5: spatial balance and contiguous mobility.
 * 21 edge free count, 22 central 4x4 free count,
 * 23 row occupancy balance, 24 column occupancy balance,
 * 25 free cells with >=3 free neighbors. */
static int v25_group5(uint64_t b){
    int borderFree=0,centralFree=0;
    for(int r=0;r<8;r++)for(int c=0;c<8;c++){
        int free=!(b&(UINT64_C(1)<<(r*8+c)));
        if(r==0||r==7||c==0||c==7)borderFree+=free;
        if(r>=2&&r<=5&&c>=2&&c<=5)centralFree+=free;
    }
    return 3*borderFree+6*centralFree-
           v25_var_occupancy(b,1)-v25_var_occupancy(b,0)+
           7*v25_3neighbor(b);
}
/* Batch 6: future-shape risk checks.
 * 26 shape-category coverage, 27 scarce square penalty,
 * 28 missing long orientation penalty, 29 eight-cell hollow-ring fit,
 * 30 two independent 3x3 landing spaces. */
static int v25_group6(uint64_t b){
    int families=v25_shape_diversity(b);
    int s3=v25_rectfit(b,3,3),h5=v25_rectfit(b,5,1),v5=v25_rectfit(b,1,5);
    int ring8=v25_fitmask(b,UINT64_C(0x70507),3,3); // 8-cell hollow 3x3 ring
    return 65*families-55*(s3<=1)-35*(h5==0||v5==0)+
           10*v25_min(ring8,12)+90*v25_pair3x3(b);
}
static int v25_extra_score(uint64_t b){
    int out=0;
    if(v25_groups_enabled>=1&&v25_weights[0])out+=v25_weights[0]*v25_group1(b);
    if(v25_groups_enabled>=2&&v25_weights[1])out+=v25_weights[1]*v25_group2(b);
    if(v25_groups_enabled>=3&&v25_weights[2])out+=v25_weights[2]*v25_group3(b);
    if(v25_groups_enabled>=4&&v25_weights[3])out+=v25_weights[3]*v25_group4(b);
    if(v25_groups_enabled>=5&&v25_weights[4])out+=v25_weights[4]*v25_group5(b);
    if(v25_groups_enabled>=6&&v25_weights[5])out+=v25_weights[5]*v25_group6(b);
    return out;
}
#endif
