## 3. 迭代器Iterator

### 3.1 迭代器模式与STL迭代器

众所周知，迭代器iterator在C++中被认为是一种泛型指针，大多数情况下这些类类型都是对容器中元素指针的封装，最特殊的一种情况就是它本身就是指针（针对原始类型容器而言）。一般而言迭代器必然涉及到容器本身的特性，所以容器设计者对迭代器的实现负有责任。

<img src="../../image/iterator.jpg" alt="iterator" style="zoom: 67%;" />

为了能够深入了解一点STL对迭代器的实现，我们就不得不提大名鼎鼎四人帮《*Design Pattern*》一书中对于迭代器模式的描述。按照这种迭代器模式的设想，迭代器的具体实现大致有两部分组成：①抽象基类（接口类）Iterator和②具体迭代器ConcreteIterator。其中所有的容器（上面的Aggregate）都应该向外提供了生成迭代器的成员函数，它的函数声明会使用抽象基类Iterator，而函数具体实现并返回的却是ConcreteIterator，并借由C++的多态机制来实现到基类Iterator的转换，从而达到隐藏迭代器内部实现细节的目的。

但我们需要注意的是C++的多态必须借由指针或者引用才能实现，这意味着我们容器中生成迭代器的接口就必须返回的是迭代器Iterator类的指针或者引用，这显然与我们日常使用的STL容器、迭代器的情况相违背！在这种情况下Vector容器中`begin()`成员函数实现可能就是这个样子了：

```c++
template<typename T>
Iterator<T>* Vector<T>::begin() {
	return new VectorIterator(this->_data_start);
}
//或者以下面方式来实现：
template<typename T>
Iterator<T>& Vector<T>::begin() {
	return *(new VectorIterator(this->_data_start));
}
```

因此我们可以确定**STL的迭代器绝对不是按照设计模式中迭代器模式的描述来实现的**（设计模式中迭代器模式所认为的迭代器我觉得更适合于像Java这样的语言来实现）。实际的情况是，任何容器的迭代器ContainerIterator确实都继承了一个名叫iterator的基类，但这个基类中并没有定义任何纯虚函数/接口。且更重要的是，具体容器中常常使用的`begin()`等成员函数返回的并不是这个iterator基类的引用或者指针，相反它返回的就是具体容器迭代器ContainerIterator！只不过容器在内部通过typedef为具体迭代器取了一个iterator的别名罢了！

也就是说STL中的迭代器使用了继承这个语言特性，但并没有使用什么多态机制！因此我们可以想象容器Vector和其迭代器的实现可能是以如下的样貌呈现：

```cpp
template<typename T>
class Iterator {
	/*  ...  */
};

template<typename T>
class VectorIterator;

//容器的实现
template<typename T>
class Vector {
public:
	/*  ...  */
	typedef VectorIterator<T> Iterator;
	Iterator begin();
	Iterator end();
private:
	T *_data_start;
	T *_data_end;
	T *_storage_end;
};

//容器相关迭代器的实现
template<typename T>
class VectorIterator : public Iterator<T> {
public:
	explicit VectorIterator(T *p) :
		_data_pointer(p) {}
	/*  ...  */
	T *operator->() { return _data_pointer; }
	T &operator*() { return *_data_pointer; }
	T &operator++() {
		++_data_pointer;
		return *this;
	}
	const T operator++(int) {
		VectorIterator tmp(*this);
		++*this;
		return tmp;
	}
	/*  ...  */

private:
	T *_data_pointer;
};

//容器迭代器生成函数的实现
template<typename T>
typename Vector<T>::Iterator Vector<T>::begin() {
	return VectorIterator<T>(this->_data_start);
}

template<typename T>
typename Vector<T>::Iterator Vector<T>::end() {
	return VectorIterator<T>(this->_data_end);
}
```

文件[test.cpp](test.cpp)大致按照上面的想法实现了一个简单的Vector及其迭代器VectorIterator。



### ==3.2 迭代器特性类iterator_traits==

#### 3.2.1 引入迭代器特性类的背景

<img src="../../image/iterators.jpg" alt="iterators" style="zoom:50%;" />

在书本的最开始我们就指出过，迭代器是连接容器和算法的桥梁，算法通过迭代器来实现对容器的操作，这种操作可以是易变性的，也可以仅仅是对容器中元素的游历。所有算法的正常执行都必须基于相应的容器支持这种操作的前提之下才能得到保证，而且算法也必须有能力知道有关容器的相关信息，例如最常见的就是容器中元素的类型信息，如果一个累加算法不知道容器中元素的类型，那么显然这个算法无法正常执行。

但是由于算法只能接触到迭代器，而不能直接接触到容器，这就使得算法无法直接获知到有关容器的任何信息，因此在STL中迭代器这个沟通容器和算法的中间桥梁必须能够向算法提供一些信息的能力。一般这种信息都是借由迭代器本身的属性信息来提供，我们将这些属性信息称为**迭代器的关联类型信息（associated types，书中称为相应类别）**。例如算法需要知道容器元素的类型信息，那它就是在询问迭代器所指向的元素类型是什么。

因此为了使得迭代器提供这些相应类别的信息，每一个容器的迭代器都会提供如下5个类型成员：

```c++
template<typename T>
class VectorIterator : public Iterator<T> {
public:
	typedef T                           value_type;
	typedef T *                         pointer;
	typedef T &                         reference;
	typedef ptrdiff_t                   difference_type;
	typedef random_access_iterator_tag  iterator_category;
    /*  ...  */
};
```

这样借助迭代器提供的关联信息，算法就可以很容易的知道迭代器所指向的元素类型等信息。例如我们可以很容易地实现出上述的累加算法：

```c++
template<typename T>
typename VectorIterator<T>::value_type
accumulate(VectorIterator<T> beg, VectorIterator<T> end) {
	typename VectorIterator<T>::value_type res(0);
	for (; beg != end; ++beg)
		res += *beg;
	return res;
}
```

但我们必须要注意到：若传递给算法的实参是容器的迭代器（类类型就像上述的VectorIterator），那么这个算法显然能够正常运行；但是若提供的原始指针，那该怎么办？一种显而易见的方法就是将这个算法进行部分特例化，提供一个能够兼容原始指针的版本，如下：

```c++
template<typename T>
T accumulate(T *beg, T *end) {
	T res(0);
	for (; beg != end; ++beg)
		res += *beg;
	return res;
}
```

但这种方法必然会带来另外一个虽然简单解决但又显得比较烦人的问题：我们必须为每一个算法提供一个部分特例化的版本以支持原始指针（包括上面没有讲到的const T*）！因此我们必须提出一种新的解决方法，来提取出迭代器中的相应类别信息，则便是迭代器类型特性类iterator_traits。



#### 3.2.2 迭代器特性类概念

迭代器特性类iterator_traits的作用就是从迭代器（不仅包括类类型的迭代器，也包括原始指针）中提取出迭代器的相关类型信息。其本质就是在迭代器相应类别信息与算法之间加入一个间接层，以统一的方式（同时支持了特殊迭代器——原始指针）取出相应类别信息。而我们原先的方法就是让算法直接从迭代器中取出这些信息，但必须让每一个算法为原始指针编写一个部分特例化的版本！下图展示了这个改进的前后对比：

<img src="../../image/iterator_traits.jpg" alt="iterator_traits" style="zoom: 50%;" />

至于迭代器特性类iterator_traits的实现更是简单，如我们所见，在改进前提取迭代器相关类别信息的工作（直接从类类型迭代器提取和对原始指针进行部分特例化）是交由算法自己来完成；而在改进后这些工作都是完全由iterator_traits来负责，相应的直接提取和部分特例化工作都变成了iterator_traits的责任。

这部分的代码实现在源文件[stl_iterator_base.h](stl_iterator_base.h)的第108行：

```c++
template <class _Iterator>
struct iterator_traits {
  typedef typename _Iterator::iterator_category iterator_category;
  typedef typename _Iterator::value_type        value_type;
  typedef typename _Iterator::difference_type   difference_type;
  typedef typename _Iterator::pointer           pointer;
  typedef typename _Iterator::reference         reference;
};

template <class _Tp>
struct iterator_traits<_Tp*> {
  typedef random_access_iterator_tag iterator_category;
  typedef _Tp                         value_type;
  typedef ptrdiff_t                   difference_type;
  typedef _Tp*                        pointer;
  typedef _Tp&                        reference;
};

template <class _Tp>
struct iterator_traits<const _Tp*> {
  typedef random_access_iterator_tag iterator_category;
  typedef _Tp                         value_type;
  typedef ptrdiff_t                   difference_type;
  typedef const _Tp*                  pointer;
  typedef const _Tp&                  reference;
};
```

我们可以从上面的源代码看到iterator_traits类中内部正好记录着迭代器需要给算法提供的5个相应类别成员：`value_type`、`pointer`、`reference`、`difference_type`、`iterator_category`，而这些类型成员都是从迭代器（包括原始指针）中“榨取”出来的！因此这个类也有着一个响当当的外号：特性榨取机！

<img src="../../image/屏幕截图 2021-01-01 112729.png" alt="屏幕截图 2021-01-01 112729" style="zoom: 80%;" />

这样我们上面累加算法就可以通过如下的形式来得到实现了：

```c++
template<typename Iterator>
typename Iterator_traits<Iterator>::value_type
accumulate(Iterator beg,Iterator end){
	typename Iterator_traits<Iterator>::value_type res(0);
	for(;beg!=end;++beg)
		res+=*beg;
	return res;
}
```



#### 3.2.3 迭代器分类iterator_category



```c++
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

template <class _Category, class _Tp, class _Distance = ptrdiff_t,
          class _Pointer = _Tp*, class _Reference = _Tp&>
struct iterator {
  typedef _Category  iterator_category;
  typedef _Tp        value_type;
  typedef _Distance  difference_type;
  typedef _Pointer   pointer;
  typedef _Reference reference;
};


template <class _Iter>
inline typename iterator_traits<_Iter>::iterator_category
__iterator_category(const _Iter&)
{
  typedef typename iterator_traits<_Iter>::iterator_category _Category;
  return _Category();
}


template <class _Iter>
inline typename iterator_traits<_Iter>::difference_type*
__distance_type(const _Iter&)
{
  return static_cast<typename iterator_traits<_Iter>::difference_type*>(0);
}

template <class _Iter>
inline typename iterator_traits<_Iter>::value_type*
__value_type(const _Iter&)
{
  return static_cast<typename iterator_traits<_Iter>::value_type*>(0);
}

template <class _Iter>
inline typename iterator_traits<_Iter>::iterator_category
iterator_category(const _Iter& __i) { return __iterator_category(__i); }


template <class _Iter>
inline typename iterator_traits<_Iter>::difference_type*
distance_type(const _Iter& __i) { return __distance_type(__i); }
```



### 3.3 类型特性类__type_traits

