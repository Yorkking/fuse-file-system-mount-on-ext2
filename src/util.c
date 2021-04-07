#include "util.h"
#include<string.h>
#include<stdlib.h>
#include<stdio.h>

//#define __NAME__MAIN__TEST__

//#define pool_file_name "/home/ubuntu/shuitang/GraduationProject/wykfs/wykfs.pmem"

PMEMobjpool* DirectoryTreePop;

void init(TOID(struct DirectoryTree)* root, const char* pool_file_name){
    DirectoryTreePop = pmemobj_open(pool_file_name, LAYOUT_NAME);
    if(DirectoryTreePop == NULL){
        DirectoryTreePop = pmemobj_create(pool_file_name,LAYOUT_NAME, PMEMOBJ_MIN_POOL,0666);
        if(DirectoryTreePop == NULL){
            perror("pmemobj_create");
            return;
        }
        TOID(struct MyRoot) root_obj = POBJ_ROOT(DirectoryTreePop,struct MyRoot);
        (*root) = D_RW(root_obj)->root;
        if(TOID_IS_NULL(add(root,"/",0))){
            perror("create root directory fail");
        }
        D_RW(root_obj)->root = (*root);
    }else{
        TOID(struct MyRoot) root_obj = POBJ_ROOT(DirectoryTreePop,struct MyRoot);
        (*root) = D_RW(root_obj)->root;
    }

    return;
}
/* mark == 1 for file, 0 for directory */
void createDirNode(TOID(struct DirectoryTree)* node, int mark){
    // TODO: add the FileDescriptor , done
    TX_BEGIN(DirectoryTreePop){
        //TX_ADD(*node);
        *node = TX_NEW(struct DirectoryTree);
        D_RW(*node)->nextLayer = TOID_NULL(struct DirectoryTree);
        D_RW(*node)->brother = TOID_NULL(struct DirectoryTree);
        if(mark){
            D_RW(*node)->fd = TX_NEW(struct FileDescriptor);

            //D_RW(D_RW(*node)->fd)->c_p = TX_NEW(struct Content);
            //D_RW(D_RW(D_RW(*node)->fd)->c_p)->next = TOID_NULL(struct Content);
            D_RW(D_RW(*node)->fd)->c_p = TOID_NULL(struct Content);

            D_RW(D_RW(*node)->fd)->alg_cnt = 0;
            D_RW(D_RW(*node)->fd)->dirty = 0;
            D_RW(D_RW(*node)->fd)->f_size = 0;
            D_RW(D_RW(*node)->fd)->isInDisk = 0;
        }else{
            D_RW(*node)->fd = TOID_NULL(struct FileDescriptor);
        }

    }TX_END
}
void freeDirNode(TOID(struct DirectoryTree)* node){
    // TODO: add the FileDescriptor, done
    TX_BEGIN(DirectoryTreePop){
        //TX_ADD(*node);
        if(!TOID_IS_NULL(D_RW(*node)->fd)){

            TOID(struct Content) head = D_RW(D_RW(*node)->fd)->c_p;
            while(!TOID_IS_NULL(head)){
                TOID(struct Content) temp = head;
                head = D_RW(head)->next;
                TX_FREE(temp);
                temp = TOID_NULL(struct Content);
            }
            /*
            TX_FREE(D_RW(D_RW(*node)->fd)->c_p);
            D_RW(D_RW(*node)->fd)->c_p = TOID_NULL(struct Content);
            */

            TX_FREE(D_RW(*node)->fd);
            D_RW(*node)->fd = TOID_NULL(struct FileDescriptor);
        }
        TX_FREE(*node);
        *node = TOID_NULL(struct DirectoryTree);
    }TX_END
}

void freeFileContent(TOID(struct DirectoryTree)* node){
    TX_BEGIN(DirectoryTreePop){
        TOID(struct Content) head = D_RW(D_RW(*node)->fd)->c_p;
        while(!TOID_IS_NULL(head)){
            TOID(struct Content) temp = head;
            head = D_RW(head)->next;
            TX_FREE(temp);
            temp = TOID_NULL(struct Content);
        }
        D_RW(D_RW(*node)->fd)->isInDisk = 1;
    }TX_END  
}
int getContentBlocks(int f_size){
    return f_size / CONTENT_LENGTH + ((f_size % CONTENT_LENGTH == 0) ? 0 : 1);
}
int writeContent(TOID(struct DirectoryTree)* node, const char* buffer,size_t size, off_t offset){
    int f_size = D_RW(D_RW(*node)->fd)->f_size;
    if(offset > f_size) {
        offset = f_size;
    }
    int file_length = offset + size;
    int cur_blocks = getContentBlocks(f_size);
    int need_blocks = getContentBlocks(file_length) - cur_blocks;

    // allocate storage space
    TOID(struct Content) head = D_RW(D_RW(*node)->fd)->c_p;
    TOID(struct Content) tail = TOID_NULL(struct Content);
    while(!TOID_IS_NULL(head)){
        if(TOID_IS_NULL(tail)) tail = head;
        head = D_RW(head)->next;
    }
    if(TOID_IS_NULL(tail)){
         TX_BEGIN(DirectoryTreePop){
            tail = TX_NEW(struct Content);
            D_RW(D_RW(*node)->fd)->c_p = tail;
        }TX_END
        need_blocks--;
    }
    while(need_blocks--){
        TX_BEGIN(DirectoryTreePop){
            TOID(struct Content) tmp_node = TX_NEW(struct Content);
            D_RW(tail)->next = tmp_node;
            tail =  D_RW(tail)->next;
        }TX_END
    }
    // write to pmem
    head = D_RW(D_RW(*node)->fd)->c_p;
    int cur = 0;
    int buffer_cur_idx = 0;
    while(!TOID_IS_NULL(head) && cur < offset + size){
        if(cur <= offset && cur + CONTENT_LENGTH > offset){
            int start_p = offset - cur;
            int end_p = (offset + size) > (cur + CONTENT_LENGTH) ? CONTENT_LENGTH-1 : offset + size - 1 - cur;
            memcpy(D_RW(head)->content + start_p, buffer,end_p - start_p + 1);
            buffer_cur_idx += end_p - start_p + 1;
        }else if(cur > offset && cur + CONTENT_LENGTH <= offset + size){
            memcpy(D_RW(head)->content,buffer+buffer_cur_idx,CONTENT_LENGTH);
            buffer_cur_idx += CONTENT_LENGTH;
        }else if(cur > offset && cur + CONTENT_LENGTH > offset + size){
            int end_p = offset + size -1 - cur;
            memcpy(D_RW(head)->content, buffer+buffer_cur_idx,end_p+1);
            buffer_cur_idx += end_p+1;
        }
        cur += CONTENT_LENGTH;
        head = D_RW(head)->next;
    }
    D_RW(D_RW(*node)->fd)->f_size = f_size > offset + buffer_cur_idx ? f_size : offset + buffer_cur_idx;
    D_RW(D_RW(*node)->fd)->dirty = 1;
    return buffer_cur_idx;

}

int readContent(TOID(struct DirectoryTree)* node, char* buffer,size_t size, off_t offset){

    if(offset>D_RW(D_RW(*node)->fd)->f_size) return -1;
    int f_size = D_RW(D_RW(*node)->fd)->f_size;
    TOID(struct Content) head = D_RW(D_RW(*node)->fd)->c_p;
    int cur = 0;
    int buffer_cur_idx = 0;
    int last_size = offset + size >= f_size ? f_size : offset + size;
    while(!TOID_IS_NULL(head) && cur < last_size){
        if(cur <= offset && cur + CONTENT_LENGTH > offset){
            int start_p = offset - cur;
            int end_p = (last_size) > (cur + CONTENT_LENGTH) ? CONTENT_LENGTH-1 : last_size - 1 - cur;
            memcpy(buffer,D_RW(head)->content + start_p, end_p - start_p + 1);
            buffer_cur_idx += end_p - start_p + 1;
        }else if(cur > offset && cur + CONTENT_LENGTH <= last_size){
            memcpy(buffer+buffer_cur_idx,D_RW(head)->content,CONTENT_LENGTH);
            buffer_cur_idx += CONTENT_LENGTH;
        }else if(cur > offset && cur + CONTENT_LENGTH > last_size){
            int end_p = last_size -1 - cur;
            memcpy(buffer+buffer_cur_idx,D_RW(head)->content, end_p+1);
            buffer_cur_idx += end_p+1;
        }
        cur += CONTENT_LENGTH;
        head = D_RW(head)->next;
    }
    return buffer_cur_idx;
}

void writeToFileContent(TOID(struct DirectoryTree)* node, const char* buffer, int size){
    // TODO: dynamic size of file
    if(TOID_IS_NULL(D_RW(D_RW(*node)->fd)->c_p)){
        TX_BEGIN(DirectoryTreePop){
            D_RW(D_RW(*node)->fd)->c_p = TX_NEW(struct Content);
            strcpy(D_RW(D_RW(D_RW(*node)->fd)->c_p)->content,buffer);

            D_RW(D_RW(*node)->fd)->f_size = size;
            // for algorithm
            D_RW(D_RW(*node)->fd)->dirty = 1;
            D_RW(D_RW(*node)->fd)->alg_cnt += 2;
        }TX_END
    }else{
        TX_BEGIN(DirectoryTreePop){
            strcpy(D_RW(D_RW(D_RW(*node)->fd)->c_p)->content,buffer);

            D_RW(D_RW(*node)->fd)->f_size = size;
            
            // for algorithm
            D_RW(D_RW(*node)->fd)->dirty = 1;
            D_RW(D_RW(*node)->fd)->alg_cnt += 2;
        }TX_END
    }
}

void getFatherCurPath(char* f_dst,char* s_dst,const char* path){
        if(strcmp(path,"/") == 0){
        strcpy(f_dst,path);
        return;
    }
    int i = strlen(path)-1;
    while(path[i] != '/') i--;
    if(i == 0){
        strncpy(f_dst,path,i+1);
        f_dst[i+1] = '\0';
    }
    else{
        strncpy(f_dst,path,i);
        f_dst[i] = '\0';
    }
    strncpy(s_dst,path+i+1, strlen(path)  - i);
}

TOID(struct DirectoryTree) find(TOID(struct DirectoryTree) root,const char* path){

    if(TOID_IS_NULL(root)) return TOID_NULL(struct DirectoryTree);
    if(path[0] == '/' && strcmp(D_RW(root)->dir_name,"/") == 0){
        if(strcmp(path,"/") == 0 ) return root;
        return find(D_RW(root)->nextLayer,path+1);
    }else{
        int i= 0;
        char tempName[MAX_FILE_NAME_LENGTH] = {'\0'};
        while(*path != '/' && *path){
            tempName[i] = *path;
            path++;
            i++;
        }
        tempName[i] = '\0';
        TOID(struct DirectoryTree) head = root;
        
        while(!TOID_IS_NULL(head)){
            if(strcmp(D_RW(head)->dir_name,tempName) == 0){
                break;
            }
            head = D_RW(head)->brother;
        }
        if( !*path || TOID_IS_NULL(head)){// encounter the '/0' or don't exist such directory or file
            return head;
        }
        return find(D_RW(head)->nextLayer,path+1); // +1 to eliminate the '/' 
    }
}

/* mark == 1 for file, 0 for directory */
TOID(struct DirectoryTree) add(TOID(struct DirectoryTree)* root, const char* path, int mark){
     if(strcmp(path,"/") == 0){
        if(TOID_IS_NULL(*root)){
            createDirNode(root,mark);
            strcpy(D_RW(*root)->dir_name,"/");
            return *root;
        }else{
            return TOID_NULL(struct DirectoryTree);
        }
    }
    char temp[MAX_FILE_NAME_LENGTH];
    int i;
    for(i=strlen(path)-1;i>=0 && path[i] != '/';--i){
    }
    int flag = i == 0?1: 0;
    
    for(int j=0;j<i+flag;++j){
        temp[j] = path[j];
    }
    temp[i+flag] = '\0';
    TOID(struct DirectoryTree) node = find(*root,temp);
    if(TOID_IS_NULL(node)){
        return node;
    }
    // TODO: should consider the atomic
    TOID(struct DirectoryTree) newNode;
    createDirNode(&newNode,mark);
    strcpy(D_RW(newNode)->dir_name,path+i+1);
    TOID(struct DirectoryTree) tempP;
    createDirNode(&tempP,mark);

    TOID(struct DirectoryTree) t_p = tempP;
    D_RW(tempP)->brother = D_RW(node)->nextLayer;

    while(!TOID_IS_NULL(D_RW(tempP)->brother)){
        if(strcmp(D_RW(D_RW(tempP)->brother)->dir_name,D_RW(newNode)->dir_name) == 0){
            freeDirNode(&newNode);
            return TOID_NULL(struct DirectoryTree);
        }
        tempP = D_RW(tempP)->brother;
    }
    D_RW(tempP)->brother = newNode;
    D_RW(node)->nextLayer = D_RW(t_p)->brother;

    freeDirNode(&t_p);
    return newNode;
}

void eraseTree(TOID(struct DirectoryTree)* root){
    if(!TOID_IS_NULL(*root)){
        TOID(struct DirectoryTree) temp = D_RW(*root)->nextLayer;
    
        freeDirNode(root);
        while(!TOID_IS_NULL(temp)){
            TOID(struct DirectoryTree) t = temp;
            eraseTree(&t);
            temp = D_RW(temp)->brother;
        }
    }
}

int eraseNode(TOID(struct DirectoryTree)* root, const char* path){
    if(strcmp(D_RW(*root)->dir_name,"/") == 0 && strcmp(path,"/") == 0){
        eraseTree(root);
        return 1;
    }

    char father[MAX_FILE_NAME_LENGTH] = {'\0'};
    char son[MAX_FILE_NAME_LENGTH] = {'\0'};
    getFatherCurPath(father,son,path);
    
    TOID(struct DirectoryTree) node =  find(*root,father); // find its father
    //printf("141----%s\n",father);
    if(TOID_IS_NULL(node)) return -1; // failure

    TOID(struct DirectoryTree) tempP;
    createDirNode(&tempP,0);
    TOID(struct DirectoryTree) t_p = tempP;
    D_RW(tempP)->brother = D_RW(node)->nextLayer;
    TOID(struct DirectoryTree) head = D_RW(node)->nextLayer;

    while(!TOID_IS_NULL(head) && strcmp(D_RW(head)->dir_name,son)){
        tempP = D_RW(tempP)->brother;
        head = D_RW(head)->brother;
    }
    if(TOID_IS_NULL(head)) return -1;
    D_RW(tempP)->brother = D_RW(head)->brother;
    D_RW(node)->nextLayer = D_RW(t_p)->brother;
    //(node->fd->f_size)--;  // directory size decrease
    eraseTree(&head);
    freeDirNode(&t_p);
    return 1;
}

void printTree(TOID(struct DirectoryTree) root){
    if(!TOID_IS_NULL(root)){
        TOID(struct DirectoryTree) head = root;
        while(!TOID_IS_NULL(head)){
            if(dirOrFileNode(head) == 0){
                printf("dir name: %s\n",D_RW(head)->dir_name);
                printTree(D_RW(head)->nextLayer);
                printf("-----\n");
            }
            else printf("--file name:%s, algcnt: %d, dirty: %d, isDisk: %d\n",D_RW(head)->dir_name, 
                    D_RW(D_RW(head)->fd)->alg_cnt,D_RW(D_RW(head)->fd)->dirty, D_RW(D_RW(head)->fd)->isInDisk);
            head = D_RW(head)->brother;
        }
    }
}

/* return 0 for dir, 1 for file */
int dirOrFileNode(TOID(struct DirectoryTree) node){
    return TOID_IS_NULL(D_RW(node)->fd) ? 0 : 1;
}

void resetAlg(TOID(struct DirectoryTree)* root){
    D_RW(D_RW(*root)->fd)->alg_cnt = 0;
}

#ifdef __NAME__MAIN__TEST__
int main(){
    char file_name[] = LAYOUT_NAME;
    DirectoryTreePop = pmemobj_open(file_name,LAYOUT_NAME);
    TOID(struct MyRoot) root_obj;
    TOID(struct DirectoryTree) root;

    int mark = 0;

    if(DirectoryTreePop == NULL){
        DirectoryTreePop = pmemobj_create(file_name,LAYOUT_NAME, PMEMOBJ_MIN_POOL,0666);
        if(DirectoryTreePop == NULL){
            perror("pmemobj_create");
            return 1;
        }
        root_obj = POBJ_ROOT(DirectoryTreePop,struct MyRoot);
        root = TOID_NULL(struct DirectoryTree);
        D_RW(root_obj)->root = root;
        TOID(struct DirectoryTree) node = TOID_NULL(struct DirectoryTree);
        node = add(&root,"/",mark);
        node = add(&root,"/a",mark);
        node = add(&root,"/a/b",mark);
        node = add(&root,"/c",mark);
        node = add(&root,"/d",mark);
        node = add(&root,"/c/a",mark);
        node = add(&root,"/d/wyk",mark);
        node = add(&root,"/d/wxr",mark);
        node = add(&root,"/d/xr",mark);
        if(TOID_IS_NULL(node = add(&root,"/d/xr",mark))) printf("error! exits!\n");
        if(TOID_IS_NULL(node = add(&root,"/d/wyk/like/xr",mark))) printf("error! \n");
        printTree(root);
        if(eraseNode(&root,"/d")>0){
            printf("success\n");
            printTree(root);
        }else{
            printf("error!\n");
            printTree(root);
        }
        D_RW(root_obj)->root = root;
    }else{
        root_obj = POBJ_ROOT(DirectoryTreePop,struct MyRoot);
        root = D_RW(root_obj)->root;
        printf("260---------\n");
        if(eraseNode(&root,"/")>0){
            printf("success\n");
            printTree(root);
        }else{
            printf("error!\n");
            printTree(root);
        }
        printTree(root);
    }
    
    pmemobj_close(DirectoryTreePop);
}
#endif