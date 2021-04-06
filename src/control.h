#ifndef CONTROL_H
#define CONTROL_H 1
/*
    this file implemment the algorithm of scheduling between disk and pmem
*/
#include "util.h"

void control_init(TOID(struct DirectoryTree)* root, const char* pool_file_name,const char* root_path);
int isFlushLoad(TOID(struct DirectoryTree) node);
void flush_load(TOID(struct DirectoryTree)* root);


#endif