import os    

def random_file_name(name_len):
    import random
    basis = 'abcdefghijklmnopqrstuvwxyz'
    s = ''
    for i in range(name_len):
        idx = random.randint(0,25)
        s += basis[idx]
    s += '.txt'
    
    return s

def random_file_content(file_size):
    import random
    basis = 'abcdefghijklmnopqrstuvwxyz'
    s = ''
    for i in range(file_size):
        idx = random.randint(0,25)
        s += basis[idx]
    return s


basis = 'abcdefghijklmnopqrstuvwxyz'
n = 7
root = r'/home/ubuntu/shuitang/GraduationProject/wykfs/src/testFs/test/'
all_dir_list = []
for i in range(0,n+1):
    for j in range(26):
        s = basis[j] + ('/' + basis[j])*i
        all_dir_list.append(s)
        try:

            os.mkdir(root + s)
        except:
            continue

# 均匀分布写入
# 每个目录写入的期望为 10 
test_num = len(all_dir_list) * 10 
file_list = []
import random
for i in range(test_num):
    idx = random.randint(0,len(all_dir_list)-1)
    file_name_len = 20
    file_content_size = 50
    file_name = all_dir_list[idx] + '/' + random_file_name(file_name_len)
    file_list.append(file_name)
    with open(root + file_name,'w')as f:
        f.write(random_file_content(file_content_size))
        

# 把文件名保存
with open("./all_file.txt",'w',encoding='utf-8') as f:
    for line in file_list:
        f.write(line + '\n')


## 均匀分布的读 trace
####################
read_file_list = []
with open("./all_file.txt",'r',encoding='utf-8')as f:
    s = f.readlines()
read_num = 1000

for i in range(read_num):
    idx = random.randint(0,len(s)-1)
    read_file_list.append(s[idx])
with open("./read_evenly_trace.txt",'w',encoding='utf-8')as f:
    for line in read_file_list:
        f.write(line)

# 长尾分布的 trace
# 使用对数正态分布来模拟
read_file_list = []
with open("./all_file.txt",'r',encoding='utf-8')as f:
    s = f.readlines()
import random
random.shuffle(s)

import numpy as np
mu, sigma = 1., 1.
read_num = 1000

t = np.random.lognormal(mu,sigma,read_num)
#print(t)
for i in t:
    idx = int(np.floor(i)) % len(s)
    read_file_list.append(s[idx])
    
with open("./read_log_normal_trace.txt",'w',encoding='utf-8')as f:
    for line in read_file_list:
        f.write(line)