# shell lab ans

参考:[CSAPP-Lab07 Shell Lab 深入解析](https://zhuanlan.zhihu.com/p/492645370)

网上答案很多，但是目前我看到的所有答案都有一个问题：对于`builtin_cmd`函数中的`bg`、`fg`操作没有阻塞信号，这肯定是不对的。因为若用`fg`命令恢复了一个前台job，但是在调用`waitfg`之前该前台job已经结束（正常结束或因信号结束），此时若没有阻塞`SIGCHILD`信号，则job会被`sigchld_handler`函数回收，之后调用`waitfg`中的`sigsuspend`函数后会永远pause在这里。

不确定的地方就将所有信号都阻塞了，这样应该没有问题。

此外，所有的系统调用应当判断错误，这里大部分未做判断。