# bomblab answer

[深入理解计算机系统bomb实验](https://blog.csdn.net/CXY_YZ/article/details/115585852)，这篇文章答案已经写的很详细了，但是最后一个隐藏关卡的`fun7`函数想了半天没想明白，记录一下。

## phase_1

Answer: `Border relations with Canada have never been better.`

## phase_2

Answer: `1 2 4 8 16 32`

## phase_3

第一个数必须在0-6之间，第二个数由第一个数决定。原函数是个`switch-case`结构，有多个答案。

Answer: `1 311`

## phase_4

Answer: `7 0`

隐藏关卡需输入:

Answer: `7 0 DrEvil`

## phase_5

Answer: `9?>567`

## phase_6

Answer: `4 3 2 1 6 5`

##  secret_phase

Answer: `22`

原答案到`fun7`这个函数就比较简单了，这里仔细记录一下。根据汇编，还原出原始的函数:

```c
int fun7(tree_node* p, int val) {
    if (p == NULL)
        return 0xffffffff;
    if (val < p->val) {
        p = p->left;
        int res = fun7(p, val);
        return 2 * res;
    } else {
        if (val == p->val)
            return 0;
        p = p->right;
        int res = fun7(p, val);
        return 2 * res + 1;
    }
}
```

由于最后要求的返回值必须是2，则递归遍历树时，返回的值依次为0->1->2，因此该结点为root->left->right.

在gdb中调试得到值为22:

```shell
(gdb) print **(*(0x6030f0+0x8)+0x10)
$45 = 22
```