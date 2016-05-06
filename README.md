CScript是一个脚本执行模块，用于嵌入到宿主程序中，是宿主具有执行脚本语言的能力，方便最终用户或者程序编写者扩充程序的功能。

CScript使用类似于C的语法，但是删减了作为嵌入脚本引擎不太会用到的功能，相比C89，下面这些功能是CScript所没有的：
1. struct、enum、位域
2. 指针声明、操作包括数组的相关操作
3. 函数声明、定义和调用
4. 编译预处理、编译指令

为了解决没有数组的缺陷，目前提供了array类型，可以如同声明整数（int）类型变量一样声明一个array类型变量。也可以用CreateArray函数来创建一个数组。像这样：
array arr = CreateArray(0x80, \x11, "hello, world");
也可以通过索引访问数组元素
int x = arr[0];
可以为它添加元素
arr.add(0, "hehe");
删除指定元素
arr.del(1);

脚本引擎提供了一些基本的库函数，比如time函数返回了一个具有6个元素的数组array对象，依次表示调用time时的本地计算机‘年’、‘月’、‘日’、‘时’、‘分’、‘秒’数据。

脚本引擎最主要的功能就是嵌入到宿主中来运行，宿主可以提供更多的功能给脚本编写者使用，关于这部分内容，请等待我有空的时候慢慢写。
