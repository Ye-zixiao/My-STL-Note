## 5. 关联容器

关联容器的侧重点在于查找，其中有序关联容器的初衷在于键值有序+快速的查找（使用红黑树可使查找插入、删除、查找操作的时间复杂度达到$O(logN)$），而无序关联容器的初衷只在于更快的查找（使用哈希表可使插入、删除、查找操作的时间复杂度达到$O(1)$）。

其中文件[AVLTree.h](AVLTree.h)按照规则简单的实现了AVL树，但仅仅支持插入等少数几个操作，尤其是不支持删除单个指定元素的操作。

对于set、map、multiset和multimap这些有序关联容器其内部是使用红黑树来实现的，而对于像hash_set、hash_map、hash_multiset以及hash_multimap（对于对应当前C++ STL下的unordered_set、unordered_map、unordered_multiset和unordered_multimap）这些无序关联容器则是通过哈希表来实现的。



### 5.1 平衡二叉树

为了能够深入的理解有序关联容器内部所使用的红黑树，我们必须先从平衡二叉树然后过渡到红黑树，分析最底层的原理才能更好的理解各种有序关联容器的实现。

#### 5.1.1 AVL树



#### 5.1.2 红黑树

红黑树的本质就是通过二叉树的方式来模拟构建2-3-4树。在《算法4》中我们其实已经了解以各种比较特殊的红黑树——左倾红黑树，它为了实现的方便，模拟构建的是2-3树而不是2-3-4树，但从本质上讲两者都是相同的。

假设一个二叉树符合红黑树的规定，那么以如下几种情况插入会破坏红黑树的平衡性（如下图所示）：

1. 向一个左倾3-结点“左左”、“左右”插入
2. 向一个右倾3-结点“右左”、“右右”插入
3. 向一个4-结点以任何方式插入

<img src="../../image/红黑树插入.jpg" alt="红黑树插入" style="zoom: 50%;" />



从上面的描述我们知道，向一个4-结点插入新的红结点会造成复杂的插入处理动作，因此SGI STL采用如下的方式来避免红黑树出现向一个4-结点插入的情况：在插入操作向下递归游走的时候及时将路径上的4-结点分解成3-结点或者2-结点，并将提取出的红结点插入到上一层，从而尽可能在新红节点插入的位置发生向4-结点插入的情况！

<img src="../../image/屏幕截图 2021-01-22 231315.png" alt="屏幕截图 2021-01-22 231315" style="zoom:65%;" />

为了强调SGI STL关于对红黑树的实现，我并不将RB-tree的代码解读放在红黑树原理小节中，而是单独放在下一节中。



### 5.2 _Rb_tree



#### 5.2.1 \_Rb\_tree结点和迭代器

```c++
#include <iostream>
#include <set>
using namespace std;

int main(){
	set<int> iset{1,2,3,4,5};

	auto begIter=iset.begin();
	for(int i=0;i<10;++i){
		if(begIter==iset.end())
			cout<<"it is endptr"<<endl;
		else
			cout<<*begIter<<endl;
		begIter++;
	}
	cout<<endl;

	auto endIter=iset.end();
	for(int i=0;i<10;++i){
		if(endIter==iset.end())
			cout<<"it is endptr"<<endl;
		else
			cout<<*endIter<<endl;
		endIter--;
	}
	return 0;
}

/*结果：
1
2
3
4
5
it is endptr
5
it is endptr
5
it is endptr

it is endptr
5
4
3
2
1
it is endptr
5
4
3
*/
```





#### 5.2.2 _Rb_tree的数据结构

实际上，若是从源文件的角度讲，其真正的红黑树类应该叫做rb_tree，但其本质就是对_Rb_tree的简单继承，而\_Rb_tree实际上又是对\_Rb_tree_base的继承。如下的类定义代码大约在源文件的第529行：

```c++
template <class _Tp, class _Alloc>
struct _Rb_tree_base
{
  typedef _Alloc allocator_type;
  allocator_type get_allocator() const { return allocator_type(); }

  _Rb_tree_base(const allocator_type&) 
    : _M_header(0) { _M_header = _M_get_node(); }
  ~_Rb_tree_base() { _M_put_node(_M_header); }

protected:
  _Rb_tree_node<_Tp>* _M_header;

  typedef simple_alloc<_Rb_tree_node<_Tp>, _Alloc> _Alloc_type;

  _Rb_tree_node<_Tp>* _M_get_node()
    { return _Alloc_type::allocate(1); }
  void _M_put_node(_Rb_tree_node<_Tp>* __p)
    { _Alloc_type::deallocate(__p, 1); }
};

#endif /* __STL_USE_STD_ALLOCATORS */

template <class _Key, class _Value, class _KeyOfValue, class _Compare,
          class _Alloc = __STL_DEFAULT_ALLOCATOR(_Value) >
class _Rb_tree : protected _Rb_tree_base<_Value, _Alloc> {
  /* ... */
protected:
  size_type _M_node_count; // keeps track of size of tree
  _Compare _M_key_compare;
  /* ... */
};
```



#### 5.2.3 _Rb_tree的构造/析构过程

默认构造、拷贝构造

```c++
template <class _Tp, class _Alloc>
struct _Rb_tree_base
{
  typedef _Alloc allocator_type;
  allocator_type get_allocator() const { return allocator_type(); }

  _Rb_tree_base(const allocator_type&) 
    : _M_header(0) { _M_header = _M_get_node(); }
  ~_Rb_tree_base() { _M_put_node(_M_header); }

protected:
  _Rb_tree_node<_Tp>* _M_header;

  typedef simple_alloc<_Rb_tree_node<_Tp>, _Alloc> _Alloc_type;

  _Rb_tree_node<_Tp>* _M_get_node()
    { return _Alloc_type::allocate(1); }
  void _M_put_node(_Rb_tree_node<_Tp>* __p)
    { _Alloc_type::deallocate(__p, 1); }
};

template <class _Key, class _Value, class _KeyOfValue, class _Compare,
          class _Alloc = __STL_DEFAULT_ALLOCATOR(_Value) >
class _Rb_tree : protected _Rb_tree_base<_Value, _Alloc> {
  /* ... */
public:
  _Rb_tree()
    : _Base(allocator_type()), _M_node_count(0), _M_key_compare()
    { _M_empty_initialize(); }

  _Rb_tree(const _Compare& __comp)
    : _Base(allocator_type()), _M_node_count(0), _M_key_compare(__comp) 
    { _M_empty_initialize(); }

  _Rb_tree(const _Compare& __comp, const allocator_type& __a)
    : _Base(__a), _M_node_count(0), _M_key_compare(__comp) 
    { _M_empty_initialize(); }

  _Rb_tree(const _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x) 
    : _Base(__x.get_allocator()),
      _M_node_count(0), _M_key_compare(__x._M_key_compare)
  { 
    if (__x._M_root() == 0)
      _M_empty_initialize();
    else {
      _S_color(_M_header) = _S_rb_tree_red;
      _M_root() = _M_copy(__x._M_root(), _M_header);
      _M_leftmost() = _S_minimum(_M_root());
      _M_rightmost() = _S_maximum(_M_root());
    }
    _M_node_count = __x._M_node_count;
  }
  ~_Rb_tree() { clear(); }
    
private:
  void _M_empty_initialize() {
    _S_color(_M_header) = _S_rb_tree_red; // used to distinguish header from 
                                          // __root, in iterator.operator++
    _M_root() = 0;
    _M_leftmost() = _M_header;
    _M_rightmost() = _M_header;
  }
    
  /* ... */
};
```



析构过程



#### 5.2.4 ==\_Rb\_tree元素插入操作==

##### 5.2.4.1 左右旋操作

```c++
inline void 
_Rb_tree_rotate_left(_Rb_tree_node_base* __x, _Rb_tree_node_base*& __root)
{
  _Rb_tree_node_base* __y = __x->_M_right;
  __x->_M_right = __y->_M_left;
  if (__y->_M_left !=0)
    __y->_M_left->_M_parent = __x;
  __y->_M_parent = __x->_M_parent;

  if (__x == __root)
    __root = __y;
  else if (__x == __x->_M_parent->_M_left)
    __x->_M_parent->_M_left = __y;
  else
    __x->_M_parent->_M_right = __y;
  __y->_M_left = __x;
  __x->_M_parent = __y;
}

inline void 
_Rb_tree_rotate_right(_Rb_tree_node_base* __x, _Rb_tree_node_base*& __root)
{
  _Rb_tree_node_base* __y = __x->_M_left;
  __x->_M_left = __y->_M_right;
  if (__y->_M_right != 0)
    __y->_M_right->_M_parent = __x;
  __y->_M_parent = __x->_M_parent;

  if (__x == __root)
    __root = __y;
  else if (__x == __x->_M_parent->_M_right)
    __x->_M_parent->_M_right = __y;
  else
    __x->_M_parent->_M_left = __y;
  __y->_M_right = __x;
  __x->_M_parent = __y;
}
```



##### 5.2.4.1 可重复插入操作

由于在红黑树中，范围元素插入操作都是由单元素插入操作实现的，所以这里只介绍

```c++
template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
typename _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::iterator
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::insert_equal(const _Value& __v)
{
  _Link_type __y = _M_header;
  _Link_type __x = _M_root();
  while (__x != 0) {
    __y = __x;
    //_KeyofValue()会提取出__v的键值，而_S_key()也会默认提取出__x的键值
    __x = _M_key_compare(_KeyOfValue()(__v), _S_key(__x)) ? 
            _S_left(__x) : _S_right(__x);
  }
  return _M_insert(__x, __y, __v);
}
```



##### 5.2.4.2 独一插入操作

```c++
template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
pair<typename _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::iterator, 
     bool>
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::insert_unique(const _Value& __v)
{
  _Link_type __y = _M_header;
  _Link_type __x = _M_root();
  bool __comp = true;
  while (__x != 0) {
    __y = __x;
    __comp = _M_key_compare(_KeyOfValue()(__v), _S_key(__x));
    __x = __comp ? _S_left(__x) : _S_right(__x);
  }
  iterator __j = iterator(__y);   
  if (__comp)
    if (__j == begin())     
      return pair<iterator,bool>(_M_insert(__x, __y, __v), true);
    else
      --__j;
  if (_M_key_compare(_S_key(__j._M_node), _KeyOfValue()(__v)))
    return pair<iterator,bool>(_M_insert(__x, __y, __v), true);
  return pair<iterator,bool>(__j, false);
}
```



##### 5.2.4.3 实际插入操作

```c++
template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
typename _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::iterator
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::_M_insert(_Base_ptr __x_, _Base_ptr __y_, const _Value& __v)
{
  _Link_type __x = (_Link_type) __x_;
  _Link_type __y = (_Link_type) __y_;
  _Link_type __z;

  //左边插入
  if (__y == _M_header || __x != 0 || 
      _M_key_compare(_KeyOfValue()(__v), _S_key(__y))) {
    __z = _M_create_node(__v);
    _S_left(__y) = __z;               // also makes _M_leftmost() = __z 
                                      //    when __y == _M_header
    if (__y == _M_header) {
      _M_root() = __z;
      _M_rightmost() = __z;
    }
    else if (__y == _M_leftmost())
      _M_leftmost() = __z;   // maintain _M_leftmost() pointing to min node
  }
  //右边插入
  else {
    __z = _M_create_node(__v);
    _S_right(__y) = __z;
    if (__y == _M_rightmost())
      _M_rightmost() = __z;  // maintain _M_rightmost() pointing to max node
  }
  _S_parent(__z) = __y;
  _S_left(__z) = 0;
  _S_right(__z) = 0;
  //维持红黑树平衡
  _Rb_tree_rebalance(__z, _M_header->_M_parent);
  ++_M_node_count;
  return iterator(__z);
}
```



##### 5.2.4.4 红黑树再平衡

当一个新结点插入到红黑树之后可能并没有满足红黑树的平衡规范，因此它必须在插入完成之后调用下面名为`_Rb_tree_rebalance()`的函数以实现红黑树的再平衡。如我们所见该函数主要将我们在上面红黑树理论小节中描述的8种情况分成如下两大类进行处理：

1. 向一个左倾3-结点或者向一个4-结点左侧插入新结点；
2. 向一个右倾3-结点或者向一个4-结点右侧插入新结点。

区分这两种手段的依据是看新插入结点的父结点是否是祖先节点的左孩子还是右孩子，然后再去区分伯父结点是否是红节点，若是红节点，这说明新结点插入到了一个4-结点身上，此时的处理的手段是将4-结点分解成2-3结点+3-结点的组合，从而避免因向上提取红节点而造成的过多的旋转操作；否则就是插入到一个3-结点，此时仅仅依靠单旋操作就足够了。

```c++
inline void 
_Rb_tree_rebalance(_Rb_tree_node_base* __x, _Rb_tree_node_base*& __root)
{
  __x->_M_color = _S_rb_tree_red;
  while (__x != __root && __x->_M_parent->_M_color == _S_rb_tree_red) {
    /* 插入结点的父结点是祖父结点的左孩子 */
    if (__x->_M_parent == __x->_M_parent->_M_parent->_M_left) {
      _Rb_tree_node_base* __y = __x->_M_parent->_M_parent->_M_right;
      //处理4-结点左侧插入这种破坏平衡的情况
      if (__y && __y->_M_color == _S_rb_tree_red) {
        __x->_M_parent->_M_color = _S_rb_tree_black;
        __y->_M_color = _S_rb_tree_black;
        __x->_M_parent->_M_parent->_M_color = _S_rb_tree_red;
        __x = __x->_M_parent->_M_parent;
      }
      //处理左倾3-结点插入这种破坏平衡的情况
      else {
        if (__x == __x->_M_parent->_M_right) {
          __x = __x->_M_parent;
          _Rb_tree_rotate_left(__x, __root);
        }
        __x->_M_parent->_M_color = _S_rb_tree_black;
        __x->_M_parent->_M_parent->_M_color = _S_rb_tree_red;
        _Rb_tree_rotate_right(__x->_M_parent->_M_parent, __root);
      }
    }
    /* 插入结点的父结点时祖父结点的右孩子 */
    else {
      _Rb_tree_node_base* __y = __x->_M_parent->_M_parent->_M_left;
      //处理4-结点右侧插入这种破坏平衡的情况
      if (__y && __y->_M_color == _S_rb_tree_red) {
        __x->_M_parent->_M_color = _S_rb_tree_black;
        __y->_M_color = _S_rb_tree_black;
        __x->_M_parent->_M_parent->_M_color = _S_rb_tree_red;
        __x = __x->_M_parent->_M_parent;
      }
      //处理右倾3-结点插入这种破坏平衡的情况
      else {
        if (__x == __x->_M_parent->_M_left) {
          __x = __x->_M_parent;
          _Rb_tree_rotate_right(__x, __root);
        }
        __x->_M_parent->_M_color = _S_rb_tree_black;
        __x->_M_parent->_M_parent->_M_color = _S_rb_tree_red;
        _Rb_tree_rotate_left(__x->_M_parent->_M_parent, __root);
      }
    }
  }
  __root->_M_color = _S_rb_tree_black;
}
```



#### 5.2.5 _Rb_tree元素删除操作



#### 5.2.6 \_Rb_tree元素搜索操作

```c++
template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
typename _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::iterator 
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::find(const _Key& __k)
{
  _Link_type __y = _M_header;      // Last node which is not less than __k. 
  _Link_type __x = _M_root();      // Current node. 

  while (__x != 0) 
    if (!_M_key_compare(_S_key(__x), __k))
      __y = __x, __x = _S_left(__x);
    else
    /* 这里不更新y是为了判定出__y左孩子的右子树任意节点<__k<__y的出界情况 */
      __x = _S_right(__x);

  iterator __j = iterator(__y);   
  return (__j == end() || _M_key_compare(__k, _S_key(__j._M_node))) ? 
     end() : __j;
}
```



### 5.3 由_Rb_tree衍生的关联容器

