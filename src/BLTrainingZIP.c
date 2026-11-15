#include "BLTrainingZIP.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct {
    char *name;
    uint32_t offset,crc,size;
} BLZipEntry;

static int wr16(FILE *f,uint16_t value) {
    return fputc(value&255,f)!=EOF && fputc((value>>8)&255,f)!=EOF;
}
static int wr32(FILE *f,uint32_t value) {
    return wr16(f,(uint16_t)(value&65535)) &&
           wr16(f,(uint16_t)(value>>16));
}
static uint32_t crc_byte(uint32_t crc,unsigned char byte) {
    crc^=byte;
    for(int i=0;i<8;++i)
        crc=(crc>>1)^((crc&1)?UINT32_C(0xedb88320):0);
    return crc;
}
static int is_safe_name(const char *name) {
    if(!name||!*name||name[0]=='/'||strstr(name,"..")||
       strchr(name,'\\')||strlen(name)>65535)return 0;
    for(const unsigned char *p=(const unsigned char *)name;*p;++p)
        if(*p<32 || *p>=127)return 0;
    return 1;
}
static int append_entry(FILE *out,const char *base,const char *name,BLZipEntry *e) {
    if(!is_safe_name(name))return 0;
    size_t len=strlen(base)+strlen(name)+2;
    char *path=(char *)malloc(len);
    if(!path)return 0;
    snprintf(path,len,"%s/%s",base,name);
    FILE *in=fopen(path,"rb");free(path);
    if(!in)return 0;
    unsigned char buf[16384];
    uint32_t crc=UINT32_C(0xffffffff);
    uint64_t size=0;
    size_t n=0;
    while((n=fread(buf,1,sizeof(buf),in))>0) {
        for(size_t i=0;i<n;++i)crc=crc_byte(crc,buf[i]);
        size+=n;
        if(size>UINT32_MAX){fclose(in);return 0;}
    }
    if(ferror(in)||fseek(in,0,SEEK_SET)!=0){fclose(in);return 0;}
    long off=ftell(out);
    if(off<0||(unsigned long)off>UINT32_MAX){fclose(in);return 0;}
    e->offset=(uint32_t)off;e->crc=~crc;e->size=(uint32_t)size;
    e->name=(char *)malloc(strlen(name)+1);
    if(!e->name){fclose(in);return 0;}
    strcpy(e->name,name);
    // Local file header, "store" method, zero DOS timestamp.
    int ok=wr32(out,UINT32_C(0x04034b50))&&wr16(out,20)&&wr16(out,0)&&
           wr16(out,0)&&wr16(out,0)&&wr16(out,0)&&
           wr32(out,e->crc)&&wr32(out,e->size)&&wr32(out,e->size)&&
           wr16(out,(uint16_t)strlen(name))&&wr16(out,0)&&
           fwrite(name,1,strlen(name),out)==strlen(name);
    while(ok&&(n=fread(buf,1,sizeof(buf),in))>0)
        ok=fwrite(buf,1,n,out)==n;
    if(ferror(in))ok=0;
    fclose(in);
    return ok;
}
int BLTrainingWriteZIP(const char *archive, const char *folder,
                       const char *const *relativePaths, size_t count) {
    if(!archive||!folder||!relativePaths||count<1||count>65535)return 0;
    FILE *out=fopen(archive,"wb");
    if(!out)return 0;
    BLZipEntry *entries=(BLZipEntry *)calloc(count,sizeof(BLZipEntry));
    int ok=entries!=NULL;
    for(size_t i=0;ok&&i<count;++i)
        ok=append_entry(out,folder,relativePaths[i],&entries[i]);
    long start=ftell(out);
    if(start<0||(unsigned long)start>UINT32_MAX)ok=0;
    for(size_t i=0;ok&&i<count;++i) {
        BLZipEntry *e=&entries[i];
        size_t len=strlen(e->name);
        ok=wr32(out,UINT32_C(0x02014b50))&&wr16(out,20)&&wr16(out,20)&&
           wr16(out,0)&&wr16(out,0)&&wr16(out,0)&&wr16(out,0)&&
           wr32(out,e->crc)&&wr32(out,e->size)&&wr32(out,e->size)&&
           wr16(out,(uint16_t)len)&&wr16(out,0)&&wr16(out,0)&&
           wr16(out,0)&&wr16(out,0)&&wr32(out,0)&&
           wr32(out,e->offset)&&
           fwrite(e->name,1,len,out)==len;
    }
    long finish=ftell(out);
    if(finish<start||(unsigned long)finish>UINT32_MAX)ok=0;
    if(ok)ok=wr32(out,UINT32_C(0x06054b50))&&
               wr16(out,0)&&wr16(out,0)&&
               wr16(out,(uint16_t)count)&&wr16(out,(uint16_t)count)&&
               wr32(out,(uint32_t)(finish-start))&&
               wr32(out,(uint32_t)start)&&wr16(out,0);
    if(fclose(out)!=0)ok=0;
    if(entries){for(size_t i=0;i<count;++i)free(entries[i].name);free(entries);}
    if(!ok)remove(archive);
    return ok;
}
