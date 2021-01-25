### 5.3 由_Rb_tree衍生的有序关联容器

#### 5.3.1 set

set仅仅是红黑树_Rb_tree的简单套壳，并由`insert_unique()`保证容器中不会出现重复的键值。其之所以能够使得set只记录键值而不记录实值，是以将红黑树结点value_filed值域整个解析成键值，而不保存实值的方式来实现的。其实现文件位于[stl_set.h](stl_set.h)。



#### 5.3.2 map

map也仅仅是红黑树_Rb_tree的简单套壳，并由`insert_unique()`保证容器中不会出现重复的键值。它通过让红黑树结点value_field值域存储一个pair的数据类型，既保存键值又保存实值，并在使用时借用一个函数对象从value_field中取出键值，从而实现键-值对搜索表的功能。其实现文件位于[stl_map.h](stl_map.h)。

```c++
template <class _Key, class _Tp, class _Compare, class _Alloc>
class map {
public:
  typedef _Key                  key_type;
  typedef _Tp                   data_type;
  typedef _Tp                   mapped_type;
  typedef pair<const _Key, _Tp> value_type;
  typedef _Compare              key_compare;

  //嵌套类
  class value_compare
    : public binary_function<value_type, value_type, bool> {
  friend class map<_Key,_Tp,_Compare,_Alloc>;
  protected :
    _Compare comp;
    value_compare(_Compare __c) : comp(__c) {}
  public:
    bool operator()(const value_type& __x, const value_type& __y) const {
      return comp(__x.first, __y.first);
    }
  };
  /* ... */
private:
  typedef _Rb_tree<key_type, value_type, 
                   _Select1st<value_type>, key_compare, _Alloc> _Rep_type;
  _Rep_type _M_t;  // red-black tree representing map
  /* ... */
  _Tp& operator[](const key_type& __k) {
    iterator __i = lower_bound(__k);
    // __i->first is greater than or equivalent to __k.
    /* 不再像书中的STL实现版本那样直接插入，而是先查找下是否存在
    	，只有不存在的时候才执行插入操作 */
    if (__i == end() || key_comp()(__k, (*__i).first))
      __i = insert(__i, value_type(__k, _Tp()));
    return (*__i).second;
  }
};
```



#### 5.3.3 multimap

multimap也是红黑树_Rb_tree的简单套壳，并使用`insert_equal()`允许容器中多个重复的键值。其实现文件位于[stl_multimap.h](stl_multimap.h)。



#### 5.3.4 multiset

multiset也是红黑树_Rb_tree的简单套壳，同样使用`insert_equal()`允许容器中存储多个重复的键值，其实现文件位于[stl_multiset.h](stl_multiset.h)。



### 5.4 哈希表

