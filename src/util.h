#ifndef UTIL_H
#define UTIL_H 1

#include <libpmemobj.h>

#define MAX_FILE_NAME_LENGTH 50

#ifndef CONTENT_LENGTH
#define CONTENT_LENGTH 128
#endif

#define LAYOUT_NAME "test_directory_tree"
POBJ_LAYOUT_BEGIN(directory_tree);
    POBJ_LAYOUT_ROOT(directory_tree,struct MyRoot);
    POBJ_LAYOUT_TOID(directory_tree,struct DirectoryTree);
    POBJ_LAYOUT_TOID(directory_tree, struct FileDescriptor);
    POBJ_LAYOUT_TOID(directory_tree, struct Content);
POBJ_LAYOUT_END(directory_tree);


// TODO: should move to pmem
typedef struct Content{
    char content[CONTENT_LENGTH];
}Content;

// TODO: should move to pmem
typedef struct FileDescriptor{
    //int mark; // 1 for file, 0 for directory 
    // TODO: other attributes shoule be considered
    TOID(struct Content) c_p; 
    int f_size; 
}FileDescriptor;

typedef struct DirectoryTree{
    char dir_name[MAX_FILE_NAME_LENGTH];
    // TODO: file descriptor
    TOID(struct FileDescriptor) fd;
    TOID(struct DirectoryTree) nextLayer;
    TOID(struct DirectoryTree) brother;
}DirectoryTree;

typedef struct MyRoot{
    TOID(struct DirectoryTree) root;
}MyRoot;

void getFatherCurPath(char* f_dst,char* s_dst,const char* path);
TOID(struct DirectoryTree) find(TOID(struct DirectoryTree) root,const char* path);
TOID(struct DirectoryTree) add(TOID(struct DirectoryTree)* root, const char* path, int mark);
int eraseNode(TOID(struct DirectoryTree)* root, const char* path);
void PrintTree(TOID(struct DirectoryTree) root);

void init();

#endif