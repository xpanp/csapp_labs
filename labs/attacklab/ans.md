# attack lab answer

参考该答案：[深入理解计算机系统attack lab](https://blog.csdn.net/weixin_41256413/article/details/80463280)

## ci1

注意：参考博客里面答案应该错了，高位的四字节0应该放在后面。

```shell
./hex2raw <ci1.txt >ci1_hex.txt
./ctarget -qi ci1_hex.txt
```

## ci2

输入的字符串需要修改返回地址，使得`getbuf`函数返回时调用字符串中实现的函数。

这里关键是，字符串中实现一个函数，该函数需要设置`%rdi`寄存器的值为cookie值，并设置`%rsp`寄存器中存指向`touch2`函数的地址。当这个函数返回的时候，就可以自动调用`touch2`函数了，并且第一个参数也设置正确。

获取字符串中函数的二进制值：
```shell
gcc -c ci2.s -o ci2.o
objdump -d ci2.o > ci2_gcc.s
```

得到答案：
```shell
./hex2raw <ci2.txt >ci2_hex.txt
./ctarget -qi ci2_hex.txt
```

## ci3

这里理解了很久，看了很多博客，原因在于忘了`getbuf`函数返回前释放了自己申请的0x28栈空间，因此我们输入的字符串没有超过0x28的部分，在新的函数被调用后很有可能被修改，因此最保守的方法就是将`touch3`函数需要的字符串参数放在超过0x28的部分。我们自己的函数在调用过后被覆盖了也没关系。

```shell
gcc -c ci3.s -o ci3.o
objdump -d ci3.o > ci3_gcc.s
```

得到答案：
```shell
./hex2raw <ci3.txt >ci3_hex.txt
./ctarget -qi ci3_hex.txt
```