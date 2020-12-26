# SGI-STL

## 1. 了解STL

### 1.1 STL的组成

STL有如下6大组件组成：

1. **容器containers**
2. **算法algorithms**
3. **迭代器iterators**
4. **函数对象functors**
5. **适配器adapters**：又分成迭代器适配器、容器适配器、函数适配器
6. **分配器(配置器)allocators**

它们之间有着非常紧密的关系，其中①**容器Containers通过空间分配器Allocators来获得数据存储空间**；②**算法Algorithms通过迭代器Iterators来存取容器中的内容**；③**函数对象/仿函数Functors可以协助算法采用不同的策略实现不同的操作**；④**适配器Adaptors可以修饰或者套接函数对象、容器（形成容器适配器、迭代器适配器和）**。

<img src="..\image\jklfsda.jpg" alt="jklfsda" style="zoom: 50%;" />



### 1.2 SGI STL文件分布

gcc采用的STL实现版本正是SGI STL，其相关的实现文件大致分布在/usr/include/c++/(版本号)这个文件之中。这些文件大致可以分成如下5组：

1. C++标准规范下的C头文件。例如cstdio、cstdlib等；
2. C++标准程序库中不属于STL范畴的文件。例如string、stream等；
3. STL标准头文件。例如vector、deque等；
4. C++标准定案前的一些STL头文件。例如vector.h、dque.h等；
5. SGI STL真正的内部实现文件。例如std_vector.h、stl_deque.h等。

其中我们比较关注的两个文件目录为/usr/include/c++/10.2.0和/usr/include/c++/10.2.0/bits，前者存放了C++标准下的C头文件、非STL范畴的C++'头文件、标准STL头文件以及一些老版本的STL标准头文件，而后者存放了真正的SGI STL内部实现文件。类似的在Win10下安装的mingw软件的安装目录中也会发现类似的目录和文件。



### 1.3 stl_config.h头文件

在上述的bits目录下有一个比较重要的配置文件stl_config.h。文件本身分成两个部分：前半部分检测编译器对C++标准的支持程度并定义类似于功能测试宏的宏，后半部分根据前半部分定义的宏定义出一些方便程序编写的宏或者对不支持的语言特定做出一些弥补性的定义。例如编译器若不支持bool类型，则该头文件会定义`#typedef bool int`、`#define true 1`这样的语句。以及例如会将空模板参数列表`template<>`定义成`__STL_TEMPLATE_NULL`方便后续使用。



