#include "util.h"
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
//#define __NAME__MAIN_TEST__

#ifndef MAX_FILE_NAME_LENGTH
#define MAX_FILE_NAME_LENGTH 50
#endif

void createDirNode(DirectoryTree** node){
    *node = (DirectoryTree*)(malloc(sizeof(DirectoryTree)));
    (*node)->fd = (FileDescriptor*)(malloc(sizeof(FileDescriptor)));
    (*node)->fd->c_p = (Content*)(malloc(sizeof(Content)));
    (*node)->nextLayer = NULL;
    (*node)->nextLayer = NULL;

}
void freeDirNode(DirectoryTree** node){
    free((*node)->fd->c_p);
    (*node)->fd->c_p = NULL;
    free((*node)->fd);
    (*node)->fd = NULL;
    free(*node);
    *node == NULL;
}

DirectoryTree* find(DirectoryTree* root,const char* path){
    // suppose the length of the fileName is less than MAX_FILE_NAME_LENGTH
    // according the '/' to split the path and search it.
    // current layer to find if the directory exists.
    if(root == NULL) return NULL;
    if(path[0] == '/' && strcmp(root->dir_name,"/") == 0){ // from root
        if(strcmp(path,"/") == 0) return root;
        return find(root->nextLayer,path+1);
    }else{
        int i = 0;
        char tempName[MAX_FILE_NAME_LENGTH] = {'\0'};
        while(*path != '/' && *path){
            tempName[i] = *path;
            path++;
            i++;
        }
        tempName[i] = '\0';
        DirectoryTree* head = root;
        
        while(head){
            if(strcmp(head->dir_name,tempName) == 0){
                break;
            }
            head = head->brother;
        }
        if( !*path || !head){// encounter the '/0' or don't exist such directory or file
            return head;
        }
        return find(head->nextLayer,path+1); // +1 to eliminate the '/' 
    }
}

// add one file or directory
DirectoryTree* add(DirectoryTree** root, const char* path){
    //printf("56 ___ %s\n",path);
    if(strcmp(path,"/") == 0 && *root == NULL){
        /*
        *root = (DirectoryTree*)(malloc(sizeof(DirectoryTree)));
        (*root)->brother = NULL;
        (*root)->nextLayer = NULL;
        (*root)->fd = (FileDescriptor*)(malloc(sizeof(FileDescriptor)));
        */
        createDirNode(root);
        strcpy((*root)->dir_name,"/");
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
    //printf("74 ___ %s\n",temp);
    DirectoryTree* node = find(*root,temp);
    if(node == NULL){
        //printf("78-----NULL\n");
        return NULL;
    }
    /*
    DirectoryTree* newNode = (DirectoryTree*)(malloc(sizeof(DirectoryTree)));
    newNode->brother = NULL;
    newNode->nextLayer = NULL;
    newNode->fd = (FileDescriptor*)(malloc(sizeof(FileDescriptor)));
    */
    DirectoryTree* newNode;
    createDirNode(&newNode);
    
    strcpy(newNode->dir_name,path+i+1);
    //printf("84 ---- %s\n",newNode->dir_name);

    DirectoryTree tempNode;
    //tempNode.dir_name[0] = '\0';
    DirectoryTree* tempP = &tempNode;
    DirectoryTree* t_p = tempP;
    tempP->brother = node->nextLayer;
    while(tempP->brother){
        
        if(strcmp(tempP->brother->dir_name,newNode->dir_name) == 0){
            /*
            free(newNode->fd);
            free(newNode);

            newNode = NULL;
            */
            freeDirNode(&newNode);
            return NULL;
        }
        
        tempP = tempP->brother;
    }
    tempP->brother = newNode;
    node->nextLayer = t_p->brother;
    (node->fd->f_size)++;

    return newNode;
}

void eraseTree(DirectoryTree** root){
    if(*root){
        DirectoryTree* temp = (*root)->nextLayer;
        /*
        free((*root)->fd);
        free(*root);
        *root = NULL;
        */
        freeDirNode(root);
        while(temp){
            DirectoryTree* t = temp;
            eraseTree(&t);
            temp = temp->brother;
        }
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

int eraseNode(DirectoryTree** root, const char* path){
    if(strcmp((*root)->dir_name,"/") == 0 && strcmp(path,"/") == 0){
        eraseTree(root);
        return 1;
    }

    char father[MAX_FILE_NAME_LENGTH] = {'\0'};
    char son[MAX_FILE_NAME_LENGTH] = {'\0'};
    getFatherCurPath(father,son,path);
    //printf("f:%s, %s\n",father,son);
    DirectoryTree* node =  find(*root,father); // find its father
    if(node == NULL) return -1; // failure

    DirectoryTree tempNode;
    DirectoryTree* tempP = &tempNode;
    DirectoryTree* t_p = tempP;
    tempP->brother = node->nextLayer;
    DirectoryTree* head = node->nextLayer;
    while(head && strcmp(head->dir_name,son)){
        tempP = tempP->brother;
        head = head->brother;
    }
    if(head == NULL) return -1;
    tempP->brother = head->brother;
    node->nextLayer = t_p->brother;
    (node->fd->f_size)--;  // directory size decrease
    eraseTree(&head);
    return 1;
}

void PrintTree(DirectoryTree* root){
    if(root){
        DirectoryTree* head = root;
        while(head){
            printf("name: --- %s\n",head->dir_name);
            PrintTree(head->nextLayer);
            printf("-----\n");
            head = head->brother;
        }
    }
}





#ifdef __NAME__MAIN_TEST__
int main(){
    DirectoryTree* root = NULL;
    DirectoryTree* node = NULL;
    node = add(&root,"/");
    node = add(&root,"/a");
    node = add(&root,"/a/b");
    node = add(&root,"/c");
    node = add(&root,"/d");
    node = add(&root,"/c/a");
    node = add(&root,"/d/wyk");
    node = add(&root,"/d/wxr");
    node = add(&root,"/d/xr");
    if((node = add(&root,"/d/xr")) == NULL) printf("error! exits!\n");
    if((node = add(&root,"/d/wyk/like/xr")) == NULL) printf("error! \n");
    PrintTree(root);
    if(eraseNode(&root,"/www")>0){
        printf("success\n");
        PrintTree(root);
    }else{
        printf("error!\n");
        PrintTree(root);
    }
}
#endif