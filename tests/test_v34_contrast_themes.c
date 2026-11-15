#include "../src/BLVisionContrast.h"
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
static unsigned char *im;
static void rgb(int w,int h,int x,int y,int r,int g,int b) {
    unsigned char *p=im+4*((size_t)y*w+x);
    p[0]=(unsigned char)r;p[1]=(unsigned char)g;p[2]=(unsigned char)b;p[3]=255;
}
static void paint(int w,int h,int wood,uint64_t board) {
    int seam[3]={wood?50:23,wood?48:30,wood?25:41};
    int empty[3]={wood?75:50,wood?72:56,wood?35:69};
    for(int y=0;y<h;y++)for(int x=0;x<w;x++)
        rgb(w,h,x,y,empty[0],empty[1],empty[2]);
    const int x0=45,y0=178,step=50;
    const int colors[4][3]={{197,112,58},{74,174,70},
                             {161,171,220},{208,157,53}};
    for(int r=0;r<8;r++)for(int c=0;c<8;c++) {
        int filled=(board>>(r*8+c))&1;
        for(int y=y0+r*step;y<y0+(r+1)*step;y++)
        for(int x=x0+c*step;x<x0+(c+1)*step;x++) {
            int edge=x<x0+c*step+3 || x>=x0+(c+1)*step-3 ||
                     y<y0+r*step+3 || y>=y0+(r+1)*step-3;
            if(edge)rgb(w,h,x,y,seam[0],seam[1],seam[2]);
            else if(filled) {
                const int *color=colors[(r+c)%4];
                rgb(w,h,x,y,color[0],color[1],color[2]);
            }
        }
    }
}
int main(void) {
    const int w=480,h=850;
    im=malloc((size_t)w*h*4);
    assert(im);
    const uint64_t mask=(UINT64_C(1)<<2)|(UINT64_C(1)<<7)|
         (UINT64_C(1)<<9)|(UINT64_C(1)<<19)|
         (UINT64_C(1)<<40)|(UINT64_C(1)<<51)|
         (UINT64_C(1)<<62);
    BLVisionResult geom={0};
    geom.firstX=70;geom.firstY=203;geom.lastX=420;geom.lastY=553;
    for(int wood=0;wood<=1;wood++) {
        paint(w,h,wood,mask);
        uint64_t result=0;int separation=0;
        assert(BLVisionContrastBoard(im,w,h,&geom,&result,&separation));
        assert(result==mask);
        assert(separation>=65);
        printf("PASS %s theme, seven known occupied cells, gap=%d\n",
             wood?"wood":"colored",separation);
    }
    paint(w,h,1,0);
    uint64_t result=0;
    assert(!BLVisionContrastBoard(im,w,h,&geom,&result,NULL));
    paint(w,h,1,UINT64_C(255));
    assert(!BLVisionContrastBoard(im,w,h,&geom,&result,NULL));
    free(im);
    return 0;
}
