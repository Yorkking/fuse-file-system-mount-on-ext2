#ifndef UTIL_H
#define UTIL_H 1

#include <libpmemobj.h>
#include <pthread.h>
#include <stdatomic.h>
#define MAX_FILE_NAME_LENGTH 128
#ifndef CONTENT_LENGTH
#define CONTENT_LENGTH 512
#endif

#define LAYOUT_NAME "test_directory_tree"
POBJ_LAYOUT_BEGIN(directory_tree);
    POBJ_LAYOUT_ROOT(directory_tree,struct MyRoot);
    POBJ_LAYOUT_TOID(directory_tree,struct DirectoryTree);
    POBJ_LAYOUT_TOID(directory_tree, struct FileDescriptor);
    POBJ_LAYOUT_TOID(directory_tree, struct Content);
POBJ_LAYOUT_END(directory_tree);


// TODO: should move to pmem:done
typedef struct Content{
    char content[CONTENT_LENGTH];
    // TODO: the length of file can be increased
    TOID(struct Content) next;
}Content;

// TODO: should move to pmem
typedef struct FileDescriptor{ 
    // TODO: other attributes shoule be considered
    int alg_cnt; // cnt for the replace algorithm
    TOID(struct Content) c_p; 
    int f_size; // just the size in pmem
    int real_size;
    int dirty;
    int isInDisk; // 0 indicate the file in pmem, 1 indicate in disk
    pthread_rwlock_t lock;
}FileDescriptor;

typedef struct DirectoryTree{
    char dir_name[MAX_FILE_NAME_LENGTH];
    atomic_uint dont_delete_count;
    // TODO: file descriptor
    TOID(struct FileDescriptor) fd; // NULL for directory, else for file
    TOID(struct DirectoryTree) nextLayer;
    TOID(struct DirectoryTree) nextLayerTail;

    TOID(struct DirectoryTree) brother;
    pthread_mutex_t file_add_lock;
}DirectoryTree;

typedef struct MyRoot{
    TOID(struct DirectoryTree) root;
    TOID(struct DirectoryTree) recycle_head;  // recycle_info
}MyRoot;

void getFatherCurPath(char* f_dst,char* s_dst,const char* path);
TOID(struct DirectoryTree) find(TOID(struct DirectoryTree) root,const char* path);
TOID(struct DirectoryTree) add(TOID(struct DirectoryTree)* root, const char* path, int mark);
int eraseNode(TOID(struct DirectoryTree)* root, const char* path);
void printTree(TOID(struct DirectoryTree) root);
int dirOrFileNode(TOID(struct DirectoryTree) node);
void freeFileContent(TOID(struct DirectoryTree)* node);
void writeToFileContent(TOID(struct DirectoryTree)* node, const char* buffer, int size);
void init(TOID(struct DirectoryTree)* root, const char* pool_file_name);

int writeContent(TOID(struct DirectoryTree)* node, const char* buffer,size_t size, off_t offset);
int readContent(TOID(struct DirectoryTree)* node, char* buffer,size_t size, off_t offset);

int fileSizeSet(TOID(struct DirectoryTree)* node,off_t length);
void resetAlg(TOID(struct DirectoryTree)* root);

void createDirNode(TOID(struct DirectoryTree)* node, int mark);

#endif