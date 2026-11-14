// Synthetic visual regressions informed by observed skins, not exhaustive official
// theme assets. Testing multiple RGB combinations validates color-independent detection.
#include "../src/BLVisionCore.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define W 430
#define H 932
typedef struct { unsigned char v[3]; } RGB;
typedef struct { const char *name; RGB outside, border, empty, tile[3]; } Theme;
static unsigned char *rgba;
static void fill(int x0,int y0,int x1,int y1,RGB color) {
    for(int y=y0;y<y1;++y)for(int x=x0;x<x1;++x) {
        if(x<0||y<0||x>=W||y>=H)continue;
        unsigned char *p=rgba+4*((size_t)y*W+x);
        memcpy(p,color.v,3);p[3]=255;
    }
}
static void sq(int row,int col,RGB color) {
    int left=23+(int)lround(col*385.0/8.0);
    int top=216+(int)lround(row*385.0/8.0);
    int right=23+(int)lround((col+1)*385.0/8.0);
    int bottom=216+(int)lround((row+1)*385.0/8.0);
    fill(left+3,top+3,right-3,bottom-3,color);
}
static void tray(int x,int y,RGB color) {
    fill(x,y,x+19,y+19,color);
}
static void build(const Theme *t,int outline) {
    fill(0,0,W,H,t->outside);
    fill(23,216,408,601,t->border);
    for(int r=0;r<8;++r) for(int c=0;c<8;++c) {
        int x=23+(int)lround(c*385.0/8),y=216+(int)lround(r*385.0/8);
        int xx=23+(int)lround((c+1)*385.0/8), yy=216+(int)lround((r+1)*385.0/8);
        fill(x+2,y+2,xx-2,yy-2,t->empty);
    }
    sq(0,0,t->tile[0]);sq(3,4,t->tile[1]);sq(7,7,t->tile[2]);
    for(int r=0;r<3;++r)for(int c=0;c<3;++c)
        tray(58+c*21,665+r*21,t->tile[0]);
    for(int r=0;r<3;++r) {
        tray(205,665+r*21,t->tile[1]);
        tray(338,665+r*21,t->tile[2]);
    }
    tray(226,707,t->tile[1]);tray(359,707,t->tile[2]);
    if(outline) {
        // The real helper draws only thin yellow borders. The detection
        // center must stay unchanged even if the highlight is in ReplayKit.
        RGB yellow={{255,221,30}};
        int x=23+(int)lround(2*385.0/8.0),y=216+(int)lround(2*385.0/8.0);
        int xx=23+(int)lround(3*385.0/8.0),yy=216+(int)lround(3*385.0/8.0);
        fill(x+5,y+5,xx-5,y+7,yellow);
        fill(x+5,yy-7,xx-5,yy-5,yellow);
        fill(x+5,y+7,x+7,yy-7,yellow);
        fill(xx-7,y+7,xx-5,yy-7,yellow);
    }
}
static void verify(const Theme *t, const BLVisionResult *v, int flipped) {
    uint64_t board=(UINT64_C(1)<<0)|(UINT64_C(1)<<(3*8+4))|(UINT64_C(1)<<63);
    if(v->board!=board) {
        fprintf(stderr,"%s: wrong board bits: got %llx, expected %llx\n",
                t->name,(unsigned long long)v->board,(unsigned long long)board);
        abort();
    }
    assert(v->flippedInput==flipped);
    assert(v->detectedPieces==3);
    assert(v->pieces[0].count==9);
    assert(v->pieces[1].count==4 && v->pieces[2].count==4);
    for(int p=1;p<3;++p) {
        for(int j=0;j<3;++j) {
            assert(v->pieces[p].x[j]==0);
            assert(v->pieces[p].y[j]==j);
        }
        assert(v->pieces[p].x[3]==1 && v->pieces[p].y[3]==2);
    }
    assert(fabsf(v->firstX-47)<8 && fabsf(v->lastY-577)<8);
}
int main(void) {
    const Theme themes[]={
        {"wood", {{192,137,106}}, {{73,38,18}}, {{80,45,25}},
         {{{215,60,65}},{{55,175,60}},{{155,85,207}}}},
        {"pink-on-pink", {{246,186,209}}, {{125,70,94}}, {{144,91,109}},
         {{{243,125,174}},{{225,146,201}},{{253,207,112}}}},
        {"neon-purple", {{63,42,95}}, {{16,10,33}}, {{31,19,49}},
         {{{255,58,150}},{{100,230,215}},{{171,94,239}}}},
        {"coffee-and-cream", {{220,163,128}}, {{83,48,30}}, {{103,62,42}},
         {{{248,209,156}},{{217,120,163}},{{154,204,105}}}},
        {"watermelon", {{249,205,201}}, {{85,90,50}}, {{108,107,68}},
         {{{238,72,119}},{{59,193,96}},{{252,204,96}}}},
        {"monochrome-metal", {{207,203,214}}, {{64,65,71}}, {{79,80,87}},
         {{{177,179,183}},{{217,200,187}},{{128,157,197}}}},
        {"pastel-mint", {{194,233,209}}, {{86,139,121}}, {{102,157,132}},
         {{{253,219,235}},{{161,217,254}},{{234,183,226}}}}
    };
    rgba=malloc((size_t)W*H*4);assert(rgba);
    unsigned char *reversed=malloc((size_t)W*H*4);assert(reversed);
    for(size_t i=0;i<sizeof(themes)/sizeof(themes[0]);++i) {
        for(int outline=0;outline<2;++outline) {
            build(&themes[i],outline);
            BLVisionResult result={0};
            if(!BLVisionAnalyzeRGBA(rgba,W,H,&result)) {
                fprintf(stderr,"%s: scan rejected (outline=%d)\n",themes[i].name,outline);
                abort();
            }
            verify(&themes[i],&result,0);
            for(int y=0;y<H;++y)
                memcpy(reversed+4*(size_t)y*W,rgba+4*(size_t)(H-1-y)*W,(size_t)W*4);
            BLVisionResult upsideDown={0};
            if(!BLVisionAnalyzeRGBA(reversed,W,H,&upsideDown)) {
                fprintf(stderr,"%s: inverted scan rejected\n",themes[i].name);
                abort();
            }
            verify(&themes[i],&upsideDown,1);
        }
        printf("variant passed: %s\n",themes[i].name);
    }
    fill(0,0,W,H,(RGB){{246,186,209}});
    BLVisionResult blank={0};
    assert(!BLVisionAnalyzeRGBA(rgba,W,H,&blank));
    free(reversed);free(rgba);
    puts("Multi-theme vision suite passed, including persistent yellow highlight borders.");
    return 0;
}
