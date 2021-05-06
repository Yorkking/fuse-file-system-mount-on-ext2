#include<bits/stdc++.h>
#define FILE_NUM 2048

using namespace std;

char write_buffer[1024*1024];
//string root_name = "../tmp_pmem/tst_rt/a/";
//string root_name2 = "../tmp_pmem/tst_rt/b/";
int w_size = 1024 * 2;
int thread_nums = 0;

string root_name = "../concurrency_test/tst_rt/a/";
string root_name2 = "../concurrency_test/tst_rt/b/";

string file_names[FILE_NUM];
void* createFile(void * arg){
    //cout<<"hello from" << 10  << endl;
    int start = *((int* )(arg)) * (FILE_NUM / thread_nums) ;
    //cout<<"-####hello from" << *((int* )(arg)) << endl;
    int end = min(start + (FILE_NUM / thread_nums), FILE_NUM);
    for(int i=start;i<end;++i){
        string real_name = root_name + file_names[i];
        //cout<<"1616----"<<i<<" "<<file_names[i]<<endl;
        FILE* fp = fopen(real_name.c_str(),"w");
        int size = w_size;
        //string t(size-1,'a');
        if(fp != NULL){
            fwrite(write_buffer,sizeof(char),size,fp);
            //printf("success :%s\n",real_name.c_str());
            fclose(fp);
        }else{
            printf("error form file: %s\n",real_name.c_str());
        }
    }
    printf("done from thread: %u\n",(unsigned int) pthread_self());

    return NULL;
}

void* singCreateFile(void * arg){
    for(int i=0;i<FILE_NUM;++i){
        string real_name = root_name2 + file_names[i];
        //cout<<"1616----"<<i<<" "<<file_names[i]<<endl;
        int size = w_size;
        //string t(size-1,'a');
        FILE* fp = fopen(real_name.c_str(),"w");
        if(fp != NULL){
            fwrite(write_buffer,sizeof(char),size,fp);
            //printf("success :%s\n",real_name.c_str());
            fclose(fp);
        }else{
            printf("error form file: %s\n",real_name.c_str());
        }
    }
    printf("done from single thread : %u\n",(unsigned int) pthread_self());
    return NULL;
}

int main(int argc, char *argv[]){
    if(argc < 3){
        printf("argc is not enough!\n");
        return -1;
    }

    //int w_size = 0;
    
    sscanf(argv[1],"%d", &w_size);
    sscanf(argv[2],"%d", &thread_nums);
    printf("w_size: %d nums: %d\n",w_size, thread_nums);
    w_size = w_size * 1024;
    for(int i=0;i<1024*1024;++i){
        write_buffer[i] = 'a';
    }

    int cnt = 0;
    // gererate file_name;
    while(cnt < FILE_NUM){
        for(int i=0;i<26;++i){
            for(int j=0;j<26;++j){
                for(int k=0;k<26;++k){
                    string name;
                    name.push_back('a' + i );
                    name.push_back('a' + j );
                    name.push_back('a' + k );
                    file_names[cnt] = name;

                    cnt++;
                    if(cnt >= FILE_NUM) goto there;
                }
            }
        }
    }
    there: 
    //cout<<"4949"<<endl;
    
    pthread_t* threads = new pthread_t[thread_nums];
    vector<int> idx(thread_nums,0);
    
    timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    for(int i=0;i<thread_nums;i++){
        //cout<<"&*&*"<<i<<endl;
        idx[i] = i;
        int rc = pthread_create(&threads[i],NULL,&createFile,&idx[i]);
        if(rc){
            printf("Error:unable to create thread, %d\n", rc);
            //exit(-1);
        }
    }
    for(int i=0;i<thread_nums;i++){
        pthread_join(threads[i],NULL);
    }
    clock_gettime(CLOCK_REALTIME, &end);

    double cost_time_4 = (double)(end.tv_sec - start.tv_sec)  + (end.tv_nsec - start.tv_nsec)/1e9;
    printf("time for %d thread(s): %lf s\n",thread_nums, cost_time_4);
    
    pthread_t singleWriter;
    clock_gettime(CLOCK_REALTIME, &start);
    if(pthread_create(&singleWriter, NULL, &singCreateFile, NULL)){
        printf("error\n");
    }
    pthread_join(singleWriter,NULL);
    clock_gettime(CLOCK_REALTIME, &end);
    double cost_time_1 = (double)(end.tv_sec - start.tv_sec)  + (end.tv_nsec - start.tv_nsec)/1e9;


    printf("time for 1 thread: %lf s\n",cost_time_1);
    
    return 0;    
}
