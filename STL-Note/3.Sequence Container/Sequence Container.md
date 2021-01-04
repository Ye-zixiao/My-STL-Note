## 4. 顺序容器

### 4.1 vector

vector的实现位于源文件[stl_vector.h](stl_vector.h)，其中比较需要关注的成员函数实现如下所示：

1. **构造函数（尤其是范围构造）**
2. **插入`insert()`操作，观察如何实现元素的移动和空间的动态增长**
3. **删除`erase()`、`pop_back()`等操作**，值得注意的是vector并没有空间动态减少的机制
4. 其他操作，看看就好

> 至于vector到底支持哪些操作，可以访问cppreference网站了解：https://zh.cppreference.com/w/cpp/container/vector。



#### 4.1.1 vector的数据结构

<img src="../../image/vector.jpg" alt="vector" style="zoom: 67%;" />

```c++
template <class _Tp, class _Alloc> 
class _Vector_base {
public:
  typedef _Alloc allocator_type;
  allocator_type get_allocator() const { return allocator_type(); }

  _Vector_base(const _Alloc&)
    : _M_start(0), _M_finish(0), _M_end_of_storage(0) {}
  /* 仅仅分配内存空间，并填充也不初始化 */
  _Vector_base(size_t __n, const _Alloc&)
    : _M_start(0), _M_finish(0), _M_end_of_storage(0) 
  {
    _M_start = _M_allocate(__n);
    _M_finish = _M_start;
    _M_end_of_storage = _M_start + __n;
  }

  ~_Vector_base() { _M_deallocate(_M_start, _M_end_of_storage - _M_start); }

protected:
  _Tp* _M_start;
  _Tp* _M_finish;
  _Tp* _M_end_of_storage;

  typedef simple_alloc<_Tp, _Alloc> _M_data_allocator;
  _Tp* _M_allocate(size_t __n)
    { return _M_data_allocator::allocate(__n); }
  void _M_deallocate(_Tp* __p, size_t __n) 
    { _M_data_allocator::deallocate(__p, __n); }
};
```

vector会继承这个_Vector_base基类，设计这个基类可能是为了出于简化设计的考虑，但我并不是知道还有谁还会继承这个基类。而且在当前g++中vector的实现中，\_Vector_base所起的作用变得更加重要，而这里仅仅是SGI STL V3.3版本。

```c++
//__STL_DEFAULT_ALLOCATOR(_Tp)就是alloc<_Tp>
template <class _Tp, class _Alloc = __STL_DEFAULT_ALLOCATOR(_Tp) >
class vector : protected _Vector_base<_Tp, _Alloc> 
{
  // requirements:

  __STL_CLASS_REQUIRES(_Tp, _Assignable);

private:
  typedef _Vector_base<_Tp, _Alloc> _Base;
public:
  typedef _Tp value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type* iterator;
  typedef const value_type* const_iterator;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  typedef typename _Base::allocator_type allocator_type;
  allocator_type get_allocator() const { return _Base::get_allocator(); }
    
    /*  ...  */
    
};
```



#### 4.1.2 vector的构造过程



#### 4.1.3 元素插入与空间的动态增长

##### 4.1.3.1 单元素插入

对于单元素插入操作`insert()`和`push_back()`实际上在SGI STL V3.3版本中都是通过一个名为`_M_insert_aux()`的辅助函数来间接实现。例如在`push_back()`的实现中，若备用空间足够，则会直接在尾后指针_M_finish上执行元素拷贝构造操作；若备用空间不够了，则会调用`_M_insert_aux()`辅助函数实现空间的动态增长，然后再插入。因此我们也可以看到：单元素的插入，vector所用内存空间的增长都是由`_M_insert_aux()`辅助函数在负责的。这段代码大致在源文件的674行：

```c++
protected:
  void _M_insert_aux(iterator __position, const _Tp& __x);
  void _M_insert_aux(iterator __position);

/*  ...  */

template <class _Tp, class _Alloc>
void 
vector<_Tp, _Alloc>::_M_insert_aux(iterator __position, const _Tp& __x)
{
  /* 若备用空间足够，则在finish位置上创建一个新元素，然后将position到原来
      finish之间范围的元素向右移动一位，最后将数据__x插入到position位置 */
  if (_M_finish != _M_end_of_storage) {
    construct(_M_finish, *(_M_finish - 1));
    ++_M_finish;
    _Tp __x_copy = __x;
    copy_backward(__position, _M_finish - 2, _M_finish - 1);
    *__position = __x_copy;
  }
  else {
    const size_type __old_size = size();
    const size_type __len = __old_size != 0 ? 2 * __old_size : 1;
    //1、分配新的空间
    iterator __new_start = _M_allocate(__len);
    iterator __new_finish = __new_start;
    //2、拷贝原空间数据到新空间
    __STL_TRY {
      __new_finish = uninitialized_copy(_M_start, __position, __new_start);
      construct(__new_finish, __x);
      ++__new_finish;
      __new_finish = uninitialized_copy(__position, _M_finish, __new_finish);
    }
    __STL_UNWIND((destroy(__new_start,__new_finish), 
                  _M_deallocate(__new_start,__len)));
    //3、析构+销毁原空间
    destroy(begin(), end());
    _M_deallocate(_M_start, _M_end_of_storage - _M_start);
    //4、更新指针数据
    _M_start = __new_start;
    _M_finish = __new_finish;
    _M_end_of_storage = __new_start + __len;
  }
}
```



##### 4.1.3.2 范围元素插入

vector的范围元素插入`insert()`在V3.3版本中是通过一个名叫`_M_fill_insert()`的辅助成员函数实现的，它的声明大概在源文件的409行，函数实现在704行：

```c++
template <class _Tp, class _Alloc>
void vector<_Tp, _Alloc>::_M_fill_insert(iterator __position, size_type __n, 
                                         const _Tp& __x)
{
  if (__n != 0) {
    /* 备用空间足够 */
    if (size_type(_M_end_of_storage - _M_finish) >= __n) {
      _Tp __x_copy = __x;
      const size_type __elems_after = _M_finish - __position;
      iterator __old_finish = _M_finish;
      /* 由于uninitialized_copy()算法是基于元素的拷贝构造函数而实现的，则
        算法调用时被拷贝的元素必然是未被初始化的；而copy()等算法是基于拷贝
        赋值运算符而实现的，因此它必然要求被拷贝的元素在算法执行前已经被初始
        化了。因而造成了如下的if-else */
      if (__elems_after > __n) {
        uninitialized_copy(_M_finish - __n, _M_finish, _M_finish);
        _M_finish += __n;
        copy_backward(__position, __old_finish - __n, __old_finish);
        fill(__position, __position + __n, __x_copy);
      }
      else {
        uninitialized_fill_n(_M_finish, __n - __elems_after, __x_copy);
        _M_finish += __n - __elems_after;
        uninitialized_copy(__position, __old_finish, _M_finish);
        _M_finish += __elems_after;
        fill(__position, __old_finish, __x_copy);
      }
    }
    /* 备用空间不够 */
    else {
      const size_type __old_size = size();        
      const size_type __len = __old_size + max(__old_size, __n);
      iterator __new_start = _M_allocate(__len);
      iterator __new_finish = __new_start;
      __STL_TRY {
        __new_finish = uninitialized_copy(_M_start, __position, __new_start);
        __new_finish = uninitialized_fill_n(__new_finish, __n, __x);
        __new_finish
          = uninitialized_copy(__position, _M_finish, __new_finish);
      }
      __STL_UNWIND((destroy(__new_start,__new_finish), 
                    _M_deallocate(__new_start,__len)));
      destroy(_M_start, _M_finish);
      _M_deallocate(_M_start, _M_end_of_storage - _M_start);
      _M_start = __new_start;
      _M_finish = __new_finish;
      _M_end_of_storage = __new_start + __len;
    }
  }
  //若__n==0则什么也不干
}
```



#### 4.1.4 元素删除操作

元素删除操作中比较值得学习的就是`erase()`操作，它主要就两个步骤：元素的移动和尾后元素的析构。而其他元素删除操作看看就好。下面就是`erase()`的实现，这段代码大约在源文件的第415行。

````c++
iterator erase(iterator __position) {
    if (__position + 1 != end())
        copy(__position + 1, _M_finish, __position);
    --_M_finish;
    destroy(_M_finish);
    return __position;
}
iterator erase(iterator __first, iterator __last) {
    iterator __i = copy(__last, _M_finish, __first);
    destroy(__i, _M_finish);
    _M_finish = _M_finish - (__last - __first);
    return __first;
}
````



### 4.2 list



#### 4.2.1 list的数据结构



#### 4.2.2 list的构造过程



#### 4.2.3 元素的插入与删除

##### 4.2.3.1 元素插入操作

所有链表list上添加元素的操作，无论是`push_back()`、`push_front()`都是借由`insert()`单元素任意插入操作实现的，甚至范围元素插入操作也都是由这个单元素操作通过逐一插入实现而来的。

```c++
iterator insert(iterator __position, const _Tp& __x) {
    _Node* __tmp = _M_create_node(__x);
    __tmp->_M_next = __position._M_node;
    __tmp->_M_prev = __position._M_node->_M_prev;
    __position._M_node->_M_prev->_M_next = __tmp;
    __position._M_node->_M_prev = __tmp;
    return __tmp;
}

template <class _Tp, class _Alloc>
void 
list<_Tp, _Alloc>::insert(iterator __position, 
                          const _Tp* __first, const _Tp* __last)
{
    for ( ; __first != __last; ++__first)
        insert(__position, *__first);
}
```



##### 4.2.3.2 元素删除操作

而与上面的情况相同，所有链表list上的删除元素操作，无论是`pop_back()`、`pop_front()`都是借由`erase()`任意单元素插入操作实现而来的，甚至范围元素删除的操作也是由这个单元素删除操作逐次调用而来的。

```c++
iterator erase(iterator __position) {
    _List_node_base* __next_node = __position._M_node->_M_next;
    _List_node_base* __prev_node = __position._M_node->_M_prev;
    _Node* __n = (_Node*) __position._M_node;
    __prev_node->_M_next = __next_node;
    __next_node->_M_prev = __prev_node;
    _Destroy(&__n->_M_data);
    _M_put_node(__n);
    return iterator((_Node*) __next_node);
}
```



#### 4.2.4 元素迁移与衍生操作

##### 4.2.4.1 元素迁移操作

```c++
protected:
  void transfer(iterator __position, iterator __first, iterator __last) {
    if (__position != __last) {
      // Remove [first, last) from its old position.
      __last._M_node->_M_prev->_M_next     = __position._M_node;
      __first._M_node->_M_prev->_M_next    = __last._M_node;
      __position._M_node->_M_prev->_M_next = __first._M_node; 

      // Splice [first, last) into its new position.
      _List_node_base* __tmp      = __position._M_node->_M_prev;
      __position._M_node->_M_prev = __last._M_node->_M_prev;
      __last._M_node->_M_prev     = __first._M_node->_M_prev; 
      __first._M_node->_M_prev    = __tmp;
    }
  }

public:
  void splice(iterator __position, list& __x) {
    if (!__x.empty()) 
      this->transfer(__position, __x.begin(), __x.end());
  }
  void splice(iterator __position, list&, iterator __i) {
    iterator __j = __i;
    ++__j;
    if (__position == __i || __position == __j) return;
    this->transfer(__position, __i, __j);
  }
  void splice(iterator __position, list&, iterator __first, iterator __last) {
    if (__first != __last) 
      this->transfer(__position, __first, __last);
  }
```



##### 4.2.4.2 元素迁移的衍生操作

```c++
void list<_Tp, _Alloc>::merge(list<_Tp, _Alloc>& __x)
{
  iterator __first1 = begin();
  iterator __last1 = end();
  iterator __first2 = __x.begin();
  iterator __last2 = __x.end();
  while (__first1 != __last1 && __first2 != __last2)
    if (*__first2 < *__first1) {
      iterator __next = __first2;
      transfer(__first1, __first2, ++__next);
      __first2 = __next;
    }
    else
      ++__first1;
  if (__first2 != __last2) transfer(__last1, __first2, __last2);
}

//reverse的实现并没有使用transfer操作，因为这样相比于swap来的低效
inline void __List_base_reverse(_List_node_base* __p)
{
  _List_node_base* __tmp = __p;
  do {
    __STD::swap(__tmp->_M_next, __tmp->_M_prev);
    __tmp = __tmp->_M_prev;     // Old next node is now prev.
  } while (__tmp != __p);
}

template <class _Tp, class _Alloc>
inline void list<_Tp, _Alloc>::reverse() 
{
  __List_base_reverse(this->_M_node);
}    

/* 这里估计用了归并排序，而并非书上所述的快速排序 */
template <class _Tp, class _Alloc>
void list<_Tp, _Alloc>::sort()
{
  // Do nothing if the list has length 0 or 1.
  if (_M_node->_M_next != _M_node && _M_node->_M_next->_M_next != _M_node) {
    list<_Tp, _Alloc> __carry;
    list<_Tp, _Alloc> __counter[64];
    int __fill = 0;
    while (!empty()) {
      __carry.splice(__carry.begin(), *this, begin());
      int __i = 0;
      while(__i < __fill && !__counter[__i].empty()) {
        __counter[__i].merge(__carry);
        __carry.swap(__counter[__i++]);
      }
      __carry.swap(__counter[__i]);         
      if (__i == __fill) ++__fill;
    } 

    for (int __i = 1; __i < __fill; ++__i)
      __counter[__i].merge(__counter[__i-1]);
    swap(__counter[__fill-1]);
  }
}
```