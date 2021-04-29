#include<bits/stdc++.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>
using namespace std;
char buffer[1024 * 1024];
int main(){
    string root = "../disk_file_test/", save_file = "disk_data/";
    //string root = "/mnt/pmem/wyk/wykfs/workout2/workout/tmp_pmem/", string save_file = "myfs/";
    vector<string> files;
    int cnt = 0;
    string tmp;
    while(cin >> tmp && cnt < 1024){
        cout<<tmp<<endl;
        files.push_back(tmp);
        cnt++;
    }
    cout<<15<<endl;
    timespec start, end;
    vector<int> size_list = {1024,512,256,128,64,32,16,1};
    for(auto& size: size_list){
        int read_size = size * 1024;
    
        double cost_time = 0.0;
        vector<double> res;

        int idx = 0;
        
        for(auto& file_name : files ){
            //cout<<" 22 " <<endl;
            
            string read_file_name = root + file_name;
            
            FILE* fp = fopen(read_file_name.c_str(),"r");
            clock_gettime(CLOCK_MONOTONIC, &start);
            if(fp != NULL){
                //cout<<"s"<<endl;
                int r_size = 0;
                while(!feof(fp) && r_size != read_size){
                    r_size += fread(buffer,sizeof(char),read_size, fp);
                    //cout<<"r_size: "<<r_size<<endl;
                }
                fflush(fp);
                int fd  = fileno(fp);
                fsync(fd);
                fclose(fp);
            }
            
            clock_gettime(CLOCK_MONOTONIC, &end);
            cost_time = ((end.tv_sec - start.tv_sec) * 10^9 + end.tv_nsec - start.tv_nsec)/1e9;
            if(cost_time < 0) cout<<read_file_name<<endl;

            cout<<"time: "<<cost_time<<endl;
            res.push_back(cost_time);
            idx++;
            if(idx % 50 == 0){
                cout<<"s: "<<idx<<endl;
            }
        }

        
        string tmp_save_file = save_file + to_string(size) + string("MB.txt");
        FILE* fp = fopen(tmp_save_file.c_str(),"w");
        char tmp_buffers[1000];
        if(fp != NULL){
            for(auto& value: res){
                sprintf(tmp_buffers,"%lf\n",value);
                fwrite(tmp_buffers,sizeof(char),strlen(tmp_buffers),fp);
            }
            fclose(fp);
        }

    }

    //int size = 1024;

    return 0;
}