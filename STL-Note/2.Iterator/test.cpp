#include <iostream>
#include <initializer_list>

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
class VectorIterator : public Iterator<T> {
public:
	typedef T value_type;
//	typedef T *                         pointer;
//	typedef T &                         reference;
//	typedef ptrdiff_t                   difference_type;
//	typedef random_access_iterator_tag  iterator_category;

	explicit VectorIterator(T *p) :
		_data_pointer(p) {}
	/*  ...  */
	T *operator->() { return _data_pointer; }
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
	/*  ...  */
};

template<typename T>
struct Iterator_traits<T *> {
	typedef T value_type;
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

int main() {
	Vector<int> vec{1, 2, 23, 34, 4};
	for (auto iter = vec.begin(); iter != vec.end(); ++iter)
		std::cout << *iter << ' ';
	std::cout << std::endl;
	std::cout << "vec accumulate: " << accumulate(vec.begin(), vec.end()) << std::endl;

	int arr[] = {1, 2, 2, 34, 46, 7};
	std::cout << "arr accumulate: " << accumulate(std::begin(arr), std::end(arr)) << std::endl;

	return 0;
}