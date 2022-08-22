# 基于 fuse 的多级调度文件系统

下面介绍一下该目录下的各种文件。

## 核心文件

main.c: 实现 fuse 提供的文件接口

util.h: 定义了存在 DRAM 的目录多叉树的数据结构
util.c: 实现上述结构的各种操作 

control.c, control.h: 实现调度算法，并发控制

目前已经完成了持久化到 pmem 的操作，同时也完成了 pmem 和 disk 的动态调度的过程。

- TODO: 
    - [x] 已完成：项目的存在数据竞争的问题，即从线程在进行 pmem 与 disk 的调度时，主线程文件系统的文件被访问时可能会起数据竞争。
    - [x] 已支持变长文件，后期将使用 content 链表进行改进。
    - [x] 已解决原子性操作问题。
    - [x] 已完成测试
    - [] 需要考虑并发删除问题，解决办法
        - 树形加锁：1. 从需要删除的目录出发，一直给其所有子结点加锁，若全部加锁成功，2. 则删除目录
        - 但是，需要考虑删除效率，这样做的话，其实删除所需时间会很长（外部进程调用删除，会导致长时间等待）
        - 优化方案一：在进行 2. 删除目录时，可以考虑把目录结点的父节点的下一级指针置为空，然后返回给外部进程信息，之后把子目录的删除交给一个删除线程完成。
            - 但是，这样并没有解决第 1 步的时间开销问题。
        - 优化方案二：从读/写文件的操作修改，实现读/写文件的操作时，考虑从根节点到写/读的文件上的路径加上不可删除锁（实际上可以每个结点设置一个不可删除计数），写/读完再释放。然后删除时，判断删除目录的不可删除计数是否为 0 ，若是则进行优化方案一的操作；否则，直接返回失败信息。需要修改 find 接口，以及如何返回的接口。
_____
# FUSE-based multi-level scheduling file system

 ## Core components

 ```main.c```: implements the file interface provided by FUSE

 ```util.h```: defines the data structure of the directory multi-tree that exists in DRAM

 ```util.c```: implements various operations of the above structure

 ```control.c```, ```control.h```: implements scheduling algorithm, concurrency control

 ## Run-time dependencies
 Choose one of the following applications, according to the host platform:

 🍎[MacFUSE](https://osxfuse.github.io) (Mac OS)

 🪟[WinFUSE](https://github.com/billziss-gh/winfuse) (Microsoft Windows)

 🪟[Dokan](https://dokan-dev.github.io) (Microsoft Windows)

 🪟[cxFUSE](https://github.com/crossmeta/cxfuse) (Microsoft Windows)

 ## Project status

 **Fixed**

 * The operation of persistence to pmem has been completed, and the process of dynamic scheduling of pmem and disk has also been completed.

 * The project has a data race problem: when the slave thread schedules pmem and disk, there may be a data race when the file of the main thread file system is accessed.  Variable-length files have been supported, and will be improved later using the content list.  Atomic operation issue has been resolved.

 **To do**

 * Need to consider the problem of concurrent deletion, the solution—

 [Tree locking](#project-status): Starting from the directory that needs to be deleted, permanently lock all its child nodes.  If all locking operations are successful, delete the directory.  However, the deletion efficiency needs to be considered.  Using this method, deletion will take a long time because the external process calls delete, which will lead to a long wait.

 [Optimization scheme 1](#project-status): When deleting a directory, consider setting the next-level pointer of the parent node of the directory node to be empty, then pass this to the external process, and then hand over deletion of the subdirectory to a deletion thread to complete.  However, this does not solve the time overhead.

 [Optimization scheme 2](#project-status): When implementing the operation of reading/writing files, consider the path from the root node to the file to be written/read plus an undeletable lock [you can set one undeletable lock per node], then release after writing/reading. Then, when deleting, determine whether the undeletable count of the deleted directory is 0. If true, perform the optimization scheme 1; otherwise, return the failure information directly.  Need to modify the find interface, and determine how to return the interface.

 ## Additional information & references

 * [Original fuse-ext2 project repository](https://github.com/alperakcan/fuse-ext2) (ignores pull requests since ```2020-7-11```,
 risk of data loss on write operations due to bugs)
 * [List of contributors & forks](https://github.com/alperakcan/fuse-ext2/network/members) (original project)
 * [MacFUSE project page](https://osxfuse.github.io) (formerly OSXFUSE)
 * [Original project documentation](https://github.com/osxfuse/osxfuse/wiki/Ext) (MacFUSE wiki)
 * [FileSystem in UserSpace (FUSE)](https://en.wikipedia.org/wiki/Filesystem_in_Userspace) (general description - Wikipedia)
