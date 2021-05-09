#coding: utf-8

import os

basis = 'abcdefghijklmnopqrstuvwxyz'
n = 7
# root should '/' as end
root = '/home/ubuntu/shuitang/GraduationProject/wykfs/' + 'tmp/tmp_pmem/'
all_dir_list = []
for i in range(0,n+1):
    for j in range(26):
        s = basis[j] + ('/' + basis[j])*i
        all_dir_list.append(s)
        try:
            os.mkdir(root + s)
        except:
            continue