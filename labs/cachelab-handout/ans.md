# cache lab ans

## Part A: Cache Simulator

这部分实现较为简单，不做详细说明。

不使用全局变量，而是用一个结构体`cache_ctl`来包含所有需要的数据。

`lru`使用计数来实现，每读取一次操作时间戳加一，因此时间戳最小的就是最近最久未使用的。

具体实现见代码。

## Part B: Optimizing Matrix Transpose

这部分较难，参考[CSAPP实验之cache lab](https://zhuanlan.zhihu.com/p/79058089), [CSAPP - Cache Lab的更(最)优秀的解法](https://zhuanlan.zhihu.com/p/387662272)。

主要需要想清楚一个blocking在cache中占用的set是间隔的。比如`32*32`的数组，此时以`8*8`分块，则该块在cache中是每4个set占用一个，一共占用8个。