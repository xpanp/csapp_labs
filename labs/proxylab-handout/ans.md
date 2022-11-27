# proxy lab ans

参考：[CSAPP-Lab09 Proxy Lab 深入解析](https://zhuanlan.zhihu.com/p/497982541)

## Note

对于自动测试脚本`driver.sh`，第301行需要将

```bash
./nop-server.py ${nop_port} &> /dev/null &
```

改为

```bash
python3 nop-server.py ${nop_port} &> /dev/null &
```

## 后记

这里没有对异常处理函数进行处理，也就是没有达到lab要求中的健壮，但是70分得到了。cache部分不怎么想写了把别人的代码改了改，别人的代码中基本也都写的比较草率，只是达到满分就结束了。

`2022-11-27`, 由于个人已经相对有编程经验了，另一方面迫切的想继续学其他内容（自己的学生生涯也就剩六七个月了），csapp这门课确实学的比较草率，从`2022-11-03`到今天也就24天，之后有空再把TODO补上，书再读一遍。