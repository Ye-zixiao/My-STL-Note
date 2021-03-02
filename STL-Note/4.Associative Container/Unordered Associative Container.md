### 5.4 哈希表基础

哈希表能够实现快速插入、删除和查找操作的诀窍在于：插入时会根据哈希函数直接计算操作元素的哈希值，然后将该哈希值作为数组中的下标并将其存放在对应的槽中。这样，下次需要进行查找、删除的时候仅仅需要再次计算哈希值就可以得到快速的响应，不过缺点就是存储后的元素之间并不会像红黑树那样呈现有序的状态，故哈希表是一个典型的无序容器。

哈希表还有一个重大问题在于多个操作元素所计算出来的哈希值可能是一致的，显然我们不能将它们都存放在数组相同的槽中，这便是元素的碰撞问题。解决这种问题主要有探测法和拉链法两种解决途径。



#### 5.4.1.1 探测法

一种场景的探测法就是当操作的元素的哈希值所对应的下标已经有不同的元素存在，那么就自动将下标线性递增，直到找到一个可用的位置。这种方法的缺点就是它很容易造成主集团的问题，即具有相同哈希值的元素因为线性探测导致成片连续存放，并进一步导致相近哈希值的元素也不得不跟在后面连续存放。

避免主集团的一种途径



#### 5.4.1.2 拉链法



#### 5.4.1.3 hash函数



### 5.5 hashtable



#### 5.5.1 hashtable节点和迭代器

```c++
//hash桶中的结点
template <class _Val>
struct _Hashtable_node
{
  _Hashtable_node* _M_next;
  _Val _M_val;
};  
```



```c++
template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc>
struct _Hashtable_iterator {
  /* ... */

  _Node* _M_cur;//指向当前的hash桶中的结点
  _Hashtable* _M_ht;//指向hash表

  _Hashtable_iterator(_Node* __n, _Hashtable* __tab) 
    : _M_cur(__n), _M_ht(__tab) {}
  _Hashtable_iterator() {}
  reference operator*() const { return _M_cur->_M_val; }
  iterator& operator++();
  iterator operator++(int);
  bool operator==(const iterator& __it) const
    { return _M_cur == __it._M_cur; }
  bool operator!=(const iterator& __it) const
    { return _M_cur != __it._M_cur; }
};

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _All>
_Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>&
_Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>::operator++()
{
  const _Node* __old = _M_cur;
  _M_cur = _M_cur->_M_next;
  if (!_M_cur) {
    size_type __bucket = _M_ht->_M_bkt_num(__old->_M_val);
    while (!_M_cur && ++__bucket < _M_ht->_M_buckets.size())
      _M_cur = _M_ht->_M_buckets[__bucket];
  }
  return *this;
}
```



#### 5.5.2 hashtable的数据结构

```c++
template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc>
class hashtable {
public:
    /* ... */
private:
  typedef _Hashtable_node<_Val> _Node;

public:
  typedef _Alloc allocator_type;
  allocator_type get_allocator() const { return allocator_type(); }
private:
  typedef simple_alloc<_Node, _Alloc> _M_node_allocator_type;
  _Node* _M_get_node() { return _M_node_allocator_type::allocate(1); }
  void _M_put_node(_Node* __p) { _M_node_allocator_type::deallocate(__p, 1); }

private:
  hasher                _M_hash;
  key_equal             _M_equals;
  _ExtractKey           _M_get_key;
  vector<_Node*,_Alloc> _M_buckets;
  size_type             _M_num_elements;

  /* ... */
};
```



#### 5.5.3 hashtable的构造/析构过程

```c++
template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc>
class hashtable {
public:
  /* ... */
public:
  hashtable(size_type __n,
            const _HashFcn&    __hf,
            const _EqualKey&   __eql,
            const _ExtractKey& __ext,
            const allocator_type& __a = allocator_type())
    : __HASH_ALLOC_INIT(__a)
      _M_hash(__hf),
      _M_equals(__eql),
      _M_get_key(__ext),
      _M_buckets(__a),
      _M_num_elements(0)
  {
    _M_initialize_buckets(__n);
  }

  hashtable(size_type __n,
            const _HashFcn&    __hf,
            const _EqualKey&   __eql,
            const allocator_type& __a = allocator_type())
    : __HASH_ALLOC_INIT(__a)
      _M_hash(__hf),
      _M_equals(__eql),
      _M_get_key(_ExtractKey()),
      _M_buckets(__a),
      _M_num_elements(0)
  {
    _M_initialize_buckets(__n);
  }

  hashtable(const hashtable& __ht)
    : __HASH_ALLOC_INIT(__ht.get_allocator())
      _M_hash(__ht._M_hash),
      _M_equals(__ht._M_equals),
      _M_get_key(__ht._M_get_key),
      _M_buckets(__ht.get_allocator()),
      _M_num_elements(0)
  {
    _M_copy_from(__ht);
  }
    
  ~hashtable() { clear(); }
    
private:
  void _M_initialize_buckets(size_type __n)
  {
    const size_type __n_buckets = _M_next_size(__n);
    _M_buckets.reserve(__n_buckets);
    _M_buckets.insert(_M_buckets.end(), __n_buckets, (_Node*) 0);
    _M_num_elements = 0;
  }  
    /* ... */
};
```



#### 5.5.4 ==hashtable元素插入操作==

##### 5.5.4.1 hashtable扩建

```c++
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::resize(size_type __num_elements_hint)
{
  const size_type __old_n = _M_buckets.size();
  if (__num_elements_hint > __old_n) {
    const size_type __n = _M_next_size(__num_elements_hint);
    if (__n > __old_n) {
      vector<_Node*, _All> __tmp(__n, (_Node*)(0),
                                 _M_buckets.get_allocator());
      __STL_TRY {
        for (size_type __bucket = 0; __bucket < __old_n; ++__bucket) {
          _Node* __first = _M_buckets[__bucket];
          while (__first) {
            //将旧哈希表桶中的元素迁移到新哈希表桶中
            size_type __new_bucket = _M_bkt_num(__first->_M_val, __n);
            _M_buckets[__bucket] = __first->_M_next;
            __first->_M_next = __tmp[__new_bucket];
            __tmp[__new_bucket] = __first;
            __first = _M_buckets[__bucket];          
          }
        }
        _M_buckets.swap(__tmp);
      }
#         ifdef __STL_USE_EXCEPTIONS
      catch(...) {
        for (size_type __bucket = 0; __bucket < __tmp.size(); ++__bucket) {
          while (__tmp[__bucket]) {
            _Node* __next = __tmp[__bucket]->_M_next;
            _M_delete_node(__tmp[__bucket]);
            __tmp[__bucket] = __next;
          }
        }
        throw;
      }
#         endif /* __STL_USE_EXCEPTIONS */
    }
  }
}
```



##### 5.5.4.1 独一插入操作

```c++
template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc>
class hashtable {
public:
  /* ... */
  pair<iterator, bool> insert_unique(const value_type& __obj)
  {
    resize(_M_num_elements + 1);
    return insert_unique_noresize(__obj);
  }
  /* ... */
};

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
pair<typename hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::iterator, bool> 
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::insert_unique_noresize(const value_type& __obj)
{
  const size_type __n = _M_bkt_num(__obj);
  _Node* __first = _M_buckets[__n];

  //先检查是否已经在桶中，若已存在则返回false
  for (_Node* __cur = __first; __cur; __cur = __cur->_M_next) 
    if (_M_equals(_M_get_key(__cur->_M_val), _M_get_key(__obj)))
      return pair<iterator, bool>(iterator(__cur, this), false);

  //若不存在桶中，则插入并返回true
  _Node* __tmp = _M_new_node(__obj);
  __tmp->_M_next = __first;
  _M_buckets[__n] = __tmp;
  ++_M_num_elements;
  return pair<iterator, bool>(iterator(__tmp, this), true);
}
```



##### 5.5.4.2 可重复插入操作

```c++
template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc>
class hashtable {
public:
  /* ... */
  iterator insert_equal(const value_type& __obj)
  {
    resize(_M_num_elements + 1);
    return insert_equal_noresize(__obj);
  }
  /* ... */
};

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
typename hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::iterator 
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::insert_equal_noresize(const value_type& __obj)
{
  const size_type __n = _M_bkt_num(__obj);
  _Node* __first = _M_buckets[__n];

  for (_Node* __cur = __first; __cur; __cur = __cur->_M_next) 
    if (_M_equals(_M_get_key(__cur->_M_val), _M_get_key(__obj))) {
      //若元素之前就存在，则在其后插入
      _Node* __tmp = _M_new_node(__obj);
      __tmp->_M_next = __cur->_M_next;
      __cur->_M_next = __tmp;
      ++_M_num_elements;
      return iterator(__tmp, this);
    }

  //否则在桶的前头插入
  _Node* __tmp = _M_new_node(__obj);
  __tmp->_M_next = __first;
  _M_buckets[__n] = __tmp;
  ++_M_num_elements;
  return iterator(__tmp, this);
}
```



#### 5.5.5 hashtable的其他操作

##### 5.5.5.1 元素删除操作

```c++
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::clear()
{
  for (size_type __i = 0; __i < _M_buckets.size(); ++__i) {
    _Node* __cur = _M_buckets[__i];
    while (__cur != 0) {
      _Node* __next = __cur->_M_next;
      _M_delete_node(__cur);
      __cur = __next;
    }
    _M_buckets[__i] = 0;
  }
  _M_num_elements = 0;
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
typename hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::size_type 
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::erase(const key_type& __key)
{
  const size_type __n = _M_bkt_num_key(__key);
  _Node* __first = _M_buckets[__n];
  size_type __erased = 0;

  if (__first) {
    _Node* __cur = __first;
    _Node* __next = __cur->_M_next;
    while (__next) {
      if (_M_equals(_M_get_key(__next->_M_val), __key)) {
        __cur->_M_next = __next->_M_next;
        _M_delete_node(__next);
        __next = __cur->_M_next;
        ++__erased;
        --_M_num_elements;
      }
      else {
        __cur = __next;
        __next = __cur->_M_next;
      }
    }
    if (_M_equals(_M_get_key(__first->_M_val), __key)) {
      _M_buckets[__n] = __first->_M_next;
      _M_delete_node(__first);
      ++__erased;
      --_M_num_elements;
    }
  }
  return __erased;
}
```



##### 5.5.5.2 整体拷贝操作

```c++
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::_M_copy_from(const hashtable& __ht)
{
  _M_buckets.clear();
  _M_buckets.reserve(__ht._M_buckets.size());
  _M_buckets.insert(_M_buckets.end(), __ht._M_buckets.size(), (_Node*) 0);
  __STL_TRY {
    for (size_type __i = 0; __i < __ht._M_buckets.size(); ++__i) {
      const _Node* __cur = __ht._M_buckets[__i];
      if (__cur) {
        //拷贝ht中的一个有效桶
        _Node* __copy = _M_new_node(__cur->_M_val);
        _M_buckets[__i] = __copy;

        //迭代拷贝桶中的串链
        for (_Node* __next = __cur->_M_next; 
             __next; 
             __cur = __next, __next = __cur->_M_next) {
          __copy->_M_next = _M_new_node(__next->_M_val);
          __copy = __copy->_M_next;
        }
      }
    }
    _M_num_elements = __ht._M_num_elements;
  }
  __STL_UNWIND(clear());
}
```



##### 5.5.5.3 查找操作

```c++
template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc>
class hashtable {
public:
  /* ... */
  iterator find(const key_type& __key) 
  {
    size_type __n = _M_bkt_num_key(__key);
    _Node* __first;
    for ( __first = _M_buckets[__n];
          __first && !_M_equals(_M_get_key(__first->_M_val), __key);
          __first = __first->_M_next)
      {}
    return iterator(__first, this);
  } 
  /* ... */
};
```



### 5.6 由hashtable衍生的无序关联容器

在SGI STL中由于hashtable衍生出来的无序关联容器在那个年代还未加入到C++的标准中，它们并没有规范、完整的hash仿函数、判等equalKey仿函数可以供使用，因此用户必须为这些容器提供针对健key的特例化hash仿函数和判断equalKey仿函数。当然在当前时代下，无序关联容器unordered_map、unordered_set、unordered_multimap、unordered_mutliset并没有这种担忧。

由于下面的无序关联容器基本上都是hashtable这一容器的简单封装，并没有太多的学习价值，因此仅简单的列出关键的代码部分。

#### 5.6.1 hash_set

```c++
template <class _Value, class _HashFcn, class _EqualKey, class _Alloc>
class hash_set
{
  /* ... */
private:
  typedef hashtable<_Value, _Value, _HashFcn, _Identity<_Value>, 
                    _EqualKey, _Alloc> _Ht;
  _Ht _M_ht;
  /* ... */
};
```



#### 5.6.2 hash_map

```c++
template <class _Key, class _Tp, class _HashFcn, class _EqualKey,
          class _Alloc>
class hash_map
{
  /* ... */
private:
  typedef hashtable<pair<const _Key,_Tp>,_Key,_HashFcn,
                    _Select1st<pair<const _Key,_Tp> >,_EqualKey,_Alloc> _Ht;
  _Ht _M_ht;

  /* ... */
  _Tp& operator[](const key_type& __key) {
    return _M_ht.find_or_insert(value_type(__key, _Tp())).second;
  }
  /* ... */
};
```



#### 5.6.3 hash_multiset

```c++
template <class _Value, class _HashFcn, class _EqualKey, class _Alloc>
class hash_multiset
{
  /* ... */
private:
  typedef hashtable<_Value, _Value, _HashFcn, _Identity<_Value>, 
                    _EqualKey, _Alloc> _Ht;
  _Ht _M_ht;
  /* ... */
};
```



#### 5.6.4 hash_multimap

```c++
template <class _Key, class _Tp, class _HashFcn, class _EqualKey, 
          class _Alloc>
class hash_multimap
{
  /* ... */
private:
  typedef hashtable<pair<const _Key, _Tp>, _Key, _HashFcn,
                    _Select1st<pair<const _Key, _Tp> >, _EqualKey, _Alloc> 
          _Ht;
  _Ht _M_ht;
  /* ... */
};
```



