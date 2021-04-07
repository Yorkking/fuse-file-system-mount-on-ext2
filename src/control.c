#include "control.h"
#include "util.h"

#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>


//#define __CONTROL_C_TEST__

char __ROOT_PATH__[MAX_FILE_NAME_LENGTH];
/* init the root path */
void control_init(TOID(struct DirectoryTree)* root, const char* pool_file_name,const char* root_path){
    init(root,pool_file_name);
    strcpy(__ROOT_PATH__, root_path);
}

void help_write_to_disk(TOID(struct DirectoryTree) node, const char* path){
    FILE* fp = fopen(path,"w");
    if(fp != NULL){
        int f_size = D_RW(D_RW(node)->fd)->f_size;
        int cur = 0;
        char buffer[CONTENT_LENGTH];
        int size;
        do{
            size = readContent(&node,buffer,CONTENT_LENGTH,cur);
            if(size > 0){
                fwrite(buffer,sizeof(char),size,fp);
            }
            cur += size;
        }while(size == CONTENT_LENGTH);
        fclose(fp);
    }
}

void help_load_to_pmem(TOID(struct DirectoryTree)* node, const char* path){
    
    FILE* fp = fopen(path,"r");
    if(fp != NULL){
        char buffer[CONTENT_LENGTH];
        int cur = 0;
        while(!feof(fp)){
            int size = fread(buffer,sizeof(char),CONTENT_LENGTH, fp);
            writeContent(node,buffer,size,cur);
            cur += size;
        }
        fclose(fp);
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
            help_write_to_disk(*root,temp);
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
        int r_size = readContent(&node,buffer,size,offset);
		D_RW(D_RW(node)->fd)->alg_cnt += 1;
		return r_size;
    }else{
        char temp[MAX_FILE_NAME_LENGTH * 5];
        strcpy(temp,__ROOT_PATH__);
        strcat(temp,path);
        FILE* fp = fopen(temp,"r");
        if(fp != NULL){
            fseek(fp,offset,SEEK_SET);
            int r_size = fread(buffer,sizeof(char),size, fp);
            fclose(fp);
            D_RW(D_RW(node)->fd)->alg_cnt += 1;
		    return r_size;
        }
    }
}
int write_to_pmem_disk(TOID(struct DirectoryTree)* node, const char* path, const char* buffer, size_t size, off_t offset){
    if(D_RW(D_RW(*node)->fd)->isInDisk == 0){
        D_RW(D_RW(*node)->fd)->alg_cnt += 2;
        D_RW(D_RW(*node)->fd)->dirty = 1;
        return writeContent(node,buffer,size,offset);
    }else{
        char temp[MAX_FILE_NAME_LENGTH * 5];
        strcpy(temp,__ROOT_PATH__);
        strcat(temp,path);
        FILE* fp = fopen(path,"w");
        if(fp != NULL){
            fseek(fp,offset,SEEK_SET);
            int w_size = fwrite(buffer,sizeof(char),size, fp);
            D_RW(D_RW(*node)->fd)->alg_cnt += 2;
            fclose(fp);
            return w_size;
        }
        return 0;
    }
}
//judge whether it is a directory
int is_dir(const char *path){
    struct stat statbuf;
    if(lstat(path, &statbuf) ==0){
        //lstat returns file information, which is stored in the stat structure
        return S_ISDIR(statbuf.st_mode) != 0;//S_ISDIR macro, to judge whether the file type is a directory
    }
    return 0;
}

//judge whether it is a regular file
int is_file(const char *path){
    struct stat statbuf;
    if(lstat(path, &statbuf) ==0)
        return S_ISREG(statbuf.st_mode) != 0;//judge whether the file is a regular file
    return 0;
}

//judge whether it is a special directory
int is_special_dir(const char *path){
    return strcmp(path, ".") == 0 || strcmp(path, "..") == 0;
}

//generate complete file path
void get_file_path(const char *path, const char *file_name,  char *file_path){
    strcpy(file_path, path);
    if(file_path[strlen(path) - 1] != '/')
        strcat(file_path, "/");
    strcat(file_path, file_name);
}

void delete_file(const char *path){
    DIR* dir;
    struct dirent* dir_info;

    char file_path[MAX_FILE_NAME_LENGTH * 5];
    if(is_file(path)){
        remove(path);
        return;
    }
    if(is_dir(path)){
        if((dir = opendir(path)) == NULL)
            return;
        while((dir_info = readdir(dir)) != NULL){
            get_file_path(path, dir_info->d_name, file_path);
            if(is_special_dir(dir_info->d_name)) continue;
            delete_file(file_path);
            rmdir(file_path);
        }
    }
}


int erase_dir(TOID(struct DirectoryTree)* root, const char* path){
    int flag = eraseNode(root,path);
    if(flag >= 0){
        char temp[MAX_FILE_NAME_LENGTH * 5];
        strcpy(temp,__ROOT_PATH__);
        strcat(temp,path);
        // TODO: recursive delete, but I want to implement it in eraseNode
        delete_file(temp);
        rmdir(temp);
        return 1;
    }
    return -ENONET;
}
int erase_file(TOID(struct DirectoryTree)* root, const char* path){
    int flag = eraseNode(root,path);
    if(flag >= 0){
        char temp[MAX_FILE_NAME_LENGTH * 5];
        strcpy(temp,__ROOT_PATH__);
        strcat(temp,path);
        return remove(temp);
    }
    return -ENONET;
}

#ifdef __CONTROL_C_TEST__
void help_write(TOID(struct DirectoryTree)* root, const char* path, char* buffer){
    TOID(struct DirectoryTree) node = find(*root,path);
	if( TOID_IS_NULL(node)){
		//printf("207-----%s\n",path);
		node = add(root,path,1);
		if(! TOID_IS_NULL(node)){
			//node->fd->mark = 1;
			writeContent(&node,buffer,strlen(buffer),0); 
		}else{
		}
	}else{
        writeContent(&node,buffer,strlen(buffer) ,0);
	}
}

int main(){
    
    TOID(struct DirectoryTree) root;
    control_init(&root,"/home/ubuntu/shuitang/GraduationProject/tmp_fs.pmem", "/home/ubuntu/shuitang/GraduationProject/control_fs");
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
    char tmp[2000];
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