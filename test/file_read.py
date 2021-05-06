#coding:utf-8
root = '/home/ubuntu/shuitang/GraduationProject/wykfs/' + 'tmp_pmem/'

with open('./all_file.txt','r',encoding='utf-8') as f:
    lines = f.readlines()

cnt = 0
for line in lines:
    file_name = root + line[:-1]
    with open(file_name,'r') as f:
        s = f.readlines()
        if cnt % 100 == 0:
            print(s)
        cnt += 1