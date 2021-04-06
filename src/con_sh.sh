#! /usr/bin/zsh

gcc -o control control.c util.c -lpmemobj -lpmem -pthread -I. &&
rm /home/ubuntu/shuitang/GraduationProject/tmp_fs.pmem