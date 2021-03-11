#### 6.3.4 ==集合操作==

##### 6.3.4.1 set_union

取并集的实现比较好理解，即序列中若两个指针指向的元素相同，那么同时步进，否则只步进值小的那个。

```c++
template <class _InputIter1, class _InputIter2, class _OutputIter>
_OutputIter set_union(_InputIter1 __first1, _InputIter1 __last1,
                      _InputIter2 __first2, _InputIter2 __last2,
                      _OutputIter __result) {
  while (__first1 != __last1 && __first2 != __last2) {
    if (*__first1 < *__first2) {
      *__result = *__first1;
      ++__first1;
    }
    else if (*__first2 < *__first1) {
      *__result = *__first2;
      ++__first2;
    }
    else {
      *__result = *__first1;
      ++__first1;
      ++__first2;
    }
    ++__result;
  }
  return copy(__first2, __last2, copy(__first1, __last1, __result));
}
```



##### 6.3.4.2 includes

算法`include()`用来检查序列二[first2,last2)是否包含在序列一[first1,last1)中，它的实现理解可能很简单，但我觉得实际动手写可能思绪就有点错乱了。

```c++
template <class _InputIter1, class _InputIter2>
bool includes(_InputIter1 __first1, _InputIter1 __last1,
              _InputIter2 __first2, _InputIter2 __last2) {
  while (__first1 != __last1 && __first2 != __last2)
    if (*__first2 < *__first1)
      /* 即第二个序列还未在序列一中碰到与自己相同的元素就遇到了一个比自己大的元素，
      	那么说明第二个序列中并没有这个元素的存在，也即序列一并没有include序列二 */
      return false;
    else if(*__first1 < *__first2) 
      ++__first1;
    else
      ++__first1, ++__first2;

  //跳出循环后检查序列二是否到尾端了，若没有到说明序列二中有序列一没有的元素
  return __first2 == __last2;
}
```



##### 6.3.4.3 merge

```c++
template <class _InputIter1, class _InputIter2, class _OutputIter>
_OutputIter merge(_InputIter1 __first1, _InputIter1 __last1,
                  _InputIter2 __first2, _InputIter2 __last2,
                  _OutputIter __result) {
  while (__first1 != __last1 && __first2 != __last2) {
    if (*__first2 < *__first1) {
      *__result = *__first2;
      ++__first2;
    }
    else {
      *__result = *__first1;
      ++__first1;
    }
    ++__result;
  }
  return copy(__first2, __last2, copy(__first1, __last1, __result));
}
```



##### 6.3.4.4 inplace_merge

算法`inplace_merge()`的目的是使两个连接在同一个序列的有序子序列[first，middle)和[middle，last)进行原地归并，完成后所有元素整体有序。该算法的实现略显复杂，主要分成两种实现方案，一种是需要借助临时缓冲区的原地归并方案，但是当临时缓冲区不能成功分配时就会被迫使用另一种无缓冲区帮助的原地归并方案。需要注意的是，即使临时缓冲区生成成功，也并不意味着原地归并算法就可以那么顺利的进行，因为生成的临时缓冲区空间很有可能不足！

<img src="../../image/原地归并.png" alt="原地归并" style="zoom: 80%;" />

在整个算法中，临时缓冲区的生成由`inplace_merge_aux()`辅助函数执行，若临时缓冲区生成成功，它就会自动调用`__merge_adaptive()`函数，对序列中的元素进行带临时缓冲区的原地归并：

1. 若临时缓冲区的空间>=前半部分子序列长度，那么就将前半部分的数据暂时拷贝到临时缓冲区中，然后调用`merge()`归并回；

2. 若临时缓冲区的空间<前半部分子序列长度但>=后半部分子序列的长度，那么就将后半部分的数据暂时拷贝到临时缓冲区中，然后调用`__merge_backward()`后向归并回；

3. 若临时缓冲区的空间不仅小于前部分子序列长度还小于后半部分的长度，那么就会进入最后一个else分支中，这个分支逻辑有一点复杂：

   ​		它首先会借助二分搜索等操作，将前半部分的子序列和后半部分的子序列进行划分，前面划分成[first，first_cut)和[first_cut，middle)两部分；后面划分成[middle，second_cut)和[second_cut，last)两部分，并能够保证[first_cut，middle)中的所有元素都是大于[middle，second_cut)子序列中的元素。

   ​		然后我们对[first_cut，second_cut)部分以middle为中心进行有缓冲区的旋转，即调用`__rotate_adaptive()`。完成后，这部分中的元素必然都是有序的，且对整体而言返回的new_middle前的元素必然都是小于new_middle后的元素！这样我们此时只需要对new_middle前半部分和后半部分递归调用`__merge_adaptive()`函数就可以完成相应的原地归并操作。具体如下图所示：

<img src="../../image/merge_adaptive.png" alt="merge_adaptive" style="zoom:67%;" />

而当临时缓冲区生成失败时，`inplace_merge_aux()`辅助函数就会调用一个无需临时缓冲区就能够执行原地归并的实现函数`__merge_without_buffer()`。这个函数由于不适用临时缓冲区，所以效率相对于有缓冲区的归并实现函数差一些，不过它实现所借用的思想基本上和`__merge_adaptive()`如出一辙，所以不再赘述之。

```c++
template <class _BidirectionalIter, class _Distance>
void __merge_without_buffer(_BidirectionalIter __first,
                            _BidirectionalIter __middle,
                            _BidirectionalIter __last,
                            _Distance __len1, _Distance __len2) {
  if (__len1 == 0 || __len2 == 0)
    return;
  if (__len1 + __len2 == 2) {
    if (*__middle < *__first)
      iter_swap(__first, __middle);
    return;
  }
  /* 下面的实现类似于__merge_adative中针对缓冲区不足时的处理方式 */
  _BidirectionalIter __first_cut = __first;
  _BidirectionalIter __second_cut = __middle;
  _Distance __len11 = 0;
  _Distance __len22 = 0;
  if (__len1 > __len2) {
    __len11 = __len1 / 2;
    advance(__first_cut, __len11);
    __second_cut = lower_bound(__middle, __last, *__first_cut);
    distance(__middle, __second_cut, __len22);
  }
  else {
    __len22 = __len2 / 2;
    advance(__second_cut, __len22);
    __first_cut = upper_bound(__first, __middle, *__second_cut);
    distance(__first, __first_cut, __len11);
  }
  _BidirectionalIter __new_middle
    = rotate(__first_cut, __middle, __second_cut);
  __merge_without_buffer(__first, __first_cut, __new_middle,
                         __len11, __len22);
  __merge_without_buffer(__new_middle, __second_cut, __last, __len1 - __len11,
                         __len2 - __len22);
}

template <class _BidirectionalIter1, class _BidirectionalIter2,
          class _Distance>
_BidirectionalIter1 __rotate_adaptive(_BidirectionalIter1 __first,
                                      _BidirectionalIter1 __middle,
                                      _BidirectionalIter1 __last,
                                      _Distance __len1, _Distance __len2,
                                      _BidirectionalIter2 __buffer,
                                      _Distance __buffer_size) {
  _BidirectionalIter2 __buffer_end;
  if (__len1 > __len2 && __len2 <= __buffer_size) {
    __buffer_end = copy(__middle, __last, __buffer);
    copy_backward(__first, __middle, __last);
    return copy(__buffer, __buffer_end, __first);
  }
  else if (__len1 <= __buffer_size) {
    __buffer_end = copy(__first, __middle, __buffer);
    copy(__middle, __last, __first);
    return copy_backward(__buffer, __buffer_end, __last);
  }
  else
    return rotate(__first, __middle, __last);
}

template <class _BidirectionalIter1, class _BidirectionalIter2,
          class _BidirectionalIter3>
_BidirectionalIter3 __merge_backward(_BidirectionalIter1 __first1,
                                     _BidirectionalIter1 __last1,
                                     _BidirectionalIter2 __first2,
                                     _BidirectionalIter2 __last2,
                                     _BidirectionalIter3 __result) {
  if (__first1 == __last1)
    return copy_backward(__first2, __last2, __result);
  if (__first2 == __last2)
    return copy_backward(__first1, __last1, __result);
  --__last1;
  --__last2;
  while (true) {
    if (*__last2 < *__last1) {
      *--__result = *__last1;
      if (__first1 == __last1)
        return copy_backward(__first2, ++__last2, __result);
      --__last1;
    }
    else {
      *--__result = *__last2;
      if (__first2 == __last2)
        return copy_backward(__first1, ++__last1, __result);
      --__last2;
    }
  }
}

template <class _BidirectionalIter, class _Distance, class _Pointer>
void __merge_adaptive(_BidirectionalIter __first,
                      _BidirectionalIter __middle, 
                      _BidirectionalIter __last,
                      _Distance __len1, _Distance __len2,
                      _Pointer __buffer, _Distance __buffer_size) {
  //临时缓冲区足够存放前半部分序列
  if (__len1 <= __len2 && __len1 <= __buffer_size) {
    _Pointer __buffer_end = copy(__first, __middle, __buffer);
    merge(__buffer, __buffer_end, __middle, __last, __first);
  }
  //临时缓冲区足够存放后半部分序列
  else if (__len2 <= __buffer_size) {
    _Pointer __buffer_end = copy(__middle, __last, __buffer);
    __merge_backward(__first, __middle, __buffer, __buffer_end, __last);
  }
  //临时缓冲区空间不足，且空间同时小于len1和len2
  else {
    _BidirectionalIter __first_cut = __first;
    _BidirectionalIter __second_cut = __middle;
    _Distance __len11 = 0;
    _Distance __len22 = 0;
    if (__len1 > __len2) {
      __len11 = __len1 / 2;
      advance(__first_cut, __len11);
      __second_cut = lower_bound(__middle, __last, *__first_cut);
      distance(__middle, __second_cut, __len22); 
    }
    else {
      __len22 = __len2 / 2;
      advance(__second_cut, __len22);
      __first_cut = upper_bound(__first, __middle, *__second_cut);
      distance(__first, __first_cut, __len11);
    }
    /* 将[first_cut,second_cut)的元素以middle为中心进行旋转，
      旋转后[first_cut,second_cut)必然有序 */
    _BidirectionalIter __new_middle =
      __rotate_adaptive(__first_cut, __middle, __second_cut, __len1 - __len11,
                        __len22, __buffer, __buffer_size);
    //以递归调用方式处理左半段
    __merge_adaptive(__first, __first_cut, __new_middle, __len11,
                     __len22, __buffer, __buffer_size);
    //以递归调用方式处理右半段
    __merge_adaptive(__new_middle, __second_cut, __last, __len1 - __len11,
                     __len2 - __len22, __buffer, __buffer_size);
  }
}

template <class _BidirectionalIter1, class _BidirectionalIter2,
          class _BidirectionalIter3, class _Compare>
_BidirectionalIter3 __merge_backward(_BidirectionalIter1 __first1,
                                     _BidirectionalIter1 __last1,
                                     _BidirectionalIter2 __first2,
                                     _BidirectionalIter2 __last2,
                                     _BidirectionalIter3 __result,
                                     _Compare __comp) {
  if (__first1 == __last1)
    return copy_backward(__first2, __last2, __result);
  if (__first2 == __last2)
    return copy_backward(__first1, __last1, __result);
  --__last1;
  --__last2;
  while (true) {
    if (__comp(*__last2, *__last1)) {
      *--__result = *__last1;
      if (__first1 == __last1)
        return copy_backward(__first2, ++__last2, __result);
      --__last1;
    }
    else {
      *--__result = *__last2;
      if (__first2 == __last2)
        return copy_backward(__first1, ++__last1, __result);
      --__last2;
    }
  }
}

template <class _BidirectionalIter, class _Distance, class _Pointer>
void __merge_adaptive(_BidirectionalIter __first,
                      _BidirectionalIter __middle, 
                      _BidirectionalIter __last,
                      _Distance __len1, _Distance __len2,
                      _Pointer __buffer, _Distance __buffer_size) {
  //临时缓冲区足够存放前半部分序列
  if (__len1 <= __len2 && __len1 <= __buffer_size) {
    _Pointer __buffer_end = copy(__first, __middle, __buffer);
    merge(__buffer, __buffer_end, __middle, __last, __first);
  }
  //临时缓冲区足够存放后半部分序列
  else if (__len2 <= __buffer_size) {
    _Pointer __buffer_end = copy(__middle, __last, __buffer);
    __merge_backward(__first, __middle, __buffer, __buffer_end, __last);
  }
  //临时缓冲区空间不足，且空间同时小于len1和len2
  else {
    _BidirectionalIter __first_cut = __first;
    _BidirectionalIter __second_cut = __middle;
    _Distance __len11 = 0;
    _Distance __len22 = 0;
    if (__len1 > __len2) {
      __len11 = __len1 / 2;
      advance(__first_cut, __len11);
      __second_cut = lower_bound(__middle, __last, *__first_cut);
      distance(__middle, __second_cut, __len22); 
    }
    else {
      __len22 = __len2 / 2;
      advance(__second_cut, __len22);
      __first_cut = upper_bound(__first, __middle, *__second_cut);
      distance(__first, __first_cut, __len11);
    }
    /* 将[first_cut, second_cut)的元素以middle为中心进行旋转，这部分的元素必然有序，
      且new_middle前的元素必然都是小于new_middle后的元素！ */
    _BidirectionalIter __new_middle =
      __rotate_adaptive(__first_cut, __middle, __second_cut, __len1 - __len11,
                        __len22, __buffer, __buffer_size);
    //递归处理new_middle左侧半部分
    __merge_adaptive(__first, __first_cut, __new_middle, __len11,
                     __len22, __buffer, __buffer_size);
    //递归处理new_middle右侧半部分
    __merge_adaptive(__new_middle, __second_cut, __last, __len1 - __len11,
                     __len2 - __len22, __buffer, __buffer_size);
  }
}

template <class _BidirectionalIter, class _Tp, class _Distance>
inline void __inplace_merge_aux(_BidirectionalIter __first,
                                _BidirectionalIter __middle,
                                _BidirectionalIter __last, _Tp*, _Distance*) {
  _Distance __len1 = 0;
  distance(__first, __middle, __len1);
  _Distance __len2 = 0;
  distance(__middle, __last, __len2);

  //试图生成临时缓冲区，但有可能不成功！
  _Temporary_buffer<_BidirectionalIter, _Tp> __buf(__first, __last);
  if (__buf.begin() == 0)
    __merge_without_buffer(__first, __middle, __last, __len1, __len2);
  else
    __merge_adaptive(__first, __middle, __last, __len1, __len2,
                     __buf.begin(), _Distance(__buf.size()));
}

template <class _BidirectionalIter>
inline void inplace_merge(_BidirectionalIter __first,
                          _BidirectionalIter __middle,
                          _BidirectionalIter __last) {
  if (__first == __middle || __middle == __last)
    return;
  __inplace_merge_aux(__first, __middle, __last,
                      __VALUE_TYPE(__first), __DISTANCE_TYPE(__first));
}
```



#### 6.3.5 ==划分操作==

##### 6.3.5.1 partition

对于`partition()`风格算法的实现有两个版本，其中一个版本是适用于前向迭代器的，它所采用的方法为**快慢指针**，即快指针一旦遇到个符合要求的元素就与慢指针指向的元素交换，其中慢指针一定指向符合要求的元素，但它的前面一定都是符合要求的元素。而另一个版本是适用于随机访问迭代器的，它所采用的方法为**双指针**，一个当遇到符合要求的元素时往前进（++），一个遇到不符合要求的元素时往后进（--），都停下时就交换元素。

> 《剑指offer》面试题21就是在考察这两个知识点。

```c++
template <class _ForwardIter, class _Predicate>
_ForwardIter __partition(_ForwardIter __first,
		         _ForwardIter __last,
			 _Predicate   __pred,
			 forward_iterator_tag) {
  if (__first == __last) return __first;

  /* 跳过第一块符合pred谓词要求的元素部分 */
  while (__pred(*__first))
    if (++__first == __last) return __first;

  _ForwardIter __next = __first;

  /* 一旦next遇到符合pred谓词要求的就与first交换指向的元素
    ，直到末尾 */
  while (++__next != __last)
    if (__pred(*__next)) {
      swap(*__first, *__next);
      ++__first;
    }

  return __first;
}

template <class _BidirectionalIter, class _Predicate>
_BidirectionalIter __partition(_BidirectionalIter __first,
                               _BidirectionalIter __last,
			       _Predicate __pred,
			       bidirectional_iterator_tag) {
  /* 这种实现思想类似于《算法4》中的partition */
  while (true) {
    /* first迭代器向前进，直到遇到一个不满足pred谓词要求的元素停下 */
    while (true)
      if (__first == __last)
        return __first;
      else if (__pred(*__first))
        ++__first;
      else
        break;
    /* last迭代器向后进，直到遇到一个符合pred谓词要求的元素停下 */
    --__last;
    while (true)
      if (__first == __last)
        return __first;
      else if (!__pred(*__last))
        --__last;
      else
        break;
    //最后交换first和last指向的元素
    iter_swap(__first, __last);
    ++__first;
  }
}

template <class _ForwardIter, class _Predicate>
inline _ForwardIter partition(_ForwardIter __first,
   			      _ForwardIter __last,
			      _Predicate   __pred) {
  return __partition(__first, __last, __pred, __ITERATOR_CATEGORY(__first));
}
```



#### 6.3.6 ==二分搜索操作==

显然，施以二分搜索操作的前提是迭代器给定的范围元素必须是有序的。

##### 6.3.6.1 lower_bound

`lower_bound()`算法使用的就是典型的二分查找，但它的特点就是即使遇到一个中间元素值与给定值的元素相等也要到前半部分找，直到遇到给定值前一个元素（不等于给定值）时通过++first得到序列的首迭代器或者应该插入位置的迭代器。关键代码的处理就在于`*__middle < __val`，而后面的`upper_bound()`算法的关键在于`__val < *__middle`！

```c++
template <class _ForwardIter, class _Tp, class _Distance>
_ForwardIter __lower_bound(_ForwardIter __first, _ForwardIter __last,
                           const _Tp& __val, _Distance*) 
{
  _Distance __len = 0;
  distance(__first, __last, __len);
  _Distance __half;
  _ForwardIter __middle;

  while (__len > 0) {
    __half = __len >> 1;
    __middle = __first;
    advance(__middle, __half);
    //若中间元素值小于val，则到后半部分找
    if (*__middle < __val) {
      __first = __middle;
      ++__first;
      __len = __len - __half - 1;
    }
    //否则中间元素大于等于val就到前半部分找。它的想法是尽可能往前找
    else
      __len = __half;
  }
  return __first;
}

template <class _ForwardIter, class _Tp>
inline _ForwardIter lower_bound(_ForwardIter __first, _ForwardIter __last,
				const _Tp& __val) {
  return __lower_bound(__first, __last, __val,
                       __DISTANCE_TYPE(__first));
}
```



##### 6.3.6.2 upper_bound

```c++
template <class _ForwardIter, class _Tp, class _Distance>
_ForwardIter __upper_bound(_ForwardIter __first, _ForwardIter __last,
                           const _Tp& __val, _Distance*)
{
  _Distance __len = 0;
  distance(__first, __last, __len);
  _Distance __half;
  _ForwardIter __middle;

  while (__len > 0) {
    __half = __len >> 1;
    __middle = __first;
    advance(__middle, __half);
    if (__val < *__middle)
      __len = __half;
    else {
      __first = __middle;
      ++__first;
      __len = __len - __half - 1;
    }
  }
  return __first;
}

template <class _ForwardIter, class _Tp>
inline _ForwardIter upper_bound(_ForwardIter __first, _ForwardIter __last,
                                const _Tp& __val) {
  return __upper_bound(__first, __last, __val,
                       __DISTANCE_TYPE(__first));
}
```



##### 6.3.6.3 equal_range

`equal_range()`算法的实现主要是借助上面的`lower_bound()`算法和`upper_bound()`算法间接实现的（当二分查找找到任意一个与给定值相等的元素时才调用这两者）。

```c++
template <class _ForwardIter, class _Tp, class _Distance>
pair<_ForwardIter, _ForwardIter>
__equal_range(_ForwardIter __first, _ForwardIter __last, const _Tp& __val,
              _Distance*)
{
  _Distance __len = 0;
  distance(__first, __last, __len);
  _Distance __half;
  _ForwardIter __middle, __left, __right;

  while (__len > 0) {
    __half = __len >> 1;
    __middle = __first;
    advance(__middle, __half);
    if (*__middle < __val) {
      __first = __middle;
      ++__first;
      __len = __len - __half - 1;
    }
    else if (__val < *__middle)
      __len = __half;
    else {
      //在[first,middle)中找等于val区间的首元素迭代器
      __left = lower_bound(__first, __middle, __val);
      advance(__first, __len);
      //在[middle+1,last)中找等于val区间的尾后元素迭代器
      __right = upper_bound(++__middle, __first, __val);
      return pair<_ForwardIter, _ForwardIter>(__left, __right);
    }
  }
  return pair<_ForwardIter, _ForwardIter>(__first, __first);
}

template <class _ForwardIter, class _Tp>
inline pair<_ForwardIter, _ForwardIter>
equal_range(_ForwardIter __first, _ForwardIter __last, const _Tp& __val) {
  return __equal_range(__first, __last, __val,
                       __DISTANCE_TYPE(__first));
}
```



##### 6.3.6.4 binary_search

二分查找操作`binary_search()`查找某个给定值是否存在于迭代器指定的范围内，实际上是由`lower_bound()`算法完成的，因为`lower_bound()`算法找到的元素必然是>=给定值的，而只要我们用!<方式证明找到的元素<=给定值，那么也就说明`lower_bound()`找到的值和给定的值相同。

```c++
template <class _ForwardIter, class _Tp>
bool binary_search(_ForwardIter __first, _ForwardIter __last,
                   const _Tp& __val) {
  _ForwardIter __i = lower_bound(__first, __last, __val);
  return __i != __last && !(__val < *__i);
}
```



#### 6.3.7 排列操作

##### 6.3.7.1 next_permutation

`next_permutation()`和下面的`pre_permutation()`实际都是排列组合问题，例如求给定序列{1，2，3，4，5}的下一序列，结果为{1，2，3，5，4}。`next_permutation()`求下一个排序序列的核心思想应该就是尽可能将序列中从尾部找起的逆序子序列的前一个元素变大，例如{1，2，3，5，4}的下一个排列就是{1，2，4，5，3}，它就是在想让原先{5，4}前面的3尽可能变大。

至于具体的解释我不做赘述，因为书中侯捷老师讲的比我好，排列相关的算法也不常用，所以跳过。

```C++
template <class _BidirectionalIter>
bool next_permutation(_BidirectionalIter __first, _BidirectionalIter __last) {
  if (__first == __last)
    return false;
  _BidirectionalIter __i = __first;
  ++__i;
  if (__i == __last)
    return false;
  __i = __last;
  --__i;

  for(;;) {
    //让i、ii向后步进
    _BidirectionalIter __ii = __i;
    --__i;
    if (*__i < *__ii) {
      _BidirectionalIter __j = __last;
      while (!(*__i < *--__j))
        {}
      iter_swap(__i, __j);
      reverse(__ii, __last);
      return true;
    }
    if (__i == __first) {
      reverse(__first, __last);
      return false;
    }
  }
}
```



##### 6.3.7.2 pre_permutation

```c++
template <class _BidirectionalIter>
bool prev_permutation(_BidirectionalIter __first, _BidirectionalIter __last) {
  if (__first == __last)
    return false;
  _BidirectionalIter __i = __first;
  ++__i;
  if (__i == __last)
    return false;
  __i = __last;
  --__i;

  for(;;) {
    /* 向后步进i、ii */
    _BidirectionalIter __ii = __i;
    --__i;
    if (*__ii < *__i) {
      _BidirectionalIter __j = __last;
      while (!(*--__j < *__i))
        {}
      iter_swap(__i, __j);
      reverse(__ii, __last);
      return true;
    }
    if (__i == __first) {
      reverse(__first, __last);
      return false;
    }
  }
}
```



#### 6.3.8 ==排序操作==

##### 6.3.8.1 partial_sort

`partital_sort()`算法最常见的使用目的就是将给定序列[first,last)中的middle-first个最小元素以递增顺序排列在原始序列的[first,middle)位置内。换句话说，就是使用它来提取出序列中的最小几个元素，并将它们重新放置在最前面的位置，而剩余位置上的元素顺序不做保证。这个算法主要借助堆算法而实现，比较简单，过程如下：

1. 首先它使得[first，middle)转换成最大堆，然后开始在middle后面的元素中找寻比最大堆堆顶元素要小的元素，我们使用i来标识这个需要到其中遍历的迭代器；
2. 若迭代器在游走的过程中找到了比堆顶元素更小的元素，那么它就会调用`__pop_heap()`操作将堆顶元素放到迭代器i指向的位置中，而原先i指向的元素会被加入到最大堆中；
3. 当i遍历到last时结束循环，此时调用`sort_heap()`将前面[first，middle)最大堆转换为从小到大的排列。

> 注意这个`__pop_heap()`算法和`pop_heap()`算法是两码事，它并不会将最大堆的长度缩小，更可以认为是一个剔除堆顶元素并加入新的元素到堆顶中的算法。而它内部调用的`__adjust_heap()`算法也正说明这个算法的实现重在“调整”而非删除，如果看过相关源代码就会发现所谓的删除更应该是`__pop_heap_aux()`的任务。

```c++
template <class _RandomAccessIter, class _Tp>
void __partial_sort(_RandomAccessIter __first, _RandomAccessIter __middle,
                    _RandomAccessIter __last, _Tp*) {
  make_heap(__first, __middle);
  for (_RandomAccessIter __i = __middle; __i < __last; ++__i)
    if (*__i < *__first) 
      __pop_heap(__first, __middle, __i, _Tp(*__i),
                 __DISTANCE_TYPE(__first));
  sort_heap(__first, __middle);
}

template <class _RandomAccessIter>
inline void partial_sort(_RandomAccessIter __first,
                         _RandomAccessIter __middle,
                         _RandomAccessIter __last) {
  __partial_sort(__first, __middle, __last, __VALUE_TYPE(__first));
}
```

具体有如下实例所示：

<img src="../../image/Snipaste_2021-03-11_09-16-12.png" alt="Snipaste_2021-03-11_09-16-12" style="zoom:65%;" />



##### 6.3.8.2 sort

在SGI STL中，默认的`sort()`排序操作启用的并不是快速排序，而是[传统三取样+插入排序辅助的快速排序](https://github.com/Ye-zixiao/My-Algorithms/blob/master/Algs4/chaps/2.Sort.md)的一种改进版本，名为*IntroSort*。之所以这样做，主要的考量还在于期望尽可能减少执行快速排序划分partition操作过程中的递归层次，虽然三取样+设置阈值转插入排序的快速排序方法足够有效，但是阈值的选择带来执行效率的上升本身就是就是视不同的机器而不同的，固定的人为选定的阈值很难保证足够好的性能。因此开发人员选择了更为优秀的内省式排序*IntroSort*。

<img src="../../image/内省式排序.png" alt="内省式排序" style="zoom:67%;" />

由于IntroSort仅仅是对传统三取样+插入排序辅助的快速排序的改进而已，所以算法的大部分原理并不需要重新解释。它最大的不同就是引入了一个名为`__lg()`函数，如其名可以知道它其实就是在计算$log_2{数据规模}$，其结果用来限制划分操作`__unguarded_partition()`的递归层数。如果划分的层数超过了这一上限就会转而调用堆排序`partial_sort()`来完成剩下元素的排序，前提是这个子序列的长度大于一个固定阈值`__stl_threshold`。如果子序列长度小于这个阈值，那么就跳出`__introsort_loop()`，把剩下整体几近有序的序列都交给最后的插入排序完成剩下的排序作业。这样说来，IntroSort其实就是快速排序+堆排序+插入排序的整合（终极无敌粘合怪😋）！执行过程如下图示：

<img src="../../image/IntroSort.png" alt="IntroSort" style="zoom: 67%;" />

```c++
/* 在没有边界防护的情况下进行一次插入操作 */
template <class _RandomAccessIter, class _Tp>
void __unguarded_linear_insert(_RandomAccessIter __last, _Tp __val) {
  _RandomAccessIter __next = __last;
  --__next;
  while (__val < *__next) {
    *__last = *__next;
    __last = __next;
    --__next;
  }
  *__last = __val;
}

template <class _RandomAccessIter, class _Tp>
inline void __linear_insert(_RandomAccessIter __first, 
                            _RandomAccessIter __last, _Tp*) {
  _Tp __val = *__last;
  /* 若last尾元素小于前面已有序数据的最小值，那么直接调用copy_backward()
    会比调用__unguarded_linear_insert()更为高效；否则调用后者从尾到前逐一
    比较再插入 */
  if (__val < *__first) {
    copy_backward(__first, __last, __last + 1);
    *__first = __val;
  }
  else
    __unguarded_linear_insert(__last, __val);
}

//插入排序
template <class _RandomAccessIter>
void __insertion_sort(_RandomAccessIter __first, _RandomAccessIter __last) {
  if (__first == __last) return; 
  for (_RandomAccessIter __i = __first + 1; __i != __last; ++__i)
    __linear_insert(__first, __i, __VALUE_TYPE(__first));
}

template <class _RandomAccessIter, class _Tp>
void __unguarded_insertion_sort_aux(_RandomAccessIter __first, 
                                    _RandomAccessIter __last, _Tp*) {
  for (_RandomAccessIter __i = __first; __i != __last; ++__i)
    __unguarded_linear_insert(__i, _Tp(*__i));
}

template <class _RandomAccessIter>
inline void __unguarded_insertion_sort(_RandomAccessIter __first, 
                                _RandomAccessIter __last) {
  __unguarded_insertion_sort_aux(__first, __last, __VALUE_TYPE(__first));
}


template <class _RandomAccessIter>
void __final_insertion_sort(_RandomAccessIter __first, 
                            _RandomAccessIter __last) {
  if (__last - __first > __stl_threshold) {
    //先将前16个元素直接使用插入排序
    __insertion_sort(__first, __first + __stl_threshold);
    //然后逐个对剩下的元素以插入的方式插入到前面合适的地方，从而完成整体的插入排序
    __unguarded_insertion_sort(__first + __stl_threshold, __last);
  }
  else
    __insertion_sort(__first, __last);
}

template <class _RandomAccessIter, class _Tp, class _Size>
void __introsort_loop(_RandomAccessIter __first,
                      _RandomAccessIter __last, _Tp*,
                      _Size __depth_limit)
{
  while (__last - __first > __stl_threshold) {  //__stl_threshold==16
    if (__depth_limit == 0) {
      //当分割次数超限后它就会自动调用堆排序来完成子序列的排序任务
      partial_sort(__first, __last, __last);
      return;
    }
    --__depth_limit;

    //使用三取样切分方式
    _RandomAccessIter __cut =
      __unguarded_partition(__first, __last,
                            _Tp(__median(*__first,
                                         *(__first + (__last - __first)/2),
                                         *(__last - 1))));
    //对于右半部分子序列进行递归调用，对于左半部分子序列重新进入迭代处理
    __introsort_loop(__cut, __last, (_Tp*) 0, __depth_limit);
    __last = __cut;
  }
}

template <class _RandomAccessIter>
inline void sort(_RandomAccessIter __first, _RandomAccessIter __last) {
  if (__first != __last) {
    __introsort_loop(__first, __last,
                     __VALUE_TYPE(__first),
                     __lg(__last - __first) * 2);
                     /* __lg()函数返回的值val用来告知调用的底层__introsort_loop()最多只能
                      对序列分割val次，防止出现分割恶化的现象 */
    //当由快速排序差不多将序列几近有序状态后，就调用插入排序来完成最后的收尾排序工作
    __final_insertion_sort(__first, __last);
  }
}
```



##### 6.3.8.3 nth_element

`nth_element()`的目的主要是重新排列[first，last)，使得迭代器nth所指向的元素即是序列中的第n小元素。这是 一个很有趣的算法，甚至可以稍加改变成为一个算法题，例如“快速找到序列中的第k小的数”。

它的实现原理基于`parition()`划分操作思想。因为任意一个无序序列经过划分操作之后，在枢轴位置前的子序列元素必然都是小于枢轴元素的，而在枢轴位置后的子序列元素必然都是大于或等于枢轴元素的。如果一次划分后返回的枢轴元素迭代器<=迭代器nth，那么它就说明序列中的第n小元素不在前半部分而是在后半部分，这意味着下一次划分操作需要到后半部分序列中进行，否则就是在前半部分进行。

当最后划分操作进行的差不多（使得左区间迭代器first和右区间迭代器last相差<=3）时，就直接跳出while循环使用插入排序完成这小部分的排序，这样第n小的元素必然已经在nth迭代器指向的位置中。上述的过程如下面的实例所示：

<img src="../../image/Snipaste_2021-03-11_09-51-01.png" alt="Snipaste_2021-03-11_09-51-01" style="zoom: 80%;" />

```c++
template <class _RandomAccessIter, class _Tp>
void __nth_element(_RandomAccessIter __first, _RandomAccessIter __nth,
                   _RandomAccessIter __last, _Tp*) {
  while (__last - __first > 3) {
    _RandomAccessIter __cut =
      __unguarded_partition(__first, __last,
                            _Tp(__median(*__first,
                                         *(__first + (__last - __first)/2),
                                         *(__last - 1))));
    if (__cut <= __nth)
      __first = __cut;
    else 
      __last = __cut;
  }
  __insertion_sort(__first, __last);
}

template <class _RandomAccessIter>
inline void nth_element(_RandomAccessIter __first, _RandomAccessIter __nth,
                        _RandomAccessIter __last) {
  __nth_element(__first, __nth, __last, __VALUE_TYPE(__first));
}
```



##### 6.3.8.4 stable_sort

```c++
template <class _RandomAccessIter>
void __inplace_stable_sort(_RandomAccessIter __first,
                           _RandomAccessIter __last) {
  //原地插入排序+归并排序
  if (__last - __first < 15) {
    __insertion_sort(__first, __last);
    return;
  }
  _RandomAccessIter __middle = __first + (__last - __first) / 2;
  __inplace_stable_sort(__first, __middle);
  __inplace_stable_sort(__middle, __last);
  __merge_without_buffer(__first, __middle, __last,
                         __middle - __first,
                         __last - __middle);
}

template <class _RandomAccessIter1, class _RandomAccessIter2,
          class _Distance>
void __merge_sort_loop(_RandomAccessIter1 __first,
                       _RandomAccessIter1 __last, 
                       _RandomAccessIter2 __result, _Distance __step_size) {
  _Distance __two_step = 2 * __step_size;

  while (__last - __first >= __two_step) {
    __result = merge(__first, __first + __step_size,
                     __first + __step_size, __first + __two_step,
                     __result);
    __first += __two_step;
  }

  __step_size = min(_Distance(__last - __first), __step_size);
  merge(__first, __first + __step_size, __first + __step_size, __last,
        __result);
}

const int __stl_chunk_size = 7;
        
template <class _RandomAccessIter, class _Distance>
void __chunk_insertion_sort(_RandomAccessIter __first, 
                            _RandomAccessIter __last, _Distance __chunk_size)
{
  while (__last - __first >= __chunk_size) {
    __insertion_sort(__first, __first + __chunk_size);
    __first += __chunk_size;
  }
  __insertion_sort(__first, __last);
}

template <class _RandomAccessIter, class _Pointer, class _Distance>
void __merge_sort_with_buffer(_RandomAccessIter __first, 
                              _RandomAccessIter __last,
                              _Pointer __buffer, _Distance*) {
  _Distance __len = __last - __first;
  _Pointer __buffer_last = __buffer + __len;

  _Distance __step_size = __stl_chunk_size;
  /* 在[first,last)每隔一个chunk_size大小的区间做一次插入排序，是每一个区间内部有序 */
  __chunk_insertion_sort(__first, __last, __step_size);

  /* 逐一对大小为step_size的子序列自下而上进行两两归并 */
  while (__step_size < __len) {
    __merge_sort_loop(__first, __last, __buffer, __step_size);
    __step_size *= 2;
    __merge_sort_loop(__buffer, __buffer_last, __first, __step_size);
    __step_size *= 2;
  }
}

template <class _RandomAccessIter, class _Pointer, class _Distance>
void __stable_sort_adaptive(_RandomAccessIter __first, 
                            _RandomAccessIter __last, _Pointer __buffer,
                            _Distance __buffer_size) {
  _Distance __len = (__last - __first + 1) / 2;
  _RandomAccessIter __middle = __first + __len;
  if (__len > __buffer_size) {
    /* 若临时缓冲区不足，则分割前后部分进行递归，直到分割出来的子序列足够放到缓冲区中 */
    __stable_sort_adaptive(__first, __middle, __buffer, __buffer_size);
    __stable_sort_adaptive(__middle, __last, __buffer, __buffer_size);
  }
  else {
    /* 对前（后）部分进行归并排序，这里做的归并排序是自下而上的 */
    __merge_sort_with_buffer(__first, __middle, __buffer, (_Distance*)0);
    __merge_sort_with_buffer(__middle, __last, __buffer, (_Distance*)0);
  }
  //自适应归并前后部分
  __merge_adaptive(__first, __middle, __last, _Distance(__middle - __first), 
                   _Distance(__last - __middle), __buffer, __buffer_size);
}

template <class _RandomAccessIter, class _Tp, class _Distance>
inline void __stable_sort_aux(_RandomAccessIter __first,
                              _RandomAccessIter __last, _Tp*, _Distance*) {
  _Temporary_buffer<_RandomAccessIter, _Tp> buf(__first, __last);
  if (buf.begin() == 0)
    //若空间不足，则使用原地归并排序，这里用的是自上而下的归并排序
    __inplace_stable_sort(__first, __last);
  else 
    //否则使用带缓存的归并排序，这里用的是自下而上的归并排序
    __stable_sort_adaptive(__first, __last, buf.begin(),
                           _Distance(buf.size()));
}

template <class _RandomAccessIter>
inline void stable_sort(_RandomAccessIter __first,
                        _RandomAccessIter __last) {
  __stable_sort_aux(__first, __last,
                    __VALUE_TYPE(__first),
                    __DISTANCE_TYPE(__first));
}
```

