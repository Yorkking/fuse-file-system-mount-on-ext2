#include "util_pm.h"
#include<string.h>
#include<stdlib.h>
#include<stdio.h>

//#define __NAME__MAIN__TEST__
void init(PMEMobjpool* pop){
    DirectoryTreePop = pop;
}
void createDirNode(TOID(struct DirectoryTree)* node){
    // TODO: add the FileDescriptor 
    TX_BEGIN(DirectoryTreePop){
        //TX_ADD(*node);
        *node = TX_NEW(struct DirectoryTree);
        D_RW(*node)->nextLayer = TOID_NULL(struct DirectoryTree);
        D_RW(*node)->brother = TOID_NULL(struct DirectoryTree);
    }TX_END
}
void freeDirNode(TOID(struct DirectoryTree)* node){
    // TODO: add the FileDescriptor
    TX_BEGIN(DirectoryTreePop){
        //TX_ADD(*node);
        TX_FREE(*node);
        *node = TOID_NULL(struct DirectoryTree);
    }TX_END
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
TOID(struct DirectoryTree) add(TOID(struct DirectoryTree)* root, const char* path){
     if(strcmp(path,"/") == 0 && TOID_IS_NULL(*root)){
        createDirNode(root);
        strcpy(D_RW(*root)->dir_name,"/");
        return *root;
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
    createDirNode(&newNode);
    strcpy(D_RW(newNode)->dir_name,path+i+1);
    TOID(struct DirectoryTree) tempP;
    createDirNode(&tempP);

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
    createDirNode(&tempP);
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
void PrintTree(TOID(struct DirectoryTree) root){
    if(!TOID_IS_NULL(root)){
        TOID(struct DirectoryTree) head = root;
        while(!TOID_IS_NULL(head)){
            printf("name: --- %s\n",D_RW(head)->dir_name);
            PrintTree(D_RW(head)->nextLayer);
            printf("-----\n");
            head = D_RW(head)->brother;
        }
    }
}

#ifdef __NAME__MAIN__TEST__
int main(){
    char file_name[] = LAYOUT_NAME;
    DirectoryTreePop = pmemobj_open(file_name,LAYOUT_NAME);
    TOID(struct MyRoot) root_obj;
    TOID(struct DirectoryTree) root;
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
        node = add(&root,"/");
        node = add(&root,"/a");
        node = add(&root,"/a/b");
        node = add(&root,"/c");
        node = add(&root,"/d");
        node = add(&root,"/c/a");
        node = add(&root,"/d/wyk");
        node = add(&root,"/d/wxr");
        node = add(&root,"/d/xr");
        if(TOID_IS_NULL(node = add(&root,"/d/xr"))) printf("error! exits!\n");
        if(TOID_IS_NULL(node = add(&root,"/d/wyk/like/xr"))) printf("error! \n");
        PrintTree(root);
        if(eraseNode(&root,"/d")>0){
            printf("success\n");
            PrintTree(root);
        }else{
            printf("error!\n");
            PrintTree(root);
        }
        D_RW(root_obj)->root = root;
    }else{
        root_obj = POBJ_ROOT(DirectoryTreePop,struct MyRoot);
        root = D_RW(root_obj)->root;
        printf("250---------\n");
        if(eraseNode(&root,"/")>0){
            printf("success\n");
            PrintTree(root);
        }else{
            printf("error!\n");
            PrintTree(root);
        }
        PrintTree(root);
    }
    
    pmemobj_close(DirectoryTreePop);
}
#endif