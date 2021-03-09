
#define FUSE_USE_VERSION 30

#include<fuse.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>

#define BUF_SIZE 512
#define RAM_SIZE 4096
#define NAME_SIZE 64
#define FILE_NUM 64
#define FILE_SIZE 4096/64

char ram_content[FILE_NUM][FILE_SIZE];
char name_list[FILE_NUM][NAME_SIZE];

int cur_file_idx = 0;
int cur_file_content_idx = 0; 


typedef struct RamFd{
    char* p;
    char* fileName;
}RamFd;

/*
存储于RAM的文件结构：

*/
typedef struct RamFile{
    RamFd* startFp;
    size_t size;
}RamFile;



static int do_getattr( const char *path, struct stat *st )
{
	st->st_uid = getuid(); // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	st->st_atime = time( NULL ); // The last "a"ccess of the file/directory is right now
	st->st_mtime = time( NULL ); // The last "m"odification of the file/directory is right now
	
	if ( path[0] == '/' )
	{
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
	}
	else
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}
	return 0;
}


// 根据文件名查找一个文件是否在 ram_file_list中
int find_file_idx(const char* path){
    for(int cur_idx = 0;cur_idx < cur_file_idx; cur_idx++){
        if( strcmp(path, name_list[cur_idx]) == 0){
            return cur_idx;
        }
    }
    return -1;
}
// 用于增加一个文件，当open('w')操作时
void add_file(const char* filename){
    strcpy(name_list[cur_file_idx], filename);
    cur_file_idx++;

    strcpy(ram_content[cur_file_content_idx], "");
    cur_file_content_idx++;
}

void write_to_ram_file(const char* filename,const char* content){
    int file_idx = find_file_idx(filename);
    if(file_idx == -1) return;

    strcpy(ram_content[file_idx], content);
}
void write_to_disk(const char* filename, const char* content){

}

static int do_write( const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info ){
	printf("write\n");
    write_to_ram_file( path, buffer );
	
	return size;
}



// 读单个文件信息
static int do_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi ){
    printf("read\n");
    int file_idx = find_file_idx(path);

    if(file_idx == -1){
        return -1;
    }
    char *content = ram_content[file_idx];

    memcpy(buffer, content + offset, size);

    return strlen(content) - offset;
}


// 读当前目录下所有的文件信息
static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi ){
    filler(buffer,".",NULL,0);
    filler(buffer,"..",NULL,0);
    for ( int curr_idx = 0; curr_idx <= cur_file_idx; curr_idx++ )
			filler( buffer, ram_content[ curr_idx ], NULL, 0 );
	
    return 0;
}

static int do_mknod( const char *path, mode_t mode, dev_t rdev )
{
	path++;
	add_file( path );
	
	return 0;
}

static struct  fuse_operations wykfs_operations = {
    .write = do_write,
    .readdir = do_readdir,
    .read = do_read,
    .mknod = do_mknod,
    .getattr = do_getattr,
};

int main(int argc, char* argv[]){
    
    fuse_main(argc, argv, &wykfs_operations, NULL);
    return 0;
}

