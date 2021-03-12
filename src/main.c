 
#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "util.h"

DirectoryTree* root;
#define FILE_NUM 100
Content file_nodes[FILE_NUM];

int isDirOrFile(const char *path){
    DirectoryTree* node = find(root,path);
	if(node == NULL) return -1; // don't exsit
	return node->fd->mark; // 1 for file, 0 for directory
}

int isLeagalPath(const char* path){
	return 1;
}
static int do_getattr( const char *path, struct stat *st )
{
    //printf("do_getattr %s\n",path);
    st->st_uid = getuid(); // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	st->st_atime = time( NULL ); // The last "a"ccess of the file/directory is right now
	st->st_mtime = time( NULL ); // The last "m"odification of the file/directory is right now
	int flag = isDirOrFile(path);
	if ( flag == 0 ) // directory
	{
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
	}
	else if ( flag == 1 ) // file
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}
	else
	{
		return -ENOENT;
	};
	return 0;
}

static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi ){	
	
	//printf("do_readdir %s\n",path);
	DirectoryTree*node =  find(root,path);
	if(node != NULL && node->fd->mark == 0){
		filler( buffer, ".", NULL, 0 ); // Current Directory
		filler( buffer, "..", NULL, 0 ); // Parent Directory
		DirectoryTree* head = node->nextLayer;
		while(head){
			filler(buffer,head->dir_name,NULL,0);
			head = head->brother;
		}
		return 0;
	}else{
		return -ENOENT;
	}
}

static int do_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
    //printf("do_read %s\n",path);
	DirectoryTree* node = find(root,path);
	if(node){
		memcpy(buffer,node->fd->c_p->content + offset,size);
		return node->fd->f_size - offset;
	}
	return -ENOENT;
	
}

static int do_mkdir( const char *path, mode_t mode )
{
    //printf("do_mkdir %s\n",path);
	// TODO: judge if path is legal
	DirectoryTree* node =  add(&root,path);
	if(node != NULL){
		node->fd->mark = 0;
		node->fd->f_size = 0;
		node->fd->c_p = NULL;
		return 0;
	}else{
		return -ENOENT;
	}
}

static int do_mknod( const char *path, mode_t mode, dev_t rdev )
{
	//printf("do_mknod %s\n",path);

	// TODO: judge if path is legal
	DirectoryTree* node = add(&root,path);
	if(node != NULL){
		node->fd->mark = 1;
		node->fd->f_size = 0;
		node->fd->c_p = NULL;
	}else{
		return -ENONET;
	}
	return 0;
}


static int do_write( const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info )
{
    //printf("do_write %s\n",path);
	//printf("%ld\n",info->fh);

	// write_to_file( path, buffer );
	// @alpha 1.1: over write
	DirectoryTree* node = find(root,path);
	if(node == NULL){
		//printf("207-----%s\n",path);
		node = add(&root,path);
		if(node){
			node->fd->mark = 1;
			node->fd->f_size = size;
			node->fd->c_p = (Content* )(malloc(sizeof(Content)));
			//printf("212-----%s\n",node->dir_name);
			strcpy(node->fd->c_p->content,buffer);
			//printf("214-----%s\n",node->fd->c_p->content);
		}else{

			return -ENONET;
		}
	}else{
		//printf("221-----%s\n",path);
		node->fd->f_size = size;
		node->fd->c_p = (Content* )(malloc(sizeof(Content)));
		strcpy(node->fd->c_p->content,buffer);
	}
	return size;
}

static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    .read	= do_read,
    .mkdir	= do_mkdir,
    .mknod	= do_mknod,
    .write	= do_write,
};

int main( int argc, char *argv[] )
{
	root = NULL;
	DirectoryTree* node = add(&root,"/");
	node->fd->mark = 0;
	node->fd->c_p = NULL;
	node->fd->f_size = 0;
	//PrintTree(root);
	fuse_main( argc, argv, &operations, NULL );
	return 0;
}