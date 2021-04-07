#! /usr/bin/zsh

gcc -o control control.c util.c -lpmemobj -lpmem -pthread -I. &&
rm /home/ubuntu/shuitang/GraduationProject/tmp_fs.pmem &&
rm -r /home/ubuntu/shuitang/GraduationProject/control_fs && 
mkdir /home/ubuntu/shuitang/GraduationProject/control_fs