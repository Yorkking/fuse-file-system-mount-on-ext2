#coding:utf-8
root = '../' + 'tmp_pmem/'


def random_file_content(file_size):
    import random
    basis = 'abcdefghijklmnopqrstuvwxyz'
    s = ''
    for _ in range(file_size):
        idx = random.randint(0,25)
        s += basis[idx]
    return s

with open('./all_file.txt','r',encoding='utf-8') as f:
    lines = f.readlines()

import time

file_size_list = [1024]
for size in file_size_list:
    f_size = size * 1024
    cnt = 0
    # save_file_name = './write_' +  str(size) + 'MB.txt'
    save_w_time_list = []

    for line in lines[0:1024]:
        file_name = root + line[:-1]
        time_start = time.time()

        try:

            with open(file_name,'w') as f:
                f.write(random_file_content(f_size))
            time_cur = time.time()
            save_w_time_list.append(time_cur-time_start)
            if cnt % 10 == 0:
                print(cnt, "s: " + file_name)
        except:
            print("err" + file_name)
            
        cnt += 1
    '''
    with open(save_file_name,'w',encoding='utf-8') as save_f:
        for i in save_w_time_list:
            save_f.write(str(i) + '\n')

    '''
            

