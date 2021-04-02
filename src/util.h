#ifndef UTIL_H
#define UTIL_H 1

#define MAX_FILE_NAME_LENGTH 50

#ifndef CONTENT_LENGTH
#define CONTENT_LENGTH 128
#endif

typedef struct Content{
    char content[CONTENT_LENGTH];
}Content;

typedef struct FileDescriptor{
    int mark; // 1 for file, 0 for directory 
    // TODO: other attributes shoule be considered
    Content* c_p; 
    int f_size; 
}FileDescriptor;

typedef struct DirectoryTree{
    char dir_name[MAX_FILE_NAME_LENGTH];
    // TODO: file descriptor : done
    FileDescriptor* fd;
    struct DirectoryTree* nextLayer;
    struct DirectoryTree* brother;
}DirectoryTree;


DirectoryTree* find(DirectoryTree* root,const char* path);
DirectoryTree* add(DirectoryTree** root, const char* path);
void eraseTree(DirectoryTree** root);
void getFatherCurPath(char* f_dst,char* s_dst,const char* path);
int eraseNode(DirectoryTree** root, const char* path);
void PrintTree(DirectoryTree* root);




#endif