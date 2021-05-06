#coding:utf-8

import os
root = '../disk_file_test/'

with open('./all_file.txt','r',encoding='utf-8') as f:
    lines = f.readlines()

import time

file_size_list = [1024,512,256,128,64,32,16,8,4,1]
for size in file_size_list:
    f_size = size * 1024
    cnt = 0
    save_file_name = './disk_data/read_' +  str(size) + 'MB.txt'
    save_w_time_list = []

    for line in lines[0:1024]:
        file_name = root + line[:-1]
        time_start = time.time()
        with open(file_name,'r') as f:
            content = f.read(f_size)
            f.flush()
            os.fsync(f.fileno())
            f.close()
        time_cur = time.time()
        save_w_time_list.append(time_cur-time_start)
        if cnt % 50 == 0:
            print(cnt,"s: " + file_name, len(content)/1024)
       
            
        cnt += 1

    with open(save_file_name,'w',encoding='utf-8') as save_f:
        for i in save_w_time_list:
            save_f.write(str(i) + '\n')
