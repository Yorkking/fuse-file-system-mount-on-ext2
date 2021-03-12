 
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



char dir_list[ 256 ][ 256 ];
int curr_dir_idx = -1;

char files_list[ 256 ][ 256 ];
int curr_file_idx = -1;

char files_content[ 256 ][ 256 ];
int curr_file_content_idx = -1;


void add_dir( const char *dir_name )
{
	curr_dir_idx++;
	strcpy( dir_list[ curr_dir_idx ], dir_name );
}

int is_dir( const char *path )
{
	path++; // Eliminating "/" in the path
	
	for ( int curr_idx = 0; curr_idx <= curr_dir_idx; curr_idx++ )
		if ( strcmp( path, dir_list[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}

void add_file( const char *filename )
{
	curr_file_idx++;
	strcpy( files_list[ curr_file_idx ], filename );
	
	curr_file_content_idx++;
	strcpy( files_content[ curr_file_content_idx ], "" );
}

int is_file( const char *path )
{
	path++; // Eliminating "/" in the path
	
	for ( int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++ )
		if ( strcmp( path, files_list[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}

int get_file_index( const char *path )
{
	path++; // Eliminating "/" in the path
	
	for ( int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++ )
		if ( strcmp( path, files_list[ curr_idx ] ) == 0 )
			return curr_idx;
	
	return -1;
}

void write_to_file( const char *path, const char *new_content )
{
	int file_idx = get_file_index( path );
	
	if ( file_idx == -1 ) // No such file
		return;
	strcpy( files_content[ file_idx ], new_content ); 
}

// ... //

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
    printf("do_read %s\n",path);
	int file_idx = get_file_index( path );
	
	if ( file_idx == -1 )
		return -1;
	
	char *content = files_content[ file_idx ];
	
	memcpy( buffer, content + offset, size );
		
	return strlen( content ) - offset;
}

static int do_mkdir( const char *path, mode_t mode )
{
    printf("do_mkdir %s\n",path);
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
	printf("do_mknod %s\n",path);

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
    printf("do_write %s\n",path);
	printf("%ld\n",info->fh);
	write_to_file( path, buffer );
	
	return size;
}

static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    //.read	= do_read,
    .mkdir	= do_mkdir,
    .mknod	= do_mknod,
    //.write	= do_write,
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