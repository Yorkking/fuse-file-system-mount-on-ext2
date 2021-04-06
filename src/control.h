#ifndef CONTROL_H
#define CONTROL_H 1
/*
    this file implemment the algorithm of scheduling between disk and pmem
*/
#include "util.h"

void control_init(const char* root_path);
int isFlush(TOID(struct DirectoryTree) node);
void flush_to_disk(TOID(struct DirectoryTree)* root);
void load_to_pmem(TOID(struct DirectoryTree)* root, const char* path);

#endif