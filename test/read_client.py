# read_client.py
import time
cnt = 1
time_list = []
root = '/home/ubuntu/shuitang/GraduationProject/wykfs/' + 'tmp_pmem/'

#read_file, save_file = './read_log_normal_trace.txt', './log_normal_time_list.txt'
read_file, save_file = './read_evenly_trace.txt', './log_evenly_time_list.txt'

text = []
with open(read_file,'r',encoding='utf-8')as f:
    file_list = f.readlines()
    for file in file_list:
        file_name = file[0:-1]
        time_start = time.time()
        try:
            with open(root + file_name,'r') as f1:
                t = f1.readlines()
                text.append(t)
                cnt += 1
                if cnt % 100 == 0 and len(t) >= 1:
                    print(t, len(t[0]))
                elif len(t) < 1:
                    print(root + file_name)
                time_cur = time.time()
                time_list.append(time_cur-time_start)
        except:
            print("error: ", root + file_name)
            
with open(save_file,'w',encoding='utf-8') as f:
    for i in time_list:
        f.write(str(i) + '\n')

with open('./ttt.txt','w')as f:
    for line in text:
        f.write(str(line))