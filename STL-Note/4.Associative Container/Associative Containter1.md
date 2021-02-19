### 5.2 _Rb_tree

SGI STL红黑树容器\_Rb_tree并非是C++标准库中的成员，我们并不能直接使用到它，但它确实是STL4种有序关联容器的底层基础容器。它的实现位于源文件[stl_tree.h](stl_tree.h)中，其中比较需要我们关注的部分如下所示：

1. **_Rb_tree的数据结构、节点和迭代器**
2. _Rb_tree的构造和析构过程
3. **元素插入和插入之后所需的红黑树再平衡操作**
4. **元素删除和删除之后所需的红黑树再平衡操作**
5. 其他操作，看看就行



#### 5.2.1 ==\_Rb\_tree节点和迭代器==

##### 5.2.1.1 _Rb_tree节点

SGI STL红黑树容器_Rb_tree的节点、迭代器都是采用了两级继承的结构。其中红黑树节点的基类为\_Rb_tree_node_base，派生类（也即红黑树真正使用的节点）为\_Rb_tree_node，整体上看其实它的内容非常简单，不过需要注意的是它有一个特别的成员指针是用来指向其父节点的。其源代码如下：

```c++
typedef bool _Rb_tree_Color_type;
const _Rb_tree_Color_type _S_rb_tree_red = false;
const _Rb_tree_Color_type _S_rb_tree_black = true;

struct _Rb_tree_node_base
{
  typedef _Rb_tree_Color_type _Color_type;
  typedef _Rb_tree_node_base* _Base_ptr;

  _Color_type _M_color; 
  _Base_ptr _M_parent;
  _Base_ptr _M_left;
  _Base_ptr _M_right;

  static _Base_ptr _S_minimum(_Base_ptr __x)
  {
    while (__x->_M_left != 0) __x = __x->_M_left;
    return __x;
  }

  static _Base_ptr _S_maximum(_Base_ptr __x)
  {
    while (__x->_M_right != 0) __x = __x->_M_right;
    return __x;
  }
};

template <class _Value>
struct _Rb_tree_node : public _Rb_tree_node_base
{
  typedef _Rb_tree_node<_Value>* _Link_type;
  _Value _M_value_field;
};
```



##### 5.2.1.2 _Rb_tree迭代器

红黑树迭代器的基类为\_Rb\_tree\_base\_iterator，其派生类为_Rb_tree_iterator（红黑树真正使用的）。红黑树迭代器最大的责任是实现在红黑树这种特殊的非顺序容器上实现向前或向后步进的操作，以符合红黑树迭代器为双向迭代器forward_iterator的要求。这两个步进操作的实际工作基本上都是在迭代器基类中完成，分别由`_M_increment()`和`_M_decrement()`成员函数实现。

当一个红黑树迭代器需要向前步进时，①它会先查看当前节点的右子树是否存在，若存在，它会步入当前节点的右子树中，找到右子树中的最小节点，然后使迭代器指向它；②若右子节点不存在，那么它会以while循环的姿态去判断自己是否是父节点的右子节点，若是则将迭代器自己暂时指向父节点，然后重新迭代检查，直到当前节点不再是父节点的右子节点为止。当从while循环退出后再将迭代器指向这个节点的父节点从完成向上方式的步进。

而当一个红黑树迭代器需要向后步进的时候会有一些不同之处，它首先会判断当前节点是否是\_Rb_tree的header节点（我们会在下面的红黑树数据结构中了解到SGI STL红黑树的根节点其实有一个特殊的父节点，它同时也是红节点，充当哨兵的作用），若是，则直接将迭代器指向红黑树中的最大节点；否则会按照向前步进类似的逻辑实现对应操作。这些部分的实现大致位于源代码的第102行：

```c++
struct _Rb_tree_base_iterator
{
  typedef _Rb_tree_node_base::_Base_ptr _Base_ptr;
  typedef bidirectional_iterator_tag iterator_category;
  typedef ptrdiff_t difference_type;
  _Base_ptr _M_node;

  void _M_increment()
  {
    //若存在右子节点，则指向右子树中的最小节点
    if (_M_node->_M_right != 0) {
      _M_node = _M_node->_M_right;
      while (_M_node->_M_left != 0)
        _M_node = _M_node->_M_left;
    }
    //若不存在右子节点，则沿路向上
    else {
      _Base_ptr __y = _M_node->_M_parent;
      while (_M_node == __y->_M_right) {
        _M_node = __y;
        __y = __y->_M_parent;
      }
      if (_M_node->_M_right != __y)
        _M_node = __y;
    }
  }

  void _M_decrement()
  {
    //若存在左子节点，则指向左子树的最大节点
    if (_M_node->_M_color == _S_rb_tree_red &&
        _M_node->_M_parent->_M_parent == _M_node)
      _M_node = _M_node->_M_right;
    else if (_M_node->_M_left != 0) {
      _Base_ptr __y = _M_node->_M_left;
      while (__y->_M_right != 0)
        __y = __y->_M_right;
      _M_node = __y;
    }
    //若不存在左子节点，则沿路径向上
    else {
      _Base_ptr __y = _M_node->_M_parent;
      while (_M_node == __y->_M_left) {
        _M_node = __y;
        __y = __y->_M_parent;
      }
      _M_node = __y;
    }
  }
};

template <class _Value, class _Ref, class _Ptr>
struct _Rb_tree_iterator : public _Rb_tree_base_iterator
{
  /* ... */
  _Self& operator++() { _M_increment(); return *this; }
  _Self operator++(int) {
    _Self __tmp = *this;
    _M_increment();
    return __tmp;
  }
    
  _Self& operator--() { _M_decrement(); return *this; }
  _Self operator--(int) {
    _Self __tmp = *this;
    _M_decrement();
    return __tmp;
  }
};
```

按照`increment()`和`decrement()`这两个函数的规则，我们可以发现一个有趣的点：①当红黑树迭代器从最小节点处开始不断地执行向前步进操作（不施以`!=_Rb_tree.end()`停止条件）时，一旦它遍历过header节点（也即尾后/哨兵节点）之后，它会跳到红黑树最大节点的左子树中的最小节点处，重新向前步进，形成一个固定的局部循环（如下面绿色箭头所示）。②而当红黑树迭代器从树中任一点开始向后不断步进步进（不加以停止）时，它却会从尾到前又从前跳到尾不断循环（如下面红黑箭头所示）。虽然这两个点在实际中并不会用到，但它们对于我们了解红黑树迭代器的工作原理有所帮助。

<img src="..\..\image\红黑树迭代器2.jpg" alt="红黑树迭代器2" style="zoom:50%;" />

下面的示例程序正反映了上述两点：

```c++
#include <iostream>
#include <functional>
#include <bits/stl_tree.h>
using namespace std;

template<typename T>
class KeyofValue : public unary_function<T, T> {
public:
	const T &operator()(const T &value) const {
		return value;
	}
};

int main() {
	_Rb_tree<int, int, KeyofValue<int>, less<>> itree;
	itree._M_insert_unique(23);
	itree._M_insert_unique(32);
	itree._M_insert_unique(12);
	itree._M_insert_unique(56);

	auto iter = itree.begin();
	for (int i = 0; i < 10; i++) {
		if (iter == itree.end())
			cout << "endptr" << ' ';
		else cout << *iter << ' ';
		++iter;
	}
	cout << endl;

	iter = itree.begin();
	for (int i = 0; i < 10; i++) {
		if (iter == itree.end())
			cout << "endptr" << ' ';
		else cout << *iter << ' ';
		--iter;
	}
	return 0;
}

/*结果：
12 23 32 56 endptr 56 endptr 56 endptr 56
12 endptr 56 32 23 12 endptr 56 32 23
*/
```



#### 5.2.2 _Rb_tree的数据结构

实际上，若是从源文件的角度讲，其真正的红黑树类应该叫做rb_tree，但其本质就是对_Rb_tree的简单继承，而\_Rb_tree实际上又是对\_Rb_tree_base的继承，其中\_Rb_tree是我们学习的重点。如下的类定义代码大约在源文件的第529行：

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



##### 5.2.4.4 插入节点树形再平衡

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
    /* 插入结点的父结点是祖父结点的左孩子，则此时处理如下4种情况：
    	1、左倾3-结点“左左”插入
    	2、左倾3-结点“左右”插入
    	3、4-结点“左左”插入
    	4、4-结点“左右”插入
    */
    if (__x->_M_parent == __x->_M_parent->_M_parent->_M_left) {
      //_y是插入节点x的伯父
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
    /* 插入结点的父结点时祖父结点的右孩子，则此时处理如下4种情况：
    	5、右倾3-结点“右左”插入
    	6、右倾3-结点“右右”插入
    	7、4-结点“右左”插入
    	8、4-结点“右右”插入
    */
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



#### 5.2.5 ==_Rb_tree元素删除操作==

##### 5.2.5.1 元素删除操作的外包装

```c++
template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
typename _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::size_type 
_Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::erase(const _Key& __x)
{
  //获得指定键值结点在红黑树中的上边沿和下边沿
  pair<iterator,iterator> __p = equal_range(__x);
  size_type __n = 0;
  //获得待删除结点的个数
  distance(__p.first, __p.second, __n);
  erase(__p.first, __p.second);
  return __n;
}

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
void _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::erase(iterator __first, iterator __last)
{
  if (__first == begin() && __last == end())
    clear();
  else
    while (__first != __last) erase(__first++);
}

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
inline void _Rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
  ::erase(iterator __position)
{
  _Link_type __y = 
    (_Link_type) _Rb_tree_rebalance_for_erase(__position._M_node,
                                              _M_header->_M_parent,
                                              _M_header->_M_left,
                                              _M_header->_M_right);
  destroy_node(__y);
  --_M_node_count;
}
```



##### 5.2.5.2 待删节点树形再平衡

红黑树在删除一个指定的结点前会首先调用一个名为`_Rb_tree_rebalance_for_erase()`的树形调整函数，它从红黑树中取出该结点然后针对结点缺失后造成的空缺进行树形调整操作，使得删除该结点后的红黑树仍然符合红黑树的平衡规范。

| 待删节点情况 |                           删除方式                           |
| :----------: | :----------------------------------------------------------: |
|     无子     | 若待删节点红（3-节点或者4-节点），则无需处理；若黑（2-节点），则需要特殊的树形调整处理 |
|     独子     | 待删节点只能为黑，且子节点必为红，此时只需要将子节点变黑然后挂接到待删节点的父节点下 |
|     双子     | 需要在待删节点的右子树上找一个替代节点来替代待删节点，而替代节点必然是一个无子或独子节点，这样此时对待删节点树形调整问题就转化成了对替代节点的树形调整问题 |

| 待删节点情况 |                           删除方式                           |
| :----------: | :----------------------------------------------------------: |
|     无子     | 若待删节点红（3或4-节点），则无需处理；若黑（2-节点），则需要特殊的树形调整处理 |
|     独子     | 待删节点只能为黑，且子节点必为红，此时只需将子节点变黑然后重新挂接 |
|     双子     | 在待删节点的右子树找一个替代节点，间接转换为对替代节点的树形调整问题 |

上面描述的待删节点的各种情况中，最复杂的就是待删节点无子且为黑的情况，从处理的角度上主要分成两种处理类型：①一种是待删节点是其父节点的左子节点；②另一种情况是待删节点是其父节点右子节点。但两种其实对比度比较高，理解其中一种就可以很好的理解另一种情况。上述两大类每类都有9种情况，且9种情况之间存在相互转换的关系，而SGI STL对待删节点的树形再平衡处理正是利用了这些转换关系，从而降低处理的复杂程度（但实际上还是很复杂的😂！下面将近150行的代码已经说明了这一切！）。这18种情形如下所示：

<img src="..\..\image\红黑树删除的18种情况.jpg" alt="红黑树删除的18种情况" style="zoom: 50%;" />

上述演示图不仅展示了删除结点时造成红黑树平衡破坏的18种情况，还展示了SGI STL中树形再平衡函数针对待删节点为其父节点的左子节点的大致处理流程，也即源代码[stl_tree.h](stl_tree.h)385~415行的代码部分。

```c++
inline _Rb_tree_node_base*
_Rb_tree_rebalance_for_erase(_Rb_tree_node_base* __z,
                             _Rb_tree_node_base*& __root,
                             _Rb_tree_node_base*& __leftmost,
                             _Rb_tree_node_base*& __rightmost)
{
  _Rb_tree_node_base* __y = __z;
  _Rb_tree_node_base* __x = 0;
  _Rb_tree_node_base* __x_parent = 0;
  /* 1、当待删节点无子或独子时，__y记录待删节点，__x记录它的左子节点（有可能为null）或者右子节点；
     2、当待删节点存在双子时，__y记录右子树中的最小节点，__x记录右子树最小节点的右子节点 */
  if (__y->_M_left == 0)     // __z has at most one non-null child. y == z.
    __x = __y->_M_right;     // __x might be null.
  else
    if (__y->_M_right == 0)  // __z has exactly one non-null child. y == z.
      __x = __y->_M_left;    // __x is not null.
    else {                   // __z has two non-null children.  Set __y to
      __y = __y->_M_right;   //   __z's successor.  __x might be null.
      while (__y->_M_left != 0)
        __y = __y->_M_left;
      __x = __y->_M_right;
    }
  //待删节点存在双子
  if (__y != __z) {          // relink y in place of z.  y is z's successor
    __z->_M_left->_M_parent = __y; 
    __y->_M_left = __z->_M_left;
    if (__y != __z->_M_right) {
      __x_parent = __y->_M_parent;
      if (__x) __x->_M_parent = __y->_M_parent;
      __y->_M_parent->_M_left = __x;      // __y must be a child of _M_left
      __y->_M_right = __z->_M_right;
      __z->_M_right->_M_parent = __y;
    }
    else
      __x_parent = __y;  
    if (__root == __z)
      __root = __y;
    else if (__z->_M_parent->_M_left == __z)
      __z->_M_parent->_M_left = __y;
    else 
      __z->_M_parent->_M_right = __y;
    __y->_M_parent = __z->_M_parent;
    
    __STD::swap(__y->_M_color, __z->_M_color);
    __y = __z;
    // __y now points to node to be actually deleted
  }
  //待删节点独子或者无子
  else {                        // __y == __z
    //重新认亲
    __x_parent = __y->_M_parent;
    if (__x) __x->_M_parent = __y->_M_parent;   
    if (__root == __z)
      __root = __x;
    else 
      if (__z->_M_parent->_M_left == __z)
        __z->_M_parent->_M_left = __x;
      else
        __z->_M_parent->_M_right = __x;

    //若删除的是最小键值结点或者最大键值结点
    if (__leftmost == __z) 
      if (__z->_M_right == 0)        // __z->_M_left must be null also
        __leftmost = __z->_M_parent;
    // makes __leftmost == _M_header if __z == __root
      else
        __leftmost = _Rb_tree_node_base::_S_minimum(__x);
    if (__rightmost == __z)  
      if (__z->_M_left == 0)         // __z->_M_right must be null also
        __rightmost = __z->_M_parent;  
    // makes __rightmost == _M_header if __z == __root
      else                      // __x == __z->_M_left
        __rightmost = _Rb_tree_node_base::_S_maximum(__x);
  }
  
  if (__y->_M_color != _S_rb_tree_red) { 
    //若删除的结点不是红节点，那么就得从祖先结点（3-结点或者4-结点中提取出一个结点以下移）
    while (__x != __root && (__x == 0 || __x->_M_color == _S_rb_tree_black))
      //待删节点是其父节点的左节点
      if (__x == __x_parent->_M_left) {
        _Rb_tree_node_base* __w = __x_parent->_M_right;
        //判断兄弟节点是否为红色，这种情况只有一种
        if (__w->_M_color == _S_rb_tree_red) {
          __w->_M_color = _S_rb_tree_black;
          __x_parent->_M_color = _S_rb_tree_red;
          _Rb_tree_rotate_left(__x_parent, __root);
          __w = __x_parent->_M_right;
        }
        //判断兄弟节点的子节点是否全黑
        if ((__w->_M_left == 0 || 
             __w->_M_left->_M_color == _S_rb_tree_black) &&
            (__w->_M_right == 0 || 
             __w->_M_right->_M_color == _S_rb_tree_black)) {
          __w->_M_color = _S_rb_tree_red;
          __x = __x_parent;
          __x_parent = __x_parent->_M_parent;
        } else {
          if (__w->_M_right == 0 || 
              __w->_M_right->_M_color == _S_rb_tree_black) {
            if (__w->_M_left) __w->_M_left->_M_color = _S_rb_tree_black;
            __w->_M_color = _S_rb_tree_red;
            _Rb_tree_rotate_right(__w, __root);
            __w = __x_parent->_M_right;
          }
          __w->_M_color = __x_parent->_M_color;
          __x_parent->_M_color = _S_rb_tree_black;
          if (__w->_M_right) __w->_M_right->_M_color = _S_rb_tree_black;
          _Rb_tree_rotate_left(__x_parent, __root);
          break;
        }
      } else {                  // same as above, with _M_right <-> _M_left.
        _Rb_tree_node_base* __w = __x_parent->_M_left;
        if (__w->_M_color == _S_rb_tree_red) {
          __w->_M_color = _S_rb_tree_black;
          __x_parent->_M_color = _S_rb_tree_red;
          _Rb_tree_rotate_right(__x_parent, __root);
          __w = __x_parent->_M_left;
        }
        if ((__w->_M_right == 0 || 
             __w->_M_right->_M_color == _S_rb_tree_black) &&
            (__w->_M_left == 0 || 
             __w->_M_left->_M_color == _S_rb_tree_black)) {
          __w->_M_color = _S_rb_tree_red;
          __x = __x_parent;
          __x_parent = __x_parent->_M_parent;
        } else {
          if (__w->_M_left == 0 || 
              __w->_M_left->_M_color == _S_rb_tree_black) {
            if (__w->_M_right) __w->_M_right->_M_color = _S_rb_tree_black;
            __w->_M_color = _S_rb_tree_red;
            _Rb_tree_rotate_left(__w, __root);
            __w = __x_parent->_M_left;
          }
          __w->_M_color = __x_parent->_M_color;
          __x_parent->_M_color = _S_rb_tree_black;
          if (__w->_M_left) __w->_M_left->_M_color = _S_rb_tree_black;
          _Rb_tree_rotate_right(__x_parent, __root);
          break;
        }
      }
    if (__x) __x->_M_color = _S_rb_tree_black;
  }
  return __y;
}
```



> 参考文档：[彻底理解红黑树（三）之删除](https://www.jianshu.com/p/84416644c080)



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