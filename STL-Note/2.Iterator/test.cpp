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
		_data_start(new int[il.size()]) {
		_data_end = _data_start + il.size();
		std::copy(il.begin(), il.end(), _data_start);
	}
	~Vector() {
		delete _data_start;
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

int main() {
	Vector<int> ivec{1, 2, 23, 34, 4};
	for (auto iter = ivec.begin(); iter != ivec.end(); ++iter)
		std::cout << *iter << std::endl;
	return 0;
}