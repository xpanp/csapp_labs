## malloc lab ans

[CSAPP:Lab5-Malloc Lab](https://zhuanlan.zhihu.com/p/150100073)

- 隐式链表
    - first fit
    
        主要为书上代码，见`mm_implicit_first_fit.c`。

    - next fit 
    
        见`mm_implicit_next_fit.c`，注意`coalesce`时要对`pre_listp`位置更新。

- 显式链表

    该项目默认编译为32位程序，因此指针占4byte。

    需要注意，当申请的块放入空闲链表时，`payload`的最前面存放的是两个链表指针。然而当该块被申请使用后，存放链表指针的位置可以放数据。由于这里一开始没想通，绕了很多弯路，同时也降低了空间的利用率。

    参考的代码里面`mm_realloc`的实现有一些问题。

    修改过后的版本见`mm_free_list.c`。


- 分离链表

    TODO 自己实现一版