#include<bits/stdc++.h>
using namespace std;


char buffer[1024 * 1024];

void createStr(char* buffer, int f_size){
    for(int i=0;i<f_size;++i){
        buffer[i] = (i%26) + 'a';
    }
}

int main(){
    string root = "../disk_file_test/", save_file = "disk_data/write";
    vector<string> files;
    int cnt = 0;
    string tmp;
    while(cin >> tmp && cnt < 1024){
        cout<<tmp<<endl;
        files.push_back(tmp);
        cnt++;
    }
    timespec start, end;
    vector<int> size_list = {1,16,32,64,128,256,512,1024};

    for(auto& size: size_list){
        int read_size = size * 1024;
        createStr(buffer,read_size);
        double cost_time = 0.0;
        vector<double> res;

        int idx = 0;
        
        for(auto& file_name : files ){
            //cout<<" 22 " <<endl;
            
            string read_file_name = root + file_name;
            
            FILE* fp = fopen(read_file_name.c_str(),"w");
            clock_gettime(CLOCK_MONOTONIC, &start);
            if(fp != NULL){
                //cout<<"s"<<endl;
                int r_size = 0;
                fwrite(buffer,sizeof(char),size*1024,fp);
                //fflush(fp);
                //int fd  = fileno(fp);
                //fsync(fd);
                fclose(fp);
            }
            
            clock_gettime(CLOCK_MONOTONIC, &end);
            cost_time = (double)(end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1e9;
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
                sprintf(tmp_buffers,"%g\n",value);
                fwrite(tmp_buffers,sizeof(char),strlen(tmp_buffers),fp);
            }
            fclose(fp);
        }

    }


    return 0;
}