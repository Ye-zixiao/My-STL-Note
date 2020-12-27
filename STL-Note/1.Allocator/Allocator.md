## 2. 空间分配器

#### 2.1 SGI STL空间分配器概览

在SGI STL的实现中主要有如下几个空间分配器：

1. 标准C++空间分配器`std::allocator`，实现在[defalloc.h](defalloc.h)
2. SGI第一级空间分配器`__malloc_alloc_template`，实现在[stl_alloc.h](stl_alloc.h)
3. SGI第二级空间分配器__default_alloc_template`，实现在[stl_alloc.h](stl_alloc.h)
4. SGI一/二级空间分配器的别名`alloc`以及简单封装类`simple_alloc`，实现在[stl_alloc.h](stl_alloc.h)
5. 由`alloc`实现的C++标准空间分配器`allocator`，实现在[stl_alloc.h](stl_alloc.h)，前提是需要使用到

其中标准C++空间分配器`std::allocator`并不是默认的空间分配器，`alloc`才是真正默认空间分配器，然而`alloc`只不过是SGI第二级空间分配器`__default_alloc_template`的别名。在容器空间的分配中，则会使用到简单封装类模板`simple_alloc`，它又仅仅是`alloc`的简单封装，因此空间分配的真正操作者是`__default_alloc_template`这个分配器而不是其他，除非特别用户指定。

SGI STL对空间分配器的实现主要是出于性能、效率和其他多种因素的考虑，它引入了一个双层级的空间分配器设计，**一级空间分配器指的是`__malloc_alloc_template`，二级空间分配器指的是`__default_alloc_template`。其中前者直接调用`malloc()`、`free()`和`realloc()`等函数来分配/销毁空间；而后者对于大于128字节空间的分配直接调用`__malloc_alloc_template`，而对于小于128字节空间的分配采用内存池策略，需要用时从内存池中取出，不需要时退回给内存池，从而避免内存碎片等多个问题**。正是这种设计的各种优点使得SGI STL将`__default_alloc_template`设置为默认的空间分配器。

下面是所有空间分配器所起作用的大致结构：

<img src="../../image/alloc.jpg" alt="alloc" style="zoom: 50%;" />



为了实现紧密分工，STL allocator还将对象的空间分配/销毁以及对象的构造/析构两种操作分离开来实现。这使得`alloc`只负责对象空间的分配/销毁：`alloc::allocate()`、`alloc::deallocate()`，而对象构造和析构由进一步封装的类，比如上述由`alloc`实现的标准空间分配器`allocator`实现或者由STL算法`std::construct()`、`std::destroy()`来完成。

其中`alloc`、一/二级空间分配器实现在[stl_alloc.h](stl_alloc.h)，STL算法`std::construct()`、`std::destroy()`实现在[stl_construct.h](stl_construct.h)，除此之外，STL还具有一些在已分配但未初始化的空间上进行拷贝构造、填充的算法`unintialized_xxx()`，它们实现在[stl_uninitialized.h](stl_uninitialized.h)，然后这些源文件全部include在标准C++头文件[memory](memory)中。文件分布如下：

<img src="../../image/屏幕截图 2020-12-27 102048.png" alt="屏幕截图 2020-12-27 102048" style="zoom:80%;" />



#### 2.2 空间分配/销毁和对象构造/析构

在[stl_construct.h](stl_construct.h)中我们可以看到STL算法`construct()`就是直接通过定位new的方式实现，而`destroy()`通过`__type_traits`技术，识别出调用元素/迭代器指定范围内的元素的类型，判断出它们是否是POD类型（析构、构造函数trivial可有可无，没什么用），若是则什么也不做，否则逐个调用析构函数。

```c++
//单元素构造
template <class _T1, class _T2>
inline void _Construct(_T1* __p, const _T2& __value) {
  new ((void*) __p) _T1(__value);
}

//单元素析构
template <class _Tp>
inline void _Destroy(_Tp* __pointer) {
  __pointer->~_Tp();
}

//迭代器指定的范围元素集合中元素类型支持可用non-trivial析构函数
template <class _ForwardIterator>
void
__destroy_aux(_ForwardIterator __first, _ForwardIterator __last, __false_type)
{
  for ( ; __first != __last; ++__first)
    destroy(&*__first);
}

//迭代器指定的范围元素集合中的元素类型不支持无用trivial析构函数，则什么也不做
template <class _ForwardIterator> 
inline void __destroy_aux(_ForwardIterator, _ForwardIterator, __true_type) {}

/* 根据元素类型的析构函数是non-trivail还是trivial
	来决定调用上述__destroy_aux的某一个版本 */
template <class _ForwardIterator, class _Tp>
inline void 
__destroy(_ForwardIterator __first, _ForwardIterator __last, _Tp*)
{
  typedef typename __type_traits<_Tp>::has_trivial_destructor
          _Trivial_destructor;
  __destroy_aux(__first, __last, _Trivial_destructor());
}

template <class _ForwardIterator>
inline void _Destroy(_ForwardIterator __first, _ForwardIterator __last) {
  __destroy(__first, __last, __VALUE_TYPE(__first));
}
```

<img src="../../image/屏幕截图 2020-12-27 102652.png" alt="屏幕截图 2020-12-27 102652" style="zoom:80%;" />



#### 2.3 SGI STL第一级空间分配器

在上面我们已经指出一级空间分配器的实现是由`malloc`、`free`、`realloc`等函数完成，并不是用`::operator new`、`::operator delete`等函数完成，虽然不支持`set_new_handler()`，但引入了一个`set_malloc_handler()`以处理空间分配意外情况。

这个第一级空间分配器大约在源代码文件[stl_lloc.h](stl_alloc.h)的109行。

```c++
template <int __inst>
class __malloc_alloc_template {

private:
  //以下函数指针用于处理内存不足的情况（oom==out of memory）
  static void* _S_oom_malloc(size_t);
  static void* _S_oom_realloc(void*, size_t);

#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
  static void (* __malloc_alloc_oom_handler)();
#endif

public:

  static void* allocate(size_t __n)
  {
    void* __result = malloc(__n);
    if (0 == __result) __result = _S_oom_malloc(__n);
    return __result;
  }

  static void deallocate(void* __p, size_t /* __n */)
  {
    free(__p);
  }

  static void* reallocate(void* __p, size_t /* old_sz */, size_t __new_sz)
  {
    void* __result = realloc(__p, __new_sz);
    if (0 == __result) __result = _S_oom_realloc(__p, __new_sz);
    return __result;
  }

  //类似于set_new_handler()
  static void (* __set_malloc_handler(void (*__f)()))()
  {
    void (* __old)() = __malloc_alloc_oom_handler;
    __malloc_alloc_oom_handler = __f;
    return(__old);
  }

};

// malloc_alloc out-of-memory handling

#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
template <int __inst>
void (* __malloc_alloc_template<__inst>::__malloc_alloc_oom_handler)() = 0;
#endif

template <int __inst>
void*
__malloc_alloc_template<__inst>::_S_oom_malloc(size_t __n)
{
    void (* __my_malloc_handler)();
    void* __result;

    for (;;) {
        __my_malloc_handler = __malloc_alloc_oom_handler;
        //一般而言，malloc分配失败时的处理函数是终止进程
        if (0 == __my_malloc_handler) { __THROW_BAD_ALLOC; }
        //但若用户确实指定了一个可行的处理函数，则尝试重新分配动态内存
        (*__my_malloc_handler)();
        __result = malloc(__n);
        if (__result) return(__result);
    }
}

template <int __inst>
void* __malloc_alloc_template<__inst>::_S_oom_realloc(void* __p, size_t __n)
{
    void (* __my_malloc_handler)();
    void* __result;

    for (;;) {
        __my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == __my_malloc_handler) { __THROW_BAD_ALLOC; }
        (*__my_malloc_handler)();
        __result = realloc(__p, __n);
        if (__result) return(__result);
    }
}

typedef __malloc_alloc_template<0> malloc_alloc;
```



#### 2.4 SGI STL第二级空间分配器

<img src="../../image/屏幕截图 2020-12-27 103126.png" alt="屏幕截图 2020-12-27 103126" style="zoom:80%;" />

在上面我们也提到过，二级空间分配器针对索要不同空间采取了不同的策略，对于大于128字节的空间直接调用`__malloc_alloc_template`来完成；而对于小于128字节空间的索取使用了内存池来实现。这里内存池的本质就是一个使用指针串起来的内存块链表，每一个链表节点既是一个完整的空间也是指针（通过union来实现），空间大小为8的倍数（8、16、24...120、128），即使用户需要的不是8的倍数也会上取整，然后分配器会为每一个不同大小的内存池链表维护一个链表指针数组，分别指向不同链表的起点。

当用户需要时二级空间分配器会从其中取出一个节点删除，将这节点的空间作为自己的所需返回；当不需要时，将这个空间（重解释成链表节点）插回到内存池链表的头部。如果内存池空间不足，二级分配器还会通过`malloc`分配出更多的空间（这个空间为$2\times所需单元空间（被上取整过）$余，这一点也是有深意的）添加到链表中。

```c++
#if defined(__SUNPRO_CC) || defined(__GNUC__)
// breaks if we make these template class members:
  enum {_ALIGN = 8};
  enum {_MAX_BYTES = 128};
  enum {_NFREELISTS = 16}; // _MAX_BYTES/_ALIGN
#endif

template <bool threads, int inst>
class __default_alloc_template {

private:
  static size_t
  _S_round_up(size_t __bytes) 
    { return (((__bytes) + (size_t) _ALIGN-1) & ~((size_t) _ALIGN - 1)); }

__PRIVATE:
  //定义free-list空闲动态内存链表节点
  union _Obj {
        union _Obj* _M_free_list_link;
        char _M_client_data[1];    /* The client sees this.        */
  };
private:
# if defined(__SUNPRO_CC) || defined(__GNUC__) || defined(__HP_aCC)
    static _Obj* __STL_VOLATILE _S_free_list[]; 
        // Specifying a size results in duplicate def for 4.1
# else
    static _Obj* __STL_VOLATILE _S_free_list[_NFREELISTS]; 
# endif
  //根据所需内存大小，决定使用上述free-list数组中的哪一个元素（链表）
  static  size_t _S_freelist_index(size_t __bytes) {
        return (((__bytes) + (size_t)_ALIGN-1)/(size_t)_ALIGN - 1);
  }

  // Returns an object of size __n, and optionally adds to size __n free list.
  static void* _S_refill(size_t __n);
  // Allocates a chunk for nobjs of size size.  nobjs may be reduced
  // if it is inconvenient to allocate the requested number.
  static char* _S_chunk_alloc(size_t __size, int& __nobjs);

  // Chunk allocation state.
  static char* _S_start_free;
  static char* _S_end_free;
  static size_t _S_heap_size;

public:

  /* __n must be > 0      */
  static void* allocate(size_t __n)
  {
    void* __ret = 0;

    if (__n > (size_t) _MAX_BYTES) {
      __ret = malloc_alloc::allocate(__n);
    }
    else {
      _Obj* __STL_VOLATILE* __my_free_list
          = _S_free_list + _S_freelist_index(__n);
      // Acquire the lock here with a constructor call.
      // This ensures that it is released in exit or during stack
      // unwinding.
#     ifndef _NOTHREADS
      /*REFERENCED*/
      _Lock __lock_instance;
#     endif
      _Obj* __RESTRICT __result = *__my_free_list;
      if (__result == 0)
        __ret = _S_refill(_S_round_up(__n));
      else {
        *__my_free_list = __result -> _M_free_list_link;
        __ret = __result;
      }
    }

    return __ret;
  };

  /* __p may not be 0 */
  static void deallocate(void* __p, size_t __n)
  {
    if (__n > (size_t) _MAX_BYTES)
      malloc_alloc::deallocate(__p, __n);
    else {
      _Obj* __STL_VOLATILE*  __my_free_list
          = _S_free_list + _S_freelist_index(__n);
      _Obj* __q = (_Obj*)__p;

      // acquire lock
#       ifndef _NOTHREADS
      /*REFERENCED*/
      _Lock __lock_instance;
#       endif /* _NOTHREADS */
      __q -> _M_free_list_link = *__my_free_list;
      *__my_free_list = __q;
      // lock is released here
    }
  }

  static void* reallocate(void* __p, size_t __old_sz, size_t __new_sz);

} ;

typedef __default_alloc_template<__NODE_ALLOCATOR_THREADS, 0> alloc;
typedef __default_alloc_template<false, 0> single_client_alloc;

template <bool __threads, int __inst>
inline bool operator==(const __default_alloc_template<__threads, __inst>&,
                       const __default_alloc_template<__threads, __inst>&)
{
  return true;
}

# ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER
template <bool __threads, int __inst>
inline bool operator!=(const __default_alloc_template<__threads, __inst>&,
                       const __default_alloc_template<__threads, __inst>&)
{
  return false;
}
# endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */



/* We allocate memory in large chunks in order to avoid fragmenting     */
/* the malloc heap too much.                                            */
/* We assume that size is properly aligned.                             */
/* We hold the allocation lock.                                         */
template <bool __threads, int __inst>
char*
__default_alloc_template<__threads, __inst>::_S_chunk_alloc(size_t __size, 
                                                            int& __nobjs)
{
    char* __result;
    size_t __total_bytes = __size * __nobjs;
    size_t __bytes_left = _S_end_free - _S_start_free;

    if (__bytes_left >= __total_bytes) {
        __result = _S_start_free;
        _S_start_free += __total_bytes;
        return(__result);
    } else if (__bytes_left >= __size) {
        __nobjs = (int)(__bytes_left/__size);
        __total_bytes = __size * __nobjs;
        __result = _S_start_free;
        _S_start_free += __total_bytes;
        return(__result);
    } else {
        size_t __bytes_to_get = 
	  2 * __total_bytes + _S_round_up(_S_heap_size >> 4);
        // Try to make use of the left-over piece.
        if (__bytes_left > 0) {
            _Obj* __STL_VOLATILE* __my_free_list =
                        _S_free_list + _S_freelist_index(__bytes_left);

            ((_Obj*)_S_start_free) -> _M_free_list_link = *__my_free_list;
            *__my_free_list = (_Obj*)_S_start_free;
        }
        _S_start_free = (char*)malloc(__bytes_to_get);
        if (0 == _S_start_free) {
            size_t __i;
            _Obj* __STL_VOLATILE* __my_free_list;
	    _Obj* __p;
            // Try to make do with what we have.  That can't
            // hurt.  We do not try smaller requests, since that tends
            // to result in disaster on multi-process machines.
            for (__i = __size;
                 __i <= (size_t) _MAX_BYTES;
                 __i += (size_t) _ALIGN) {
                __my_free_list = _S_free_list + _S_freelist_index(__i);
                __p = *__my_free_list;
                if (0 != __p) {
                    *__my_free_list = __p -> _M_free_list_link;
                    _S_start_free = (char*)__p;
                    _S_end_free = _S_start_free + __i;
                    return(_S_chunk_alloc(__size, __nobjs));
                    // Any leftover piece will eventually make it to the
                    // right free list.
                }
            }
	    _S_end_free = 0;	// In case of exception.
            _S_start_free = (char*)malloc_alloc::allocate(__bytes_to_get);
            // This should either throw an
            // exception or remedy the situation.  Thus we assume it
            // succeeded.
        }
        _S_heap_size += __bytes_to_get;
        _S_end_free = _S_start_free + __bytes_to_get;
        return(_S_chunk_alloc(__size, __nobjs));
    }
}


/* Returns an object of size __n, and optionally adds to size __n free list.*/
/* We assume that __n is properly aligned.                                */
/* We hold the allocation lock.                                         */
template <bool __threads, int __inst>
void*
__default_alloc_template<__threads, __inst>::_S_refill(size_t __n)
{
    int __nobjs = 20;
    char* __chunk = _S_chunk_alloc(__n, __nobjs);
    _Obj* __STL_VOLATILE* __my_free_list;
    _Obj* __result;
    _Obj* __current_obj;
    _Obj* __next_obj;
    int __i;

    if (1 == __nobjs) return(__chunk);
    __my_free_list = _S_free_list + _S_freelist_index(__n);

    /* Build free list in chunk */
      __result = (_Obj*)__chunk;
      *__my_free_list = __next_obj = (_Obj*)(__chunk + __n);
      for (__i = 1; ; __i++) {
        __current_obj = __next_obj;
        __next_obj = (_Obj*)((char*)__next_obj + __n);
        if (__nobjs - 1 == __i) {
            __current_obj -> _M_free_list_link = 0;
            break;
        } else {
            __current_obj -> _M_free_list_link = __next_obj;
        }
      }
    return(__result);
}

template <bool threads, int inst>
void*
__default_alloc_template<threads, inst>::reallocate(void* __p,
                                                    size_t __old_sz,
                                                    size_t __new_sz)
{
    void* __result;
    size_t __copy_sz;

    if (__old_sz > (size_t) _MAX_BYTES && __new_sz > (size_t) _MAX_BYTES) {
        return(realloc(__p, __new_sz));
    }
    if (_S_round_up(__old_sz) == _S_round_up(__new_sz)) return(__p);
    __result = allocate(__new_sz);
    __copy_sz = __new_sz > __old_sz? __old_sz : __new_sz;
    memcpy(__result, __p, __copy_sz);
    deallocate(__p, __old_sz);
    return(__result);
}

#ifdef __STL_THREADS
    template <bool __threads, int __inst>
    _STL_mutex_lock
    __default_alloc_template<__threads, __inst>::_S_node_allocator_lock
        __STL_MUTEX_INITIALIZER;
#endif


template <bool __threads, int __inst>
char* __default_alloc_template<__threads, __inst>::_S_start_free = 0;

template <bool __threads, int __inst>
char* __default_alloc_template<__threads, __inst>::_S_end_free = 0;

template <bool __threads, int __inst>
size_t __default_alloc_template<__threads, __inst>::_S_heap_size = 0;

template <bool __threads, int __inst>
typename __default_alloc_template<__threads, __inst>::_Obj* __STL_VOLATILE
__default_alloc_template<__threads, __inst> ::_S_free_list[
# if defined(__SUNPRO_CC) || defined(__GNUC__) || defined(__HP_aCC)
    _NFREELISTS
# else
    __default_alloc_template<__threads, __inst>::_NFREELISTS
# endif
] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
// The 16 zeros are necessary to make version 4.1 of the SunPro
// compiler happy.  Otherwise it appears to allocate too little
// space for the array.

```



#### 2.5 SGI STL一/二级分配器的封装类

这段代码大致在源代码文件`std_alloc.h`的193行

```c++
template<class _Tp, class _Alloc>
class simple_alloc {

public:
    static _Tp* allocate(size_t __n)
      { return 0 == __n ? 0 : (_Tp*) _Alloc::allocate(__n * sizeof (_Tp)); }
    static _Tp* allocate(void)
      { return (_Tp*) _Alloc::allocate(sizeof (_Tp)); }
    static void deallocate(_Tp* __p, size_t __n)
      { if (0 != __n) _Alloc::deallocate(__p, __n * sizeof (_Tp)); }
    static void deallocate(_Tp* __p)
      { _Alloc::deallocate(__p, sizeof (_Tp)); }
};
```



#### 2.6 未初始化内存拷贝/填充算法

未初始化内容拷贝函数`uninitialzed_copy()`和未初始化内存填充函数`uninitalized_fill()`和`uninitalized_fill_n()`函数实现的方法类似于上述对象析构函数`destroy()`的实现原理。

它们通过`__type_traits`技术来区分待初始化内存上欲构造的类型是POD类型还是非POD类型，其中POD类型指的是具有trivial没有屌用的构造、析构、拷贝和赋值函数的原始类型、C-结构化类型，例如int、double之类的。①若是POD类型就直接使用copy()、fill()这样的STL算法直接来完成内存数据的拷贝和填充；②若不是，这对每一个迭代器上指向的元素执行拷贝构造函数。

这些代码位于源文件`stl_uninitalized.h`之中。下面仅以`uninitalized_fill()`函数为例：

```c++
/* 针对具有trivial无用构造、拷贝等函数的POD类型直接
	调用STL的fill()函数直接进行内存数据的拷贝 */
template <class _ForwardIter, class _Tp>
inline void
__uninitialized_fill_aux(_ForwardIter __first, _ForwardIter __last, 
                         const _Tp& __x, __true_type)
{
  fill(__first, __last, __x);
}

//针对非POD类型逐个调用拷贝构造函数
template <class _ForwardIter, class _Tp>
void
__uninitialized_fill_aux(_ForwardIter __first, _ForwardIter __last, 
                         const _Tp& __x, __false_type)
{
  _ForwardIter __cur = __first;
  __STL_TRY {
    for ( ; __cur != __last; ++__cur)
      _Construct(&*__cur, __x);
  }
  __STL_UNWIND(_Destroy(__first, __cur));
}

//使用__type_traits技术分情况调用__uninitalized_fill_aux()函数
template <class _ForwardIter, class _Tp, class _Tp1>
inline void __uninitialized_fill(_ForwardIter __first, 
                                 _ForwardIter __last, const _Tp& __x, _Tp1*)
{
  typedef typename __type_traits<_Tp1>::is_POD_type _Is_POD;
  __uninitialized_fill_aux(__first, __last, __x, _Is_POD());
                   
}

template <class _ForwardIter, class _Tp>
inline void uninitialized_fill(_ForwardIter __first,
                               _ForwardIter __last, 
                               const _Tp& __x)
{
  __uninitialized_fill(__first, __last, __x, __VALUE_TYPE(__first));
}
```

