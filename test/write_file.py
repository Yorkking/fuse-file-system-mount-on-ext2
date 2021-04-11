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

with open('./all_file.txt','r',encoding='utf-8') as f:
    lines = f.readlines()

cnt = 0
for line in lines:
    file_name = root + line[:-1]
    f_size = 1000
    try:

        with open(file_name,'w') as f:
            f.write(random_file_content(f_size))
        if cnt % 50 == 0:
            print("s: " + file_name)
    except:
        print(file_name)
        break
    cnt += 1
   
        

