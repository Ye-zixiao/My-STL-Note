/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * Copyright (c) 1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */

/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */

#ifndef __SGI_STL_INTERNAL_HEAP_H
#define __SGI_STL_INTERNAL_HEAP_H

__STL_BEGIN_NAMESPACE

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1209
#endif

// Heap-manipulation functions: push_heap, pop_heap, make_heap, sort_heap.

template <class _RandomAccessIterator, class _Distance, class _Tp>
void 
__push_heap(_RandomAccessIterator __first,
            _Distance __holeIndex, _Distance __topIndex, _Tp __value)
{
  _Distance __parent = (__holeIndex - 1) / 2;
  //父节点、祖先节点先下沉，为插入元素预留一个空位
  while (__holeIndex > __topIndex && *(__first + __parent) < __value) {
    *(__first + __holeIndex) = *(__first + __parent);
    __holeIndex = __parent;
    __parent = (__holeIndex - 1) / 2;
  }
  *(__first + __holeIndex) = __value;
}

template <class _RandomAccessIterator, class _Distance, class _Tp>
inline void 
__push_heap_aux(_RandomAccessIterator __first,
                _RandomAccessIterator __last, _Distance*, _Tp*)
{
  __push_heap(__first, _Distance((__last - __first) - 1), _Distance(0), 
              _Tp(*(__last - 1)));
}

template <class _RandomAccessIterator>
inline void 
push_heap(_RandomAccessIterator __first, _RandomAccessIterator __last)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __STL_REQUIRES(typename iterator_traits<_RandomAccessIterator>::value_type,
                 _LessThanComparable);
  __push_heap_aux(__first, __last,
                  __DISTANCE_TYPE(__first), __VALUE_TYPE(__first));
}

template <class _RandomAccessIterator, class _Distance, class _Tp, 
          class _Compare>
void
__push_heap(_RandomAccessIterator __first, _Distance __holeIndex,
            _Distance __topIndex, _Tp __value, _Compare __comp)
{
  _Distance __parent = (__holeIndex - 1) / 2;
  while (__holeIndex > __topIndex && __comp(*(__first + __parent), __value)) {
    *(__first + __holeIndex) = *(__first + __parent);
    __holeIndex = __parent;
    __parent = (__holeIndex - 1) / 2;
  }
  *(__first + __holeIndex) = __value;
}

template <class _RandomAccessIterator, class _Compare,
          class _Distance, class _Tp>
inline void 
__push_heap_aux(_RandomAccessIterator __first,
                _RandomAccessIterator __last, _Compare __comp,
                _Distance*, _Tp*) 
{
  __push_heap(__first, _Distance((__last - __first) - 1), _Distance(0), 
              _Tp(*(__last - 1)), __comp);
}

template <class _RandomAccessIterator, class _Compare>
inline void 
push_heap(_RandomAccessIterator __first, _RandomAccessIterator __last,
          _Compare __comp)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __push_heap_aux(__first, __last, __comp,
                  __DISTANCE_TYPE(__first), __VALUE_TYPE(__first));
}

template <class _RandomAccessIterator, class _Distance, class _Tp>
void 
__adjust_heap(_RandomAccessIterator __first, _Distance __holeIndex,
              _Distance __len, _Tp __value)
              /* hostIndex指的是当前空洞元素的位置 */
{
  _Distance __topIndex = __holeIndex;
  _Distance __secondChild = 2 * __holeIndex + 2;
  while (__secondChild < __len) {
    //若左边的兄弟结点值比当前预判的孩子结点大，那么换一下下标
    if (*(__first + __secondChild) < *(__first + (__secondChild - 1)))
      __secondChild--;
    *(__first + __holeIndex) = *(__first + __secondChild);
    __holeIndex = __secondChild;
    __secondChild = 2 * (__secondChild + 1);
  }
  if (__secondChild == __len) {
    *(__first + __holeIndex) = *(__first + (__secondChild - 1));
    __holeIndex = __secondChild - 1;
  }
  __push_heap(__first, __holeIndex, __topIndex, __value);
}

template <class _RandomAccessIterator, class _Tp, class _Distance>
inline void 
__pop_heap(_RandomAccessIterator __first, _RandomAccessIterator __last,
           _RandomAccessIterator __result, _Tp __value, _Distance*)
{
  //并没有删除最大值元素，仅仅将它放到容器（指定范围内）的最后位
  *__result = *__first;
  __adjust_heap(__first, _Distance(0), _Distance(__last - __first), __value);
}

template <class _RandomAccessIterator, class _Tp>
inline void 
__pop_heap_aux(_RandomAccessIterator __first, _RandomAccessIterator __last,
               _Tp*)
{
  __pop_heap(__first, __last - 1, __last - 1, //长度被缩减了1
             _Tp(*(__last - 1)), __DISTANCE_TYPE(__first));
             //这里的__DISTANCE_TYPE仅仅是为了模板实参推断
}

template <class _RandomAccessIterator>
inline void pop_heap(_RandomAccessIterator __first, 
                     _RandomAccessIterator __last)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __STL_REQUIRES(typename iterator_traits<_RandomAccessIterator>::value_type,
                 _LessThanComparable);
  __pop_heap_aux(__first, __last, __VALUE_TYPE(__first));
}

template <class _RandomAccessIterator, class _Distance,
          class _Tp, class _Compare>
void
__adjust_heap(_RandomAccessIterator __first, _Distance __holeIndex,
              _Distance __len, _Tp __value, _Compare __comp)
{
  _Distance __topIndex = __holeIndex;
  _Distance __secondChild = 2 * __holeIndex + 2;
  while (__secondChild < __len) {
    if (__comp(*(__first + __secondChild), *(__first + (__secondChild - 1))))
      __secondChild--;
    *(__first + __holeIndex) = *(__first + __secondChild);
    __holeIndex = __secondChild;
    __secondChild = 2 * (__secondChild + 1);
  }
  if (__secondChild == __len) {
    *(__first + __holeIndex) = *(__first + (__secondChild - 1));
    __holeIndex = __secondChild - 1;
  }
  __push_heap(__first, __holeIndex, __topIndex, __value, __comp);
}

template <class _RandomAccessIterator, class _Tp, class _Compare, 
          class _Distance>
inline void 
__pop_heap(_RandomAccessIterator __first, _RandomAccessIterator __last,
           _RandomAccessIterator __result, _Tp __value, _Compare __comp,
           _Distance*)
{
  *__result = *__first;
  __adjust_heap(__first, _Distance(0), _Distance(__last - __first), 
                __value, __comp);
}

template <class _RandomAccessIterator, class _Tp, class _Compare>
inline void 
__pop_heap_aux(_RandomAccessIterator __first,
               _RandomAccessIterator __last, _Tp*, _Compare __comp)
{
  __pop_heap(__first, __last - 1, __last - 1, _Tp(*(__last - 1)), __comp,
             __DISTANCE_TYPE(__first));
}

template <class _RandomAccessIterator, class _Compare>
inline void 
pop_heap(_RandomAccessIterator __first,
         _RandomAccessIterator __last, _Compare __comp)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __pop_heap_aux(__first, __last, __VALUE_TYPE(__first), __comp);
}

template <class _RandomAccessIterator, class _Tp, class _Distance>
void 
__make_heap(_RandomAccessIterator __first,
            _RandomAccessIterator __last, _Tp*, _Distance*)
{
  if (__last - __first < 2) return;
  _Distance __len = __last - __first;
  _Distance __parent = (__len - 2)/2;
  /* 从最后一个非叶子节点开始到根节点逐一执行下沉sink操作（算法4叫这名字，
    不过这里SGI STL叫adjust_heap，无所谓🙃） */
    
  while (true) {
    __adjust_heap(__first, __parent, __len, _Tp(*(__first + __parent)));
    if (__parent == 0) return;
    __parent--;
  }
}

template <class _RandomAccessIterator>
inline void 
make_heap(_RandomAccessIterator __first, _RandomAccessIterator __last)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __STL_REQUIRES(typename iterator_traits<_RandomAccessIterator>::value_type,
                 _LessThanComparable);
  __make_heap(__first, __last,
              __VALUE_TYPE(__first), __DISTANCE_TYPE(__first));
}

template <class _RandomAccessIterator, class _Compare,
          class _Tp, class _Distance>
void
__make_heap(_RandomAccessIterator __first, _RandomAccessIterator __last,
            _Compare __comp, _Tp*, _Distance*)
{
  if (__last - __first < 2) return;
  _Distance __len = __last - __first;
  _Distance __parent = (__len - 2)/2;
    
  while (true) {
    __adjust_heap(__first, __parent, __len, _Tp(*(__first + __parent)),
                  __comp);
    if (__parent == 0) return;
    __parent--;
  }
}

template <class _RandomAccessIterator, class _Compare>
inline void 
make_heap(_RandomAccessIterator __first, 
          _RandomAccessIterator __last, _Compare __comp)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __make_heap(__first, __last, __comp,
              __VALUE_TYPE(__first), __DISTANCE_TYPE(__first));
}

template <class _RandomAccessIterator>
void sort_heap(_RandomAccessIterator __first, _RandomAccessIterator __last)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __STL_REQUIRES(typename iterator_traits<_RandomAccessIterator>::value_type,
                 _LessThanComparable);
  while (__last - __first > 1)
    pop_heap(__first, __last--);
}

template <class _RandomAccessIterator, class _Compare>
void 
sort_heap(_RandomAccessIterator __first,
          _RandomAccessIterator __last, _Compare __comp)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  while (__last - __first > 1)
    pop_heap(__first, __last--, __comp);
}

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1209
#endif

__STL_END_NAMESPACE

#endif /* __SGI_STL_INTERNAL_HEAP_H */

// Local Variables:
// mode:C++
// End:
