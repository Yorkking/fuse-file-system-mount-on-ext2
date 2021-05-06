#include<bits/stdc++.h>
#define FILE_NUM 2000

using namespace std;

string root_name = "../tmp_pmem/tst_rt/";

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
                   
                    
                    if(cnt % 50 == 0){
                        cout<<"s48: "<<file_names[cnt]<<endl;
                        cout<<"s: "<<name<<endl;
                    }
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
    for(int i=0;i<4;i++){
        //cout<<"&*&*"<<i<<endl;
        idx[i] = i;
        int rc = pthread_create(&threads[i],NULL,&createFile,&idx[i]);
        if(rc){
            printf("Error:unable to create thread, %d\n", rc);
            //exit(-1);
        }
    }
    while(1){
        
    }
    

}
