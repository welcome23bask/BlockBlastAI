#include "BLTrainedVision.h"
#include <math.h>
#include <stdint.h>
#include <stddef.h>

// Trained RGB logistic detector, held-out teacher-label agreement 1920/1920.
// This is a safety second opinion ONLY; not an independent ground truth.
// Restricted to the dark-purple training skin so other skins are unaffected.
static const float kW[3]={19.80822241f,20.25588506f,10.82568555f};
static const float kB=-16.37673473f;
static int sample_median(const uint8_t *im,int w,int h,int x,int y,int flip,int k){
    unsigned char values[25];int n=0;
    if(x<2||x>=w-2||y<2||y>=h-2)return -1;
    for(int dy=-2;dy<=2;++dy)for(int dx=-2;dx<=2;++dx){
        int actualY=flip ? h-1-y-dy:y+dy;
        values[n++]=im[4*((size_t)actualY*w+x+dx)+k];
    }
    for(int i=1;i<n;++i){int v=values[i],j=i-1;
        while(j>=0&&values[j]>v){values[j+1]=values[j];--j;}
        values[j+1]=(unsigned char)v;
    }
    return values[12];
}
int BLTrainedVisionOpinionRGBA(const uint8_t *im,int w,int h,
                                const BLVisionResult *native) {
    if(!im||!native||w<240||h<350)return 0;
    double sx=(native->lastX-native->firstX)/7.0;
    double sy=(native->lastY-native->firstY)/7.0;
    if(sx<10||sy<10||fabs(sx-sy)>sx*.15)return 0;
    int rgb[64][3];int dark=0,empty=0,eligible=0;
    for(int r=0;r<8;++r)for(int c=0;c<8;++c){
        int i=r*8+c;
        int x=(int)lround(native->firstX+c*sx);
        int y=(int)lround(native->firstY+r*sy);
        for(int k=0;k<3;++k)rgb[i][k]=sample_median(im,w,h,x,y,native->flippedInput,k);
        if(rgb[i][0]<0)return 0;
        if(((native->board>>i)&1U)==0) {
            ++empty;
            int R=rgb[i][0],G=rgb[i][1],B=rgb[i][2];
            if(R>=25&&R<=48 && G>=30&&G<=57 && B>=69&&B<=106)++dark;
        }
    }
    // The classifier is not known to generalize to other skins; turn it off.
    if(empty<6||dark<6||dark*5<empty*4)return 0;
    for(int i=0;i<64;++i){
        float logit=kB;
        for(int k=0;k<3;++k)logit+=kW[k]*rgb[i][k]/255.f;
        if(fabsf(logit)<1.25f)continue; // unknown, never override
        int pred=logit>0.f;
        int expected=((native->board>>i)&1U)!=0;
        if(pred!=expected)return -1;
        ++eligible;
    }
    return eligible>=32?1:0;
}
