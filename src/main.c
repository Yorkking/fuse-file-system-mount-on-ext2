 
#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "control.h"

TOID(struct DirectoryTree) root;

int isDirOrFile(const char *path){
    TOID(struct DirectoryTree) node = find(root,path);
	if(TOID_IS_NULL(node)) return -1; // don't exsit
	return TOID_IS_NULL(D_RW(node)->fd) ? 0 : 1 ; // 1 for file, 0 for directory
}

int isLeagalPath(const char* path){ // TODO
	return 1;
}
static int do_getattr( const char *path, struct stat *st ){
    //printf("do_getattr %s\n",path);
    st->st_uid = getuid(); // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	st->st_atime = time( NULL ); // The last "a"ccess of the file/directory is right now
	st->st_mtime = time( NULL ); // The last "m"odification of the file/directory is right now
	TOID(struct DirectoryTree) node = find(root,path);
	if(TOID_IS_NULL(node)) return -ENOENT; // don't exsit
	int flag  = TOID_IS_NULL(D_RW(node)->fd) ? 0 : 1 ; // 1 for file, 0 for directory

	if ( flag == 0 ) // directory
	{
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
	}
	else if ( flag == 1 ) // file
	{
		st->st_mode = S_IFREG | 0777;
		st->st_nlink = 1;
		st->st_size = D_RW(D_RW(node)->fd)->real_size;
	}else{
		return -ENOENT;
	};
	return 0;
}

static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi ){	
	
	//printf("do_readdir %s\n",path);
	TOID(struct DirectoryTree) node =  find(root,path);
	if( ! TOID_IS_NULL(node) && TOID_IS_NULL(D_RW(node)->fd)){
		filler( buffer, ".", NULL, 0 ); // Current Directory
		filler( buffer, "..", NULL, 0 ); // Parent Directory
		TOID(struct DirectoryTree) head = D_RW(node)->nextLayer;
		while(!TOID_IS_NULL(head)){
			filler(buffer,D_RW(head)->dir_name,NULL,0);
			head = D_RW(head)->brother;
		}
		return 0;
	}else{
		return -ENOENT;
	}
}

static int do_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi ){
    //printf("do_read %s\n",path);
	TOID(struct DirectoryTree) node = find(root,path);
	if(!TOID_IS_NULL(node)){
		return read_from_pmem_disk(node,path,buffer,size,offset);
	}
	return -ENOENT;
	
}

static int do_mkdir( const char *path, mode_t mode ){
    //printf("do_mkdir %s\n",path);
	// TODO: judge if path is legal
	TOID(struct DirectoryTree) node =  add(&root,path,0);
	if(!TOID_IS_NULL(node)){
		return 0;
	}else{
		return -ENOENT;
	}
}

static int do_mknod( const char *path, mode_t mode, dev_t rdev ){
	//printf("do_mknod %s\n",path);
	// TODO: judge if path is legal
	TOID(struct DirectoryTree) node = add(&root,path,1);
	if(! TOID_IS_NULL(node)){
		//node->fd->mark = 1;
		D_RW(D_RW(node)->fd)->f_size = 0;
	}else{
		return -ENONET;
	}
	return 0;
}

static int do_write( const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info ){
	// write_to_file( path, buffer );
	// @alpha 1.1: over write
	TOID(struct DirectoryTree) node = find(root,path);
	if( TOID_IS_NULL(node)){
		//printf("207-----%s\n",path);
		node = add(&root,path,1);
		if(! TOID_IS_NULL(node)){
			return write_to_pmem_disk(&node,path,buffer,size,offset);
		}else{
			return -ENONET;
		}
	}else{
		return write_to_pmem_disk(&node,path,buffer,size,offset);
	}
}

static int do_unlink(const char* path){
	int flag = erase_file(&root,path);
	if(flag < 0) -ENONET;
	return 0;
}
static int do_rmdir(const char* path){
	int flag = erase_dir(&root,path);
	if(flag < 0) -ENONET;
	return 0;
}
static int do_truncate(const char *path, off_t length){
	TOID(struct DirectoryTree) node = find(root,path);
	printf("---main 134: %ld, %s\n",length, path);
	if(D_RW(D_RW(node)->fd)->real_size > length){
		return fileSizeSet(&node,length);
	}
	return 0;
}
static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    .read	= do_read,
    .mkdir	= do_mkdir,
    .mknod	= do_mknod,
    .write	= do_write,
	.unlink =  do_unlink,
	.rmdir = do_rmdir,
	.truncate = do_truncate,
};

void* schedule(){
	int seconds = 25;
	while(1){
		sleep(seconds);
		// should consider mutex
		printf("----start scheudule\n");
		flush_load(&root);
		printf("----end scheudule\n");
	}
}

int main( int argc, char *argv[] ){
	const char* pool_file_name =  "/home/ubuntu/shuitang/GraduationProject/wykfs/wykfs.pmem";
	const char* root_path = "/home/ubuntu/shuitang/GraduationProject/wykfs/fs_tmp";
    control_init(&root,pool_file_name, root_path);
	pthread_t thread;
	int rc = pthread_create(&thread,NULL,schedule,NULL);
	if(rc){
		printf("Error:unable to create thread, %d\n", rc);
		exit(-1);
	}
	fuse_main( argc, argv, &operations, NULL ); 
	return 0;
}