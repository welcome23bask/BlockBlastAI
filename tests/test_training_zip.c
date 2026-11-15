// ZIP writer regression: offline ZIP is readable and rejects traversal.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../src/BLTrainingZIP.h"

int main(void){
    char directory[]="/tmp/blockblast-training-XXXXXX";
    assert(mkdtemp(directory)!=NULL);
    char path[1024],zip[1024];
    snprintf(path,sizeof(path),"%s/sample.jpg",directory);
    FILE *f=fopen(path,"wb");assert(f);
    const unsigned char jpegish[]={0xff,0xd8,0xff,0xe0,0,0x10,0x4a,0x46,0x49,0x46,0,0x01,0,0,0,0};
    assert(fwrite(jpegish,1,sizeof(jpegish),f)==sizeof(jpegish));fclose(f);
    snprintf(path,sizeof(path),"%s/sample.json",directory);
    f=fopen(path,"wb");assert(f);
    const char *json="{\"reviewed\":false,\"trainable\":false,\"board\":[0,1]}";
    assert(fwrite(json,1,strlen(json),f)==strlen(json));fclose(f);
    const char *files[]={"sample.jpg","sample.json"};
    snprintf(zip,sizeof(zip),"%s/training.zip",directory);
    assert(BLTrainingWriteZIP(zip,directory,files,2));
    const char *illegal[]={"../secrets.txt"};
    snprintf(path,sizeof(path),"%s/bad.zip",directory);
    assert(!BLTrainingWriteZIP(path,directory,illegal,1));
    puts(zip);
    return 0;
}
