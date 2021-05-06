#include<bits/stdc++.h>
#define FILE_NUM 2000

using namespace std;

string root_name = "/home/ubuntu/shuitang/GraduationProject/wykfs/tmp_pmem/tst_rt/a/";
string root_name2 = "/home/ubuntu/shuitang/GraduationProject/wykfs/tmp_pmem/tst_rt/b/";
string file_names[FILE_NUM];
void* createFile(void * arg){
    //cout<<"hello from" << 10  << endl;
    int start = *((int* )(arg)) * 500;
    //cout<<"-####hello from" << *((int* )(arg)) << endl;
    int end = start + 500;
    for(int i=start;i<end;++i){
        string real_name = root_name + file_names[i];
        //cout<<"1616----"<<i<<" "<<file_names[i]<<endl;
        FILE* fp = fopen(real_name.c_str(),"w");
        int size = 256;
        string t(size-1,'a');
        if(fp != NULL){
            fwrite(t.c_str(),sizeof(char),t.length(),fp);
            //printf("success :%s\n",real_name.c_str());
            fclose(fp);
        }else{
            printf("error form file: %s\n",real_name.c_str());
        }
    }
    printf("done from thread: %u\n",(unsigned int) pthread_self());
}

void* singCreateFile(void * arg){
    for(int i=0;i<FILE_NUM;++i){
        string real_name = root_name2 + file_names[i];
        //cout<<"1616----"<<i<<" "<<file_names[i]<<endl;
        int size = 256;
        string t(size-1,'a');
        FILE* fp = fopen(real_name.c_str(),"w");
        if(fp != NULL){
            fwrite(t.c_str(),sizeof(char),t.length(),fp);
            //printf("success :%s\n",real_name.c_str());
            fclose(fp);
        }else{
            printf("error form file: %s\n",real_name.c_str());
        }
    }
    printf("done from single thread : %u\n",(unsigned int) pthread_self());
}

int main(){

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
    pthread_t threads[4];
    int idx[4];

    timespec start, end;

    clock_gettime(CLOCK_REALTIME, &start);
    for(int i=0;i<4;i++){
        //cout<<"&*&*"<<i<<endl;
        idx[i] = i;
        int rc = pthread_create(&threads[i],NULL,&createFile,&idx[i]);
        if(rc){
            printf("Error:unable to create thread, %d\n", rc);
            //exit(-1);
        }
    }
    for(int i=0;i<4;i++){
        pthread_join(threads[i],NULL);
    }
    clock_gettime(CLOCK_REALTIME, &end);

    double cost_time_4 = (double)(end.tv_sec - start.tv_sec)  + (end.tv_nsec - start.tv_nsec)/1e9;
    printf("time for 4 threads: %lf s\n",cost_time_4);

    pthread_t singleWriter;
    if(clock_gettime(CLOCK_REALTIME, &start) == -1){
        printf("97:  error\n");
    }
    if(pthread_create(&singleWriter, NULL, &singCreateFile, NULL)){
        printf("error\n");
    }
    pthread_join(singleWriter,NULL);

    if( clock_gettime(CLOCK_REALTIME, &end) == -1){
        printf("105: error : \n");
    }
    double cost_time_1 = (double)(end.tv_sec - start.tv_sec)  + (end.tv_nsec - start.tv_nsec)/1e9;


    printf("time for 1 thread: %lf s\n",cost_time_1);

    return 0;    
}
