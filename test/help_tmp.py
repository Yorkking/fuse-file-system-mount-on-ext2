#coding:utf-8
root = '/home/ubuntu/shuitang/GraduationProject/wykfs/' + 'tmp_pmem/'


def random_file_content(file_size):
    import random
    basis = 'abcdefghijklmnopqrstuvwxyz'
    s = ''
    for _ in range(file_size):
        idx = random.randint(0,25)
        s += basis[idx]
    return s

file_name = '../tmp_pmem/a/chyvncfxofvtumwpajck.txt'

with open(file_name,'w')as f:
    f.write(random_file_content(1024*20))
