#! /usr/bin/zsh

size_array=(1 2 4 8 16 32 64 128 256 512 1024)
thread_nums=(1 2 4 8 16 32)

for loop in 1 2 4 8 16 32
do
    ./mult_write_disk 32 $loop < ./all_file.txt > "./concurrency/disk_${loop}thread_32KB.txt"  &&
    rm -r ../concurrency_test/tst_rt/a/ && 
    rm -r ../concurrency_test/tst_rt/b/ &&
    mkdir ../concurrency_test/tst_rt/a &&
    mkdir ../concurrency_test/tst_rt/b
done



