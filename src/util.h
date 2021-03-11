#ifndef UTIL_H
#define UTIL_H 1

#define MAX_FILE_NAME_LENGTH 50
//extern int hash_file_fd(char* file_name);

typedef struct DirectoryTree{
    char dir_name[MAX_FILE_NAME_LENGTH];
    // TODO: file descriptor
    struct DirectoryTree* nextLayer;
    struct DirectoryTree* brother;
}DirectoryTree;

DirectoryTree* find(DirectoryTree* root,char* path);
DirectoryTree* add(DirectoryTree** root, char* path);
void eraseTree(DirectoryTree** root);
void getFatherCurPath(char* f_dst,char* s_dst,char* path);
int eraseNode(DirectoryTree** root, char* path);
void PrintTree(DirectoryTree* root);

#endif