// Non-identifying, manually checked 64 cell-center RGB samples from the
// user's NEW 41-capture ZIP. Screenshots and provisional JSON stay local.
// The original unreviewed scanner missed 8+ FILLED squares in each of
// several mixed-color examples; this regression uses VISUALLY checked masks.
#include "../src/BLVisionContrast.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static int nibble(char c){
    if(c>='0'&&c<='9')return c-'0';
    if(c>='a'&&c<='f')return c-'a'+10;
    if(c>='A'&&c<='F')return c-'A'+10;
    return -1;
}
static void evaluate(const char *name,const char *hex,uint64_t expected,
                     int minNewlyRecovered,int expectEmpty){
    uint8_t rgb[64][3];
    assert(strlen(hex)==384);
    for(int i=0;i<192;i++){
        int hi=nibble(hex[2*i]),lo=nibble(hex[2*i+1]);
        assert(hi>=0&&lo>=0);
        rgb[i/3][i%3]=(uint8_t)(hi*16+lo);
    }
    BLVisionLumaEvidence proof={0};
    int good=BLVisionLumaFromCenters((const uint8_t (*)[3])rgb,&proof);
    if(expectEmpty){
        assert(!good); // all-empty board cannot certify target cells
        printf("PASS %s: no fabricated foreground, no unsafe empty proof\n",name);
        return;
    }
    assert(good);
    assert(proof.filled==expected);
    assert((proof.filled&proof.definitelyEmpty)==0);
    assert(__builtin_popcountll(proof.filled)>=minNewlyRecovered);
    assert(proof.separation>=50);
    printf("PASS %s: visually checked %d occupied, %d certain empty, gap %d\n",
       name,__builtin_popcountll(proof.filled),
       __builtin_popcountll(proof.definitelyEmpty),proof.separation);
}
int main(void){
    evaluate("gray/purple missed 8", 
      "90b9bd2f32392f32392f32392f32392f32392f32392f323991babe2f32392f3239d9999aa694bea694bea795bf2f323990b9bd2f32392f32392f32392f32397b8c6c7b8c6c7b8c6c2f32392f32392f3239c9b68b2f32392f32392f32392f3239d9999ad898992f32392f32392f32392f32392f32392f3239d79899d7999a2f32392f32392f32392f32392f32392f3239d89899d999992f3239a795bfa694be2f32392f32392f32392f32392f32392f3239d9999a2f32392e32392f32392f3239",
      UINT64_C(0x081b030308e17901),20,0);
    evaluate("bright multi-skin missed 18",
      "90b9bda694bc90b9bd2f32392f32392f32392f32392f323990b9bda795bd90b9bdd9999aa795bfa795bea694be2f323991b8bda795bd90b9bd2f3239ce98807b8c6c7b8c6c7b8c6c2f3239a795bd90b8bdc9b68bce9880ce9880cd977fce98802f3239a795be91b8bd2f3239ce9880ce9880ce987f7b8c6cd89899d9999a2f32392f32397b8c6c2f32392f32397b8c6cd99999d9999a2f3239a795bfa795bf2f32392f32397b8c6c2e33392f32392f3239d9999a2f32392f32392f32397b8c6c",
      UINT64_C(0x889b93f6fef77f07),41,0);
    evaluate("green theme",
      "164137164138164137164137164137164137164137164137164137164138164137164137164137164137164137164137164137164138164137164137164137164137164137164137164137164137e2434be2454ae2444ae1454ae2454beaaa10164137164138f37928f37828f37828f57a2af57a2ae9aa0f1641371641372caacb2daacb2eaacc2dabcd2fabceeaaa1016413716413716413716413716413716413716413716413716413716413716413816413732b84e33ba4f164137164137",
      UINT64_C(0x3000fcfcfc000000),20,0);
    evaluate("fully empty wood theme",
      "45382a45382946382b45382945372a45372a45372945372a44382945382a46382946382b45372945372a44372845372a45372a45382946382b45372a45382945372a45382845382946392945382844382845372a45382845382846382a46382945372944372945372a45372a45372a45372a45372845382a45372a45382945372a45372945382945372a45382845372a45382945382945372945372945382845372a45382845372944362945372a44382a45372a443727443729453729443629",
      UINT64_C(0),0,1);
    puts("PASS reviewed real color samples: miss-prone themes no longer treated as empty");
    return 0;
}
