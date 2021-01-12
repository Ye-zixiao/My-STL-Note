### 4.4 容器适配器

#### 4.4.1 stack

stack的本质就是借别的顺序容器（默认情况下是deque）造个壳罢了。实现源文件[stl_stack.h](stl_stack.h)。



#### 4.4.2 queue

queue先进后出队列的实现也非常简单，就是借一个顺序容器（默认是deque，但不支持vector，因为它本身就不支持前向出入的操作）再造一个壳。实现源文件[stl_queue.h](stl_queue.h)。



#### 4.4.3 二叉堆heap

在STL中，二叉堆并不是以容器的形式存在，而是属于算法的范畴，它是后面优先队列实现的前提。

`push_heap()`从元素内容的角度来看，它实际上并没有push添加新的元素，新的元素是之前用户自己调用`push_back()`添加的；`pop_heap()`同样的，也其实并没有pop删除容器中的元素，只不过将最大值放到了最后的位置罢了。

##### 4.4.3.1 二叉堆的插入

`push_heap()`的实现（不考虑可以自定义比较器的版本）：

```c++
//执行真正的元素“插入”工作，本质就是算法4讲的上浮swim操作
template <class _RandomAccessIterator, class _Distance, class _Tp>
void 
__push_heap(_RandomAccessIterator __first,
            _Distance __holeIndex, _Distance __topIndex, _Tp __value)
{
  _Distance __parent = (__holeIndex - 1) / 2;
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
    //使用value_type()、distance_type()方便辅助函数获知迭代器相关类型并以此模板参数推断
}
```



##### 4.4.3.2 二叉堆的删除

我们知道算法作用于迭代器之上，而增删元素可能会造成迭代器的失效，所以算法绝对不会对自己所作用的容器执行增删操作，故这里的`pop_heap()`算法并不会真的删除最大值元素，而是将它放到指定范围容器的最后一个位置。`pop_heap()`算法的实现如下：

```c++
//取走最大元素之后的调整（本质就是算法4讲的下沉sink()操作）
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
  //若退出循环后只有左孩子，那么就让空洞元素和左孩子交换
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
  //且最后一个元素也已经被记录，就是下面的__value
  *__result = *__first;
  __adjust_heap(__first, _Distance(0), _Distance(__last - __first), __value);
                      //这里传入的长度不是last-first+1！，所以二叉堆的长度被缩短了
}

template <class _RandomAccessIterator, class _Tp>
inline void 
__pop_heap_aux(_RandomAccessIterator __first, _RandomAccessIterator __last,
               _Tp*)
{
  __pop_heap(__first, __last - 1, __last - 1, 
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
```

可以发现`push_heap()`和`pop_heap()`算法都不是基于交换来完成的，所以有一点差别。



##### 4.4.3.3 堆排序

`pop_heap()`的另一个好处就是我们可以利用它将最大元素放置到容器（指定范围内）的最后一个位置的特点，一边pop一边缩小pop的堆大小以实现堆排序。

```c++
template <class _RandomAccessIterator>
void sort_heap(_RandomAccessIterator __first, _RandomAccessIterator __last)
{
  __STL_REQUIRES(_RandomAccessIterator, _Mutable_RandomAccessIterator);
  __STL_REQUIRES(typename iterator_traits<_RandomAccessIterator>::value_type,
                 _LessThanComparable);
  while (__last - __first > 1)
    pop_heap(__first, __last--);
}
```



##### 4.4.3.4 建堆操作

对一个支持随机访问迭代器的容器执行建堆操作的本质就是从最后一个非叶子节点到根结点挨个执行下沉操作`sink()`，只不过在SGI STL中叫`__adjust_heap()`

```c++
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

```



#### 4.4.4 priority_queue

默认情况下，priority_queue = vector + heap算法，其实也是套壳实现的，套的有技术含量罢了。

````c++
template <class _Tp, 
          class _Sequence __STL_DEPENDENT_DEFAULT_TMPL(vector<_Tp>),
          class _Compare
          __STL_DEPENDENT_DEFAULT_TMPL(less<typename _Sequence::value_type>) >
class priority_queue {

  // requirements:

  __STL_CLASS_REQUIRES(_Tp, _Assignable);
  __STL_CLASS_REQUIRES(_Sequence, _Sequence);
  __STL_CLASS_REQUIRES(_Sequence, _RandomAccessContainer);
  typedef typename _Sequence::value_type _Sequence_value_type;
  __STL_CLASS_REQUIRES_SAME_TYPE(_Tp, _Sequence_value_type);
  __STL_CLASS_BINARY_FUNCTION_CHECK(_Compare, bool, _Tp, _Tp);

public:
  typedef typename _Sequence::value_type      value_type;
  typedef typename _Sequence::size_type       size_type;
  typedef          _Sequence                  container_type;

  typedef typename _Sequence::reference       reference;
  typedef typename _Sequence::const_reference const_reference;
protected:
  _Sequence c;//内部容器
  _Compare comp;//比较器
public:
  priority_queue() : c() {}
  explicit priority_queue(const _Compare& __x) :  c(), comp(__x) {}
    
  template <class _InputIterator>
  priority_queue(_InputIterator __first, _InputIterator __last) 
    : c(__first, __last) { make_heap(c.begin(), c.end(), comp); }

  template <class _InputIterator>
  priority_queue(_InputIterator __first, 
                 _InputIterator __last, const _Compare& __x)
    : c(__first, __last), comp(__x) 
    { make_heap(c.begin(), c.end(), comp); }

  template <class _InputIterator>
  priority_queue(_InputIterator __first, _InputIterator __last,
                 const _Compare& __x, const _Sequence& __s)
  : c(__s), comp(__x)
  { 
    c.insert(c.end(), __first, __last);
    make_heap(c.begin(), c.end(), comp);
  }
    
  bool empty() const { return c.empty(); }
  size_type size() const { return c.size(); }
  const_reference top() const { return c.front(); }
  void push(const value_type& __x) {
    __STL_TRY {
      c.push_back(__x); 
      push_heap(c.begin(), c.end(), comp);
    }
    __STL_UNWIND(c.clear());
  }
  void pop() {
    __STL_TRY {
      pop_heap(c.begin(), c.end(), comp);
      c.pop_back();
    }
    __STL_UNWIND(c.clear());
  }
};
````



### 4.5 slist(forward_list的前辈)



#### 4.5.1 slist的结点和迭代器



```c++
struct _Slist_node_base
{
  _Slist_node_base* _M_next;
};

template <class _Tp>
struct _Slist_node : public _Slist_node_base
{
  _Tp _M_data;
};
```



```c++
struct _Slist_iterator_base
{
  typedef size_t               size_type;
  typedef ptrdiff_t            difference_type;
  typedef forward_iterator_tag iterator_category;

  _Slist_node_base* _M_node;

  _Slist_iterator_base(_Slist_node_base* __x) : _M_node(__x) {}
  void _M_incr() { _M_node = _M_node->_M_next; }

  bool operator==(const _Slist_iterator_base& __x) const {
    return _M_node == __x._M_node;
  }
  bool operator!=(const _Slist_iterator_base& __x) const {
    return _M_node != __x._M_node;
  }
};

template <class _Tp, class _Ref, class _Ptr>
struct _Slist_iterator : public _Slist_iterator_base
{
  typedef _Slist_iterator<_Tp, _Tp&, _Tp*>             iterator;
  typedef _Slist_iterator<_Tp, const _Tp&, const _Tp*> const_iterator;
  typedef _Slist_iterator<_Tp, _Ref, _Ptr>             _Self;

  typedef _Tp              value_type;
  typedef _Ptr             pointer;
  typedef _Ref             reference;
  typedef _Slist_node<_Tp> _Node;

  _Slist_iterator(_Node* __x) : _Slist_iterator_base(__x) {}
  _Slist_iterator() : _Slist_iterator_base(0) {}
  _Slist_iterator(const iterator& __x) : _Slist_iterator_base(__x._M_node) {}

  reference operator*() const { return ((_Node*) _M_node)->_M_data; }

  _Self& operator++()
  {
    _M_incr();
    return *this;
  }
  _Self operator++(int)
  {
    _Self __tmp = *this;
    _M_incr();
    return __tmp;
  }
};
```



#### 4.5.3 slist的数据结构

```c++
template <class _Tp, class _Alloc> 
struct _Slist_base {
  typedef _Alloc allocator_type;
  allocator_type get_allocator() const { return allocator_type(); }

  _Slist_base(const allocator_type&) { _M_head._M_next = 0; }
  ~_Slist_base() { _M_erase_after(&_M_head, 0); }//将除_M_head后面的结点全部干掉

protected:
  typedef simple_alloc<_Slist_node<_Tp>, _Alloc> _Alloc_type;
  _Slist_node<_Tp>* _M_get_node() { return _Alloc_type::allocate(1); }
  void _M_put_node(_Slist_node<_Tp>* __p) { _Alloc_type::deallocate(__p, 1); }

  _Slist_node_base* _M_erase_after(_Slist_node_base* __pos);
  _Slist_node_base* _M_erase_after(_Slist_node_base*, _Slist_node_base*);

protected:
  _Slist_node_base _M_head;//做单链表前面的哨兵结点
}; 

template <class _Tp, class _Alloc = __STL_DEFAULT_ALLOCATOR(_Tp) >
class slist : private _Slist_base<_Tp,_Alloc>
{
  /* ... */
public:
  explicit slist(const allocator_type& __a = allocator_type()) : _Base(__a) {}

  slist(size_type __n, const value_type& __x,
        const allocator_type& __a =  allocator_type()) : _Base(__a)
    { _M_insert_after_fill(&this->_M_head, __n, __x); }

  explicit slist(size_type __n) : _Base(allocator_type())
    { _M_insert_after_fill(&this->_M_head, __n, value_type()); }
  /* ... */
};
```



#### 4.5.3 元素的插入和删除

```c++
//向__prev_node后插入新的结点
inline _Slist_node_base*
__slist_make_link(_Slist_node_base* __prev_node,
                  _Slist_node_base* __new_node)
{
  __new_node->_M_next = __prev_node->_M_next;
  __prev_node->_M_next = __new_node;
  return __new_node;
}
```



单元素插入：

```c++
template <class _Tp, class _Alloc = __STL_DEFAULT_ALLOCATOR(_Tp) >
class slist : private _Slist_base<_Tp,_Alloc>
{
  /* ... */
private:
  _Node* _M_insert_after(_Node_base* __pos, const value_type& __x) {
    return (_Node*) (__slist_make_link(__pos, _M_create_node(__x)));
  }

  _Node* _M_insert_after(_Node_base* __pos) {
    return (_Node*) (__slist_make_link(__pos, _M_create_node()));
  }

  void _M_insert_after_fill(_Node_base* __pos,
                            size_type __n, const value_type& __x) {
    for (size_type __i = 0; __i < __n; ++__i)
      __pos = __slist_make_link(__pos, _M_create_node(__x));
  }
  /* ... */
public:

  iterator insert_after(iterator __pos, const value_type& __x) {
    return iterator(_M_insert_after(__pos._M_node, __x));
  }

  iterator insert_after(iterator __pos) {
    return insert_after(__pos, value_type());
  }

  void insert_after(iterator __pos, size_type __n, const value_type& __x) {
    _M_insert_after_fill(__pos._M_node, __n, __x);
  }
  /* ... */
};
```



元素删除操作的实现：

```c++
template <class _Tp, class _Alloc> 
struct _Slist_base {
  /* ... */
  _Slist_node_base* _M_erase_after(_Slist_node_base* __pos)
  {
    _Slist_node<_Tp>* __next = (_Slist_node<_Tp>*) (__pos->_M_next);
    _Slist_node_base* __next_next = __next->_M_next;
    __pos->_M_next = __next_next;
    destroy(&__next->_M_data);
    _M_put_node(__next);
    return __next_next;
  }
  _Slist_node_base* _M_erase_after(_Slist_node_base*, _Slist_node_base*);

protected:
  _Slist_node_base _M_head;//做单链表前面的哨兵结点
};  

template <class _Tp, class _Alloc> 
_Slist_node_base*
_Slist_base<_Tp,_Alloc>::_M_erase_after(_Slist_node_base* __before_first,
                                        _Slist_node_base* __last_node) {
  _Slist_node<_Tp>* __cur = (_Slist_node<_Tp>*) (__before_first->_M_next);
  while (__cur != __last_node) {
    _Slist_node<_Tp>* __tmp = __cur;
    __cur = (_Slist_node<_Tp>*) __cur->_M_next;
    destroy(&__tmp->_M_data);
    _M_put_node(__tmp);
  }
  __before_first->_M_next = __last_node;
  return __last_node;
}

template <class _Tp, class _Alloc = __STL_DEFAULT_ALLOCATOR(_Tp) >
class slist : private _Slist_base<_Tp,_Alloc>
  /* ... */
public:
  iterator erase_after(iterator __pos) {
    return iterator((_Node*) this->_M_erase_after(__pos._M_node));
  }
  iterator erase_after(iterator __before_first, iterator __last) {
    return iterator((_Node*) this->_M_erase_after(__before_first._M_node, 
                                                  __last._M_node));
  } 
  /* ... */
};
```



#### 4.5.4 slist的拼接及其衍生操作

```c++
inline void __slist_splice_after(_Slist_node_base* __pos,
                                 _Slist_node_base* __before_first,
                                 _Slist_node_base* __before_last)
{
  if (__pos != __before_first && __pos != __before_last) {
    _Slist_node_base* __first = __before_first->_M_next;
    _Slist_node_base* __after = __pos->_M_next;
    //将串链从原来的链表上剔除
    __before_first->_M_next = __before_last->_M_next;
    //将串链拼接到链表结点pos位置的后面
    __pos->_M_next = __first;
    __before_last->_M_next = __after;
  }
}

public:
  // Moves the range [__before_first + 1, __before_last + 1) to *this,
  //  inserting it immediately after __pos.  This is constant time.
  void splice_after(iterator __pos, 
                    iterator __before_first, iterator __before_last)
  {
    if (__before_first != __before_last) 
      __slist_splice_after(__pos._M_node, __before_first._M_node, 
                           __before_last._M_node);
  }

  // Moves the element that follows __prev to *this, inserting it immediately
  //  after __pos.  This is constant time.
  void splice_after(iterator __pos, iterator __prev)
  {
    __slist_splice_after(__pos._M_node,
                         __prev._M_node, __prev._M_node->_M_next);
  }


  // Removes all of the elements from the list __x to *this, inserting
  // them immediately after __pos.  __x must not be *this.  Complexity:
  // linear in __x.size().
  void splice_after(iterator __pos, slist& __x)
  {
    __slist_splice_after(__pos._M_node, &__x._M_head);
  }
```



```c++
template <class _Tp, class _Alloc>
void slist<_Tp,_Alloc>::merge(slist<_Tp,_Alloc>& __x)
{
  _Node_base* __n1 = &this->_M_head;
  while (__n1->_M_next && __x._M_head._M_next) {
    if (((_Node*) __x._M_head._M_next)->_M_data < 
        ((_Node*)       __n1->_M_next)->_M_data) 
      //将__x中的第一个有效结点拼接到__n1后面
      __slist_splice_after(__n1, &__x._M_head, __x._M_head._M_next);
    __n1 = __n1->_M_next;
  }
  if (__x._M_head._M_next) {
    __n1->_M_next = __x._M_head._M_next;
    __x._M_head._M_next = 0;
  }
}
```



```c++
//实现原理和双链表的排序操作相同，都是使用归并排序
template <class _Tp, class _Alloc>
void slist<_Tp,_Alloc>::sort()
{
  if (this->_M_head._M_next && this->_M_head._M_next->_M_next) {
    slist __carry;
    slist __counter[64];
    int __fill = 0;
      
    while (!empty()) {
      __slist_splice_after(&__carry._M_head,
                           &this->_M_head, this->_M_head._M_next);
      int __i = 0;
      while (__i < __fill && !__counter[__i].empty()) {
        __counter[__i].merge(__carry);
        __carry.swap(__counter[__i]);
        ++__i;
      }
      __carry.swap(__counter[__i]);
      if (__i == __fill)
        ++__fill;
    }

    for (int __i = 1; __i < __fill; ++__i)
      __counter[__i].merge(__counter[__i-1]);
    this->swap(__counter[__fill-1]);
  }
}
```

