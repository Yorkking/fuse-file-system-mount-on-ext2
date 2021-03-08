
#define FUSE_USE_VERSION 30

#include<fuse.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>


#define BUF_SIZE 512
#define RAM_SIZE 4096
#define NAME_SIZE 50
#define File_NUM 100

char ram_content[RAM_SIZE];
char name_list[File_NUM][NAME_SIZE];




typedef struct RamFd{
    char* p;
    char* fileName;
}RamFd;

/*
存储于RAM的文件结构：

*/
typedef struct RamFile{
    RamFd* startFp;
    size_t size;
}RamFile;

void add_file(const char* filename){

}


int main(){
    

    return 0;
}

