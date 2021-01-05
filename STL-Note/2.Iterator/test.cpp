#include <iostream>
#include <initializer_list>

struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

//template<typename T>
//class Iterator {
//	/*  ...  */
//};

template<typename Category, typename T, typename Distance=ptrdiff_t,
	typename Pointer=T *, typename Reference=T &>
struct Iterator {
	typedef Category iterator_category;
	typedef Distance difference_type;
	typedef T value_type;
	typedef Pointer pointer;
	typedef Reference reference;
};


template<typename T>
class VectorIterator;

//容器的实现
template<typename T>
class Vector {
public:
	Vector(std::initializer_list<T> il) :
		_data_start(new int[il.size()]), _data_end(_data_start + il.size()) {
		std::copy(il.begin(), il.end(), _data_start);
	}
	~Vector() {
		delete[] _data_start;
	}
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
class VectorIterator: public Iterator<random_access_iterator_tag, T> {
public:
//	typedef T value_type;
//	typedef T *                         pointer;
//	typedef T &                         reference;
//	typedef ptrdiff_t difference_type;
//	typedef random_access_iterator_tag iterator_category;

	explicit VectorIterator(T *p) :
		_data_pointer(p) {}
	/*  ...  */
	T* operator->() { return _data_pointer; }
	T &operator*() { return *_data_pointer; }
	VectorIterator &operator++() {
		++_data_pointer;
		return *this;
	}
	const VectorIterator operator++(int) {
		VectorIterator tmp(*this);
		++*this;
		return tmp;
	}
	VectorIterator &operator+=(ptrdiff_t n) {
		_data_pointer += n;
		return *this;
	}
	/*  ...  */

private:
	T *_data_pointer;
};



template<typename T>
bool operator!=(VectorIterator<T> lhs, VectorIterator<T> rhs) {
	return lhs.operator->() != rhs.operator->();
}

//容器迭代器生成函数的实现
template<typename T>
typename Vector<T>::Iterator Vector<T>::begin() {
	return VectorIterator<T>(this->_data_start);
}

template<typename T>
typename Vector<T>::Iterator Vector<T>::end() {
	return VectorIterator<T>(this->_data_end);
}


//template<typename T>
//typename VectorIterator<T>::value_type
//accumulate(VectorIterator<T> beg, VectorIterator<T> end) {
//	typename VectorIterator<T>::value_type res(0);
//	for (; beg != end; ++beg)
//		res += *beg;
//	return res;
//}
//
//template<typename T>
//T accumulate(T *beg, T *end) {
//	T res(0);
//	for (; beg != end; ++beg)
//		res += *beg;
//	return res;
//}


//迭代器特性类实现
template<typename T>
struct Iterator_traits {
	typedef typename T::value_type value_type;
	typedef typename T::iterator_category iterator_category;
	/*  ...  */
};

template<typename T>
struct Iterator_traits<T *> {
	typedef T value_type;
	typedef random_access_iterator_tag iterator_category;
	/*  ...  */
};

//累加算法实现
template<typename Iterator>
typename Iterator_traits<Iterator>::value_type
accumulate(Iterator beg, Iterator end) {
	typename Iterator_traits<Iterator>::value_type res(0);
	for (; beg != end; ++beg)
		res += *beg;
	return res;
}

//迭代器步进算法实现
template<typename InputIterator, typename Dist>
void advance(InputIterator &iter, Dist n, input_iterator_tag) {
	while (n--) iter++;
}

template<typename RandomAccessIterator, typename Dist>
void advance(RandomAccessIterator &iter, Dist n, random_access_iterator_tag) {
	iter += n;
}

template<typename Iterator, typename Dist>
void advance(Iterator &iter, Dist n) {
	using iter_category = typename Iterator_traits<Iterator>::iterator_category;
	advance(iter, n, iter_category());
}



int main() {
	Vector<int> vec{1, 2, 23, 34, 4};
	for (auto iter = vec.begin(); iter != vec.end(); ++iter)
		std::cout << *iter << ' ';
	std::cout << std::endl;
	std::cout << "vec accumulate: " << accumulate(vec.begin(), vec.end()) << std::endl;

	int arr[] = {1, 2, 2, 34, 46, 7};
	std::cout << "arr accumulate: " << accumulate(std::begin(arr), std::end(arr)) << std::endl;

	auto iter = vec.begin();
	std::cout << *iter << std::endl;
	advance(iter, 2);
	std::cout << *iter << std::endl;

	VectorIterator<int>::value_type val;

	return 0;
}

