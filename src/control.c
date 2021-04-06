#include "control.h"
#include "util.h"

#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include <stdio.h>


//#define __CONTROL_C_TEST__

char __ROOT_PATH__[MAX_FILE_NAME_LENGTH];
/* init the root path */
void control_init(TOID(struct DirectoryTree)* root, const char* pool_file_name,const char* root_path){
    init(root,pool_file_name);
    strcpy(__ROOT_PATH__, root_path);
}

void help_write_to_pmem(TOID(struct DirectoryTree) node, const char* path){
    FILE* fp = fopen(path,"w");
    if(fp != NULL){
        fwrite(D_RW(D_RW((D_RW(node)->fd))->c_p)->content,sizeof(char),D_RW((D_RW(node)->fd))->f_size, fp);
        fclose(fp);
    }
}

void help_load_to_pmem(TOID(struct DirectoryTree)* node, const char* path){
    
    FILE* fp = fopen(path,"r");
    if(fp != NULL){
        char buffer[128];
        fread(buffer,sizeof(char),D_RW((D_RW(*node)->fd))->f_size, fp);
        fclose(fp);
        writeToFileContent(node,buffer,D_RW((D_RW(*node)->fd))->f_size);
        D_RW(D_RW(*node)->fd)->isInDisk = 0;
    }
}

/* 
    judge if a node should write to disk, release it from the pmem and load to pmem from disk 
    1 just for release, 2 for load, 3 just for write not release, 4 for realse and write, 0 for nothing
*/
int isFlushLoad(TOID(struct DirectoryTree) node){
    if(TOID_IS_NULL(node)) return -1; // error
    int threshold = 4;
    if(D_RW(D_RW(node)->fd)->alg_cnt > threshold){ 
        if(D_RW(D_RW(node)->fd)->isInDisk == 0){
            if(D_RW(D_RW(node)->fd)->dirty == 1) return 3;
            return 0;
        }else{
            return 2;
        }
        
    }else if(D_RW(D_RW(node)->fd)->isInDisk == 0){
        if(D_RW(D_RW(node)->fd)->dirty == 1){ // dirty 
            return 4;
        }else return 1;
    }else{
        return 0;
    }
}
void help_flush_load(TOID(struct DirectoryTree)* root, char* cur_path){
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
                help_flush_load(&t_p,cur_path);
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
                help_flush_load(&t_p,cur_path);
                t_p = D_RW(t_p)->brother;
            }
            //cur_path[len] = '\0'; // should consider to clear all the char of array
            int len1 = strlen(cur_path);
            for(int i=len;i<len1;++i){
                cur_path[i] = '\0';
            }
        }
    }else{ // just file
        int flag = isFlushLoad(*root);
        if(flag == 3 || flag == 4){ //write
            // flush to disk
            char temp[MAX_FILE_NAME_LENGTH * 5];
            strcpy(temp,__ROOT_PATH__);
            strcat(temp,cur_path);
            strcat(temp,D_RW(*root)->dir_name);
            //printf("73----%s\n",temp);
            // TODO: consider the atomic and the dynamic length of file
            help_write_to_pmem(*root,temp);
        }
        if(flag == 2 ){ // load
            // TODO: consider the atomic and the dynamic length of file
            char temp[MAX_FILE_NAME_LENGTH * 5];
            strcpy(temp,__ROOT_PATH__);
            strcat(temp,cur_path);
            strcat(temp,D_RW(*root)->dir_name);

            help_load_to_pmem(root,temp);
        }
        if(flag == 1 || flag == 4){ // release
            freeFileContent(root);
        }
        resetAlg(root); // set to 0 after flush or load
    }
}
/* according to the algorithm to flush and load some nodes of the directory tree */
void flush_load(TOID(struct DirectoryTree)* root){
    char __recur_temp__[MAX_FILE_NAME_LENGTH*5];
    help_flush_load(root,__recur_temp__);
}

int read_from_pmem_disk(TOID(struct DirectoryTree) node, const char* path, char* buffer, size_t size, off_t offset){
    if(D_RW(D_RW(node)->fd)->isInDisk == 0){
        memcpy(buffer,D_RW(D_RW(D_RW(node)->fd)->c_p)->content + offset,size);
		D_RW(D_RW(node)->fd)->alg_cnt += 1;
		return D_RW(D_RW(node)->fd)->f_size - offset;
    }else{
        char temp[MAX_FILE_NAME_LENGTH * 5];
        strcpy(temp,__ROOT_PATH__);
        strcat(temp,path);
        FILE* fp = fopen(temp,"r");
        if(fp != NULL){
            fread(buffer,sizeof(char),D_RW((D_RW(node)->fd))->f_size, fp);
            fclose(fp);
            D_RW(D_RW(node)->fd)->alg_cnt += 1;
		    return D_RW(D_RW(node)->fd)->f_size - offset;
        }
    }
}
int write_to_pmem_disk(TOID(struct DirectoryTree)* node, const char* path, const char* buffer, size_t size, off_t offset){
    if(D_RW(D_RW(*node)->fd)->isInDisk == 0){
        writeToFileContent(node,buffer,size);
    }else{
        char temp[MAX_FILE_NAME_LENGTH * 5];
        strcpy(temp,__ROOT_PATH__);
        strcat(temp,path);
        FILE* fp = fopen(path,"w");
        if(fp != NULL){
            fwrite(buffer,sizeof(char),size, fp);
            fclose(fp);
            D_RW(D_RW(*node)->fd)->f_size = size;
        }
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
			writeToFileContent(&node,buffer,strlen(buffer)); 
		}else{
		}
	}else{
        writeToFileContent(&node,buffer,strlen(buffer));
	}
}

int main(){
    
    TOID(struct DirectoryTree) root;
    control_init(&root,"/home/ubuntu/shuitang/GraduationProject/tmp_fs.pmem", "/home/ubuntu/shuitang/GraduationProject/tmp_fs");
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
    int n,m;
    scanf("%d %d",&n,&m);
    while(n--){
        scanf("%s %s",path,tmp);
        help_write(&root,path,tmp);
    }
    if(m == 1)
    flush_load(&root);

    printTree(root);

}
#endif