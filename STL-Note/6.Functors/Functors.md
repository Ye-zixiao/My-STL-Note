## 7. 函数对象

### 7.1 函数对象概念与适配性

函数对象指的是那些重载了`operator()`的类，其目的是为了搭配STL算法使用，为其提供自定义的策略。这部分的源码主要定义在[stl_function.h](stl_function.h)中。

为了应用后续的函数适配器，使得标准库中的函数对象具有适配能力。每一个C++标准库函数对象必须继承一元函数基类unary_function或者二元函数基类binary_function，而在这两个基类中则会定义出一些与函数对象相关的类别成员，这样函数适配器就可以从传入的标准库函数对象中提取出这些信息。

> 需要提醒的是：在C++11标准出现之后，函数对象和后面涉及到的适配器中有些组件已经被弃用了，不过至少在新版的g++相关头文件中unary_function、binary_function、证同\_Identiry、选择\_Select1st/ \_Select2nd还是存在的，投射我没找到。但这些东西在C++20之后必然是要被移除的，所以看看就行。



#### 7.1.1 unary_function

unary_function是一元仿函数基类，在旧式的C++版本中，任何一个允许函数适配器去适配的一元函数对象都需要继承这个基类。其所派生出来的函数对象称为一元函数对象。

例如函数适配器`not1()`就必须传递给它的函数对象继承unary_function，这样它就可以从传入的函数对象中提取出result_type、argument_type这些相关类型信息。

```c++
template <class _Arg, class _Result>
struct unary_function {
  typedef _Arg argument_type;
  typedef _Result result_type;
};
```



#### 7.1.2 binary_function

与上类似，binary_function是二元仿函数基类，在旧式的C++版本中，任何一个允许函数适配器取适配的二元函数对象都必须继承这个基类。其所派生出来的函数对象称为二元函数对象。

```c++
template <class _Arg1, class _Arg2, class _Result>
struct binary_function {
  typedef _Arg1 first_argument_type;
  typedef _Arg2 second_argument_type;
  typedef _Result result_type;
};  
```



### 7.2 常用函数对象

#### 7.2.1 算术类函数对象

算术类函数对象除了否定，都是二元仿函数：

- 加法：`plus<T>`
- 减法：`minus<T>`
- 乘法：`multiplies<T>`
- 除法：`divides<T>`
- 取模：`modulus<T>`
- 否定：`negate<T>`

> 顺便提下证同元素（不是下面的证同）。所谓“运算op的证同元素”指的是数值A若与该元素做op运算，会得到A自己。例如乘法的证同元素是1，加减法的证同元素是0.



#### 7.2.2 关系类函数对象

关系运算类函数对象包括如下几个：

- 等于：`equal_to<T>`
- 不等于：`not_equal_to<T>`
- 大于：`greater<T>`
- 大于或等于：`greater_equal<T>`
- 小于：`less<T>`
- 小于或等于：`less_equal<T>`



#### 7.2.3 逻辑类函数对象

逻辑运算类函数对象包括如下几个：

- 逻辑与：`logical_and<T>`
- 逻辑或：`logical_or<T>`
- 逻辑非：`logical_not<T>`



#### 7.2.4 位运算函数对象

位运算函数对象包括如下几个：

- 位与：`bit_and<T>`
- 位或：`bit_or<T>`
- 位异或：`bit_xor<T>`
- 位非：`bit_not<T>`



### 7.3 证同、选择和投射

证同、选择和投射都是几个非常简单的概念，且都不是C++标准中的成员，SGI STL将它们制作成了函数对象。加入它们的目的主要是为了引入一个抽象层，方便后续的使用。

#### 7.3.1 证同

其中证同指的是一模一样的返回传入参数：

```c++
template <class _Tp>
struct _Identity : public unary_function<_Tp,_Tp> {
  const _Tp& operator()(const _Tp& __x) const { return __x; }
};

template <class _Tp> struct identity : public _Identity<_Tp> {};
```



#### 7.3.2 选择

选择指的是从一个类型参数功能类似于pair的对中选择其中一个成员，可以first成员也可以是second成员，这样就造就了两个函数对象：

```c++
template <class _Pair>
struct _Select1st : public unary_function<_Pair, typename _Pair::first_type> {
  const typename _Pair::first_type& operator()(const _Pair& __x) const {
    return __x.first;
  }
};

template <class _Pair>
struct _Select2nd : public unary_function<_Pair, typename _Pair::second_type>
{
  const typename _Pair::second_type& operator()(const _Pair& __x) const {
    return __x.second;
  }
};

template <class _Pair> struct select1st : public _Select1st<_Pair> {};
template <class _Pair> struct select2nd : public _Select2nd<_Pair> {};
```



#### 7.3.3 投射

返回两个传入元素的其中一个，可以是第一个传入参数，也可以是第二个参数，同样的它造就了两个函数对象：

```c++
template <class _Arg1, class _Arg2>
struct _Project1st : public binary_function<_Arg1, _Arg2, _Arg1> {
  _Arg1 operator()(const _Arg1& __x, const _Arg2&) const { return __x; }
};

template <class _Arg1, class _Arg2>
struct _Project2nd : public binary_function<_Arg1, _Arg2, _Arg2> {
  _Arg2 operator()(const _Arg1&, const _Arg2& __y) const { return __y; }
};

template <class _Arg1, class _Arg2> 
struct project1st : public _Project1st<_Arg1, _Arg2> {};

template <class _Arg1, class _Arg2>
struct project2nd : public _Project2nd<_Arg1, _Arg2> {};
```

