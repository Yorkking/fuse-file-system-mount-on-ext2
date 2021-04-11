#include<bits/stdc++.h>
using namespace std;

int main(){
    const char* file_name = "/home/ubuntu/shuitang/GraduationProject/wykfs/tmp_pmem/c/oshrvrpfpmjapdbrwmyx.txt";
    if(0){
        FILE* fp = fopen(file_name,"r");
        if(fp != NULL){
            char buffer[2048];
            int size = 2048;
            size = fread(buffer,sizeof(char),size,fp);
            buffer[size] = '\0';
            printf("%d: %s\n",size, buffer);
        }
    }
    else{
        FILE* fp = fopen(file_name,"w");
        if(fp != NULL){
            char buffer[52] = "wykwywkwywykkehwk";
            int size = fwrite(buffer,sizeof(char),strlen(buffer),fp);
            buffer[size] = '\0';
            printf("%d: %s\n",size, buffer);
        }
    }
    
}