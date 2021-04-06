#include "control.h"
#include "util.h"

#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include <stdio.h>


#define __CONTROL_C_TEST__

char __ROOT_PATH__[MAX_FILE_NAME_LENGTH];
/* init the root path */
void control_init(const char* root_path){
    strcpy(__ROOT_PATH__, root_path);
}


/* judge if a node should write to disk and release it from the pmem */
int isFlush(TOID(struct DirectoryTree) node){
    return 1;
}

void help_flush_to_disk(TOID(struct DirectoryTree)* root, char* cur_path){
    //printf("24----%s\n",cur_path);
    if(TOID_IS_NULL(*root)) return ;  
    if(dirOrFileNode(*root) == 0){ // directory
        if(strcmp(D_RW(*root)->dir_name,"/") == 0){//  is root
            //strcpy(cur_path,__ROOT_PATH__);
            if(mkdir(__ROOT_PATH__, 0777) != EEXIST){
            }
            strcpy(cur_path,"/");
            TOID(struct DirectoryTree) t_p = D_RW(*root)->nextLayer;
            while(!TOID_IS_NULL(t_p)){
                help_flush_to_disk(&t_p,cur_path);
                t_p = D_RW(t_p)->brother;
            }
        }else{
            // consider to cut down the space of temp array: done
            int len = strlen(cur_path);
            strcat(cur_path,D_RW(*root)->dir_name);

            char temp[MAX_FILE_NAME_LENGTH * 5];
            strcpy(temp,__ROOT_PATH__);
            strcat(temp,cur_path);
            //printf("49-----%s\n",temp);
            if(mkdir(temp, 0777) != EEXIST){
            }

            strcat(cur_path,"/");
            TOID(struct DirectoryTree) t_p = D_RW(*root)->nextLayer;
            while(!TOID_IS_NULL(t_p)){
                help_flush_to_disk(&t_p,cur_path);
                t_p = D_RW(t_p)->brother;
            }
            //cur_path[len] = '\0'; // should consider to clear all the char of array
            int len1 = strlen(cur_path);
            for(int i=len;i<len1;++i){
                cur_path[i] = '\0';
            }
        }
    }else{ // just file
        if(isFlush(*root) > 0){
            // flush to disk
            char temp[MAX_FILE_NAME_LENGTH * 5];
            strcpy(temp,__ROOT_PATH__);
            strcat(temp,cur_path);
            strcat(temp,D_RW(*root)->dir_name);
            //printf("73----%s\n",temp);
            // TODO: consider the atomic and the dynamic length of file
            FILE* fp = fopen(temp,"w");
            if(fp != NULL){
                fwrite(D_RW(D_RW((D_RW(*root)->fd))->c_p)->content,sizeof(char),D_RW((D_RW(*root)->fd))->f_size, fp);
                fclose(fp);
                freeFileContent(root);
                D_RW(D_RW(*root)->fd)->isInDisk = 1;
                // should consider the mark of dirty
            }
        }
    }
}
/* according to the algorithm to flush some nodes of the directory tree */
void flush_to_disk(TOID(struct DirectoryTree)* root){
    char __recur_temp__[MAX_FILE_NAME_LENGTH*5];
    help_flush_to_disk(root,__recur_temp__);
}

void help_load_to_pmem(TOID(struct DirectoryTree)* node, const char* path){
    if(D_RW(D_RW(*node)->fd)->isInDisk > 0){
        char temp[MAX_FILE_NAME_LENGTH*2];
        strcpy(temp,__ROOT_PATH__);
        strcat(temp,path);
        //printf("82--------%s\n",temp);
        FILE* fp = fopen(temp,"r");
        if(fp != NULL){
            char buffer[128];
            fread(buffer,sizeof(char),D_RW((D_RW(*node)->fd))->f_size, fp);
            fclose(fp);
            writeToFileContent(node,buffer,D_RW((D_RW(*node)->fd))->f_size);
            D_RW(D_RW(*node)->fd)->isInDisk = 0;
            // should consider the mark of dirty
        }
    }
}

void load_to_pmem(TOID(struct DirectoryTree)* root,const char* path){
    TOID(struct DirectoryTree) node = find(*root,path);
    if(!TOID_IS_NULL(node)){
        help_load_to_pmem(&node,path);
    }
}

#ifdef __CONTROL_C_TEST__
void help_write(TOID(struct DirectoryTree)* root, const char* path, char* buffer){
    TOID(struct DirectoryTree) node = find(*root,path);
	if( TOID_IS_NULL(node)){
		//printf("207-----%s\n",path);
		node = add(root,path,1);
		if(! TOID_IS_NULL(node)){
			//node->fd->mark = 1;
			D_RW(D_RW(node)->fd)->f_size = strlen(buffer);
			strcpy(D_RW(D_RW(D_RW(node)->fd)->c_p)->content,buffer);      
		}else{
		}
	}else{
		D_RW(D_RW(node)->fd)->f_size = strlen(buffer);
		//node->fd->c_p = (Content* )(malloc(sizeof(Content)));
		strcpy(D_RW(D_RW(D_RW(node)->fd)->c_p)->content,buffer);
	}
}

int main(){
    control_init("/home/ubuntu/shuitang/GraduationProject/tmp_fs");
    TOID(struct DirectoryTree) root;
    init(&root,"/home/ubuntu/shuitang/GraduationProject/tmp_fs.pmem");

    int mark = 0;
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
    char tmp[1000];
    char path[50];
    int n;
    scanf("%d",&n);
    while(n--){
        scanf("%s %s",path,tmp);
        help_write(&root,path,tmp);
    }
    flush_to_disk(&root);

}
#endif