## 6. 算法

### 6.1 算法库纵览

![Snipaste_2021-02-28_10-51-18](file://E:\Desktop\My-STL-Note\image\Snipaste_2021-02-28_10-51-18.png?lastModify=1615426560)

在本章中，除了上述较新标准和C库下的算法实现没有在书中提到之外，基本上都可以在书中找到。不过并不是每一个算法都值得我们特别注意，事实上，我们阅读源代码的目的更多为了学习一些比较重要算法的思想和SGI STL为实现之而使用的编程技法，因此在这里我仅仅列出如下一些个人觉得值得学习的算法：

|       算法类型       |                            算法名                            |
| :------------------: | :----------------------------------------------------------: |
|       数值算法       |                       `power`(SGI专属)                       |
|      非质变算法      |                     `search`、`find_end`                     |
|       质变算法       |                       `copy`、`rotate`                       |
|       集合操作       | `set_union`、`set_intersection`、`set_difference`、  `set_symmetric_difference`、`includes`、`merge`、`inplace_merge` |
|       划分操作       |                         `partition`                          |
|       排序操作       |    `partial_sort`、`sort`、`nth_element`、`stable_sort()`    |
|       排列操作       |            `next_permutation`、`pre_permutation`             |
|     二分搜索操作     | `lower_bound`、`upper_bound`、`equal_range`、`binary_search` |
|        堆操作        |                              略                              |
|    最小/大值操作     |                              略                              |
| 未初始化存储上的操作 |                              略                              |



### 6.2 算法的一般形式

一般而言，STL的算法几乎都有着如下的形式和规范：

- 大多数的算法的前两个参数都是一对迭代器，采用前闭后开的表示方法，用来以指出算法的操作区间；
- 许多STL算法都会进行重载，提供至少两个版本，一个版本采用默认的运算，另一个版本可以允许用户提供一个可调用对象，以作为自定义策略使用。而且有些算法还会在算法名的尾部加后缀`_if`进行标识；
- 质变算法通常提供两个版本：①一个in-place（原地进行）版本，例如`sort()`算法就是典型的原地质变算法；②一个copy版本，这种版本的算法进行会在名字的尾部加以`_copy`标识，允许将执行的结果拷贝到一个指定的容器之中。



### 6.3 各算法实现

C++算法库中的大部分算法实现都主要集中在[stl_algo.h](stl_algo.h)、[stl_algobase.h](stl_algobase.h)、[stl_numeric.h](stl_numeric.h)、[stl_heap.h](stl_heap.h)这几个头文件中，从源文件的角度上讲这些算法可以分成普通算法、基础算法、数值算法等类型，但是这里在展开解释的时候还是主要按照上述cppreference的一般分类来进行。

#### 6.3.1 数值算法

##### 6.3.1.1 power

该乘幂算法只支持对正次幂的计算，不过稍微改造下就可以支持对负次幂的计算。这个算法其实利用到了幂次运算的分解原理：



根据这个递推关系，对于x的13（二进制1101）次方，我们其实可以先计算，这个值既需要保留以用做计算最后结果，而且还可以用来计算下一个分解式，例如，即存在一值多用。这也便是下面有两个while的缘故，其中第一个while就是用来计算最小的分解式，而第二个while是用来计算后续的更大的分解式，并将每一个分解式乘到result之中。

```c++
 template<class T, class Integer, class MonoidOperatoion>
 T power(T x, Integer n, MonoidOperatoion op) {
     if (n == 0) return 1;
     else {
         while ((n & 1) == 0) {
             n >>= 1;
             x = op(x, x);
         }
 
         T result = x;
         n >>= 1;
         while (n != 0) {
             x = op(x, x);
             if ((n & 1) != 0)
                 result = op(result, x);
             n >>= 1;
         }
         return result;
     }
 }
 
 template<class T, class Integer>
 inline T power(T x, Integer n) {
     return power(x, n, multiplies<T>());
 }
```

> 这个算法与《剑指offer》面试题16的查考知识点相同。



#### 6.3.2 非质变算法

##### 6.3.2.1 search

SGI STL中的`search()`算法采用的是典型的暴力搜索方法。对于一个大序列X和小序列x而言，它首先会在大序列X中查找与小序列x第一个元素相同的元素，然后在这个元素之后挨个匹配大序列与小序列的元素，直到完全匹配；否则重新在剩下的序列中执行上面的操作。

```c++
 template <class _ForwardIter1, class _ForwardIter2>
 _ForwardIter1 search(_ForwardIter1 __first1, _ForwardIter1 __last1,
                      _ForwardIter2 __first2, _ForwardIter2 __last2) 
 {
   // Test for empty ranges
   if (__first1 == __last1 || __first2 == __last2)
     return __first1;
 
   // Test for a pattern of length 1.
   _ForwardIter2 __tmp(__first2);
   ++__tmp;
   if (__tmp == __last2)
     return find(__first1, __last1, *__first2);
 
   // General case.
 
   _ForwardIter2 __p1, __p;
   __p1 = __first2; ++__p1;
   _ForwardIter1 __current = __first1;
   //这里采用的就是暴力搜索法
   while (__first1 != __last1) {
     __first1 = find(__first1, __last1, *__first2);
     if (__first1 == __last1)
       return __last1;
 
     __p = __p1;
     __current = __first1; 
     if (++__current == __last1)
       return __last1;
 
     while (*__current == *__p) {
       //若所有元素都对上了，则返回该模式序列在欲查找序列上的首元素迭代器
       if (++__p == __last2)
         return __first1;
       if (++__current == __last1)
         return __last1;
     }
 
     ++__first1;
   }
   return __first1;
 }
```



##### 6.3.2.2 find_end

`find_end()`算法会在大序列中找出小序列在其中最后一次出现的首迭代器。它的原理也非常简单，就是不断地使用`search()`算法，若在大序列中找到一个与小序列对应的子序列，那么就继续调用`search()`直到再也找不到。那么最后一次所找到的对应子序列的首迭代器就是我们所期望的结果。

```c++
 template <class _ForwardIter1, class _ForwardIter2>
 _ForwardIter1 __find_end(_ForwardIter1 __first1, _ForwardIter1 __last1,
                          _ForwardIter2 __first2, _ForwardIter2 __last2,
                          forward_iterator_tag, forward_iterator_tag)
 {
   if (__first2 == __last2)
     return __last1;
   else {
     _ForwardIter1 __result = __last1;
     while (1) {
       _ForwardIter1 __new_result
         = search(__first1, __last1, __first2, __last2);
       if (__new_result == __last1)
         return __result;
       else {
         __result = __new_result;
         __first1 = __new_result;
         ++__first1;
       }
     }
   }
 }
```



#### 6.3.3 质变算法

##### 6.3.3.1 copy

SGI STL V3.3上的`copy()`算法实现乍看之下和《STL源码剖析》中的代码不一样，但实际上两者并没有什么特殊的区别，反而觉得SGI STL V3.3显得更稍显得巧妙。它是典型的学习C++模板技法、静态多态/函数匹配（解析）机制的好例子。

![Snipaste_2021-03-10_10-07-26](file://E:\Desktop\My-STL-Note\image\Snipaste_2021-03-10_10-07-26.png?lastModify=1615426560)

在原先的（也即《STL源码剖析》）实现中，`copy()`会按照如上图所示的顺序（假设`copy()`的特化版本不使用）进行调用：

1. 首先根据模板实参推断的结果调用拷贝分派器`__copy_dispatch`中的静态成员函数`__copy()`，若是原始指针类型，则会调用它的偏特化版本；否则调用泛化版本。
2. 对于泛化版本，它会根据迭代器的类型由编译器选择InputIterator版本的`__copy()`实现函数，或者选择RandomAccessIterator版本的`__copy()`实现函数，而后者又会调用一个叫做`__copy_d()`的函数完成真正的操作。对于泛化版本的拷贝实现，我们有一点是可以确认的：那就是它们都会调用拷贝元素的非平凡（有实际意义）拷贝赋值运算符来完成拷贝工作。
3. 而对于指针特化版本而言，它会根据指针所指向的元素类型支持非平凡trivial拷贝赋值运算符的与否来选择相应的`__copy_t()`（t代表trivial）版本。若指针指向元素类型不支持non-trivial拷贝运算符，那么编译器就会选择实际以`memmove()`完成拷贝工作的`__copy_t()`版本；而若指针指向的元素类型支持non-trivial拷贝赋值运算符，那么编译器就会为其选择实际以调用`__copy_d()`完成拷贝工作的`__copy_t()`版本。

由于实际上这个版本的SGI STL `copy()`实现略显复杂。当算法的前两个传入参数为指向支持non-trivial拷贝赋值运算符元素类型的指针时，其所调用的偏特化`__copy_dispatch::__copy_t()`绕了一大圈竟然还是最后调用了模板参数为`template<class _RandomAccessIterator ....>`的`__copy_d()`函数😂！何必呢？！

所以SGI STL V3.3中的`copy()`实现直接改了，它直接将迭代器指向元素是否支持non-trivial拷贝赋值运算符的特性提取工作放到了一开始的`copy()`入口函数中！**1）**若一个迭代器或者指针指向的元素支持non-trivial拷贝赋值运算符，那么编译器就只会为其选择泛化版本的`__copy_dispatch::copy()`静态成员函数；**2）**若不支持non-trivial拷贝赋值运算符但迭代器也不是原始指针，那么也会选择泛化版本；**3）**但若输入的不仅是一个指针而且指针指向的元素类型也不支持non-trivial的拷贝赋值运算符，那么编译器会为其选择指针偏特化的版本。这种逻辑相比于《STL源码剖析》上所述的`copy()`实现简洁了很多，而且安全可靠些，具体如下图所示：

![copy实现](file://E:\Desktop\My-STL-Note\image\copy实现.png?lastModify=1615426560)

```c++
 template <class _InputIter, class _OutputIter, class _Distance>
 inline _OutputIter __copy(_InputIter __first, _InputIter __last,
                           _OutputIter __result,
                           input_iterator_tag, _Distance*)
 {
   for ( ; __first != __last; ++__result, ++__first)
     *__result = *__first;
   return __result;
 }
 
 template <class _RandomAccessIter, class _OutputIter, class _Distance>
 inline _OutputIter
 __copy(_RandomAccessIter __first, _RandomAccessIter __last,
        _OutputIter __result, random_access_iterator_tag, _Distance*)
 {
   for (_Distance __n = __last - __first; __n > 0; --__n) {
     *__result = *__first;
     ++__first;
     ++__result;
   }
   return __result;
 }
 
 template <class _Tp>
 inline _Tp*
 __copy_trivial(const _Tp* __first, const _Tp* __last, _Tp* __result) {
   memmove(__result, __first, sizeof(_Tp) * (__last - __first));
   return __result + (__last - __first);
 }
 
 template <class _InputIter, class _OutputIter, class _BoolType>
 struct __copy_dispatch {
   static _OutputIter copy(_InputIter __first, _InputIter __last,
                           _OutputIter __result) {
     typedef typename iterator_traits<_InputIter>::iterator_category _Category;
     typedef typename iterator_traits<_InputIter>::difference_type _Distance;
     return __copy(__first, __last, __result, _Category(), (_Distance*) 0);
   }
 };
 
 template <class _Tp>
 struct __copy_dispatch<_Tp*, _Tp*, __true_type>
 {
   static _Tp* copy(const _Tp* __first, const _Tp* __last, _Tp* __result) {
     return __copy_trivial(__first, __last, __result);
   }
 };
 
 template <class _Tp>
 struct __copy_dispatch<const _Tp*, _Tp*, __true_type>
 {
   static _Tp* copy(const _Tp* __first, const _Tp* __last, _Tp* __result) {
     return __copy_trivial(__first, __last, __result);
   }
 };
 
 template <class _InputIter, class _OutputIter>
 inline _OutputIter copy(_InputIter __first, _InputIter __last,
                         _OutputIter __result) {
   typedef typename iterator_traits<_InputIter>::value_type _Tp;
   typedef typename __type_traits<_Tp>::has_trivial_assignment_operator
           _Trivial;
   return __copy_dispatch<_InputIter, _OutputIter, _Trivial>
     ::copy(__first, __last, __result);
 }
```



##### 6.3.3.2 rotate

`rotate()`算法适用于随机访问迭代器的版本理解起来比较复杂，我暂时没看懂。不过适用于前向和双向迭代器的版本还是比较好理解的。个人这部分的总结还是看书吧，侯捷老师已经写的不错了。

> 《剑指offer》中有一道题和这个序列的旋转有关，主要是求旋转数组中的最小值，即面试题11。

```c++
 template <class _EuclideanRingElement>
 _EuclideanRingElement __gcd(_EuclideanRingElement __m,
                             _EuclideanRingElement __n)
 {
   while (__n != 0) {
     _EuclideanRingElement __t = __m % __n;
     __m = __n;
     __n = __t;
   }
   return __m;
 }
 
 template <class _ForwardIter, class _Distance>
 _ForwardIter __rotate(_ForwardIter __first,
                       _ForwardIter __middle,
                       _ForwardIter __last,
                       _Distance*,
                       forward_iterator_tag) {
   if (__first == __middle)
     return __last;
   if (__last  == __middle)
     return __first;
 
   _ForwardIter __first2 = __middle;
   do {
     swap(*__first++, *__first2++);
     if (__first == __middle)
       __middle = __first2;
   } while (__first2 != __last);
 
   _ForwardIter __new_middle = __first;
 
   __first2 = __middle;
 
   while (__first2 != __last) {
     swap (*__first++, *__first2++);
     if (__first == __middle)
       __middle = __first2;
     else if (__first2 == __last)
       __first2 = __middle;
   }
 
   return __new_middle;
 }
 
 
 template <class _BidirectionalIter, class _Distance>
 _BidirectionalIter __rotate(_BidirectionalIter __first,
                             _BidirectionalIter __middle,
                             _BidirectionalIter __last,
                             _Distance*,
                             bidirectional_iterator_tag) {
   if (__first == __middle)
     return __last;
   if (__last  == __middle)
     return __first;
 
   __reverse(__first,  __middle, bidirectional_iterator_tag());
   __reverse(__middle, __last,   bidirectional_iterator_tag());
   
   /* 下面的实现就是等于：reverse(_first,__middle,bidirectional_iterator_tag());
     只不过这里需要返回旋转数组第二部分首元素的迭代器罢了 */
   while (__first != __middle && __middle != __last)
     swap (*__first++, *--__last);
 
   if (__first == __middle) {
     __reverse(__middle, __last,   bidirectional_iterator_tag());
     return __last;
   }
   else {
     __reverse(__first,  __middle, bidirectional_iterator_tag());
     return __first;
   }
 }
 
 template <class _RandomAccessIter, class _Distance, class _Tp>
 _RandomAccessIter __rotate(_RandomAccessIter __first,
                            _RandomAccessIter __middle,
                            _RandomAccessIter __last,
                            _Distance *, _Tp *) {
   /* 下面这个我暂时不理解 */
   _Distance __n = __last   - __first;
   _Distance __k = __middle - __first;
   _Distance __l = __n - __k;
   _RandomAccessIter __result = __first + (__last - __middle);
 
   if (__k == 0)
     return __last;
 
   else if (__k == __l) {
     swap_ranges(__first, __middle, __middle);
     return __result;
   }
 
   _Distance __d = __gcd(__n, __k);
 
   for (_Distance __i = 0; __i < __d; __i++) {
     _Tp __tmp = *__first;
     _RandomAccessIter __p = __first;
 
     if (__k < __l) {
       for (_Distance __j = 0; __j < __l/__d; __j++) {
         if (__p > __first + __l) {
           *__p = *(__p - __l);
           __p -= __l;
         }
 
         *__p = *(__p + __k);
         __p += __k;
       }
     }
 
     else {
       for (_Distance __j = 0; __j < __k/__d - 1; __j ++) {
         if (__p < __last - __k) {
           *__p = *(__p + __k);
           __p += __k;
         }
 
         *__p = * (__p - __l);
         __p -= __l;
       }
     }
 
     *__p = __tmp;
     ++__first;
   }
 
   return __result;
 }
 
 template <class _ForwardIter>
 inline _ForwardIter rotate(_ForwardIter __first, _ForwardIter __middle,
                            _ForwardIter __last) {
   return __rotate(__first, __middle, __last,
                   __DISTANCE_TYPE(__first),
                   __ITERATOR_CATEGORY(__first));
 }
```