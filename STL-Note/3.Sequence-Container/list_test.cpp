//
// Created by Ye-zixiao on 2021/1/3.
//

#include <iostream>
#include <iterator>
#include <algorithm>
#include <initializer_list>
#include <memory>

//链表结点
struct list_node_base {
	list_node_base *prev, *next;
};

template<typename T>
struct list_node : public list_node_base {
	T data;
};

//链表迭代器
struct list_iterator_base {
	using iterator_category = std::bidirectional_iterator_tag;
	using difference_type = ptrdiff_t;

	explicit list_iterator_base(list_node_base *_p = nullptr) :
		M_node(_p) {}

	bool operator==(const list_iterator_base &rhs) const {
		return this->M_node == rhs.M_node;
	}

	bool operator!=(const list_iterator_base &rhs) const {
		return !this->operator==(rhs);
	}

	//M_node指针是公开的
	list_node_base *M_node;

protected:
	void M_incr() { M_node = M_node->next; }
	void M_decr() { M_node = M_node->prev; }
};

template<typename T>
struct list_iterator : public list_iterator_base {
	using value_type = T;
	using reference = T &;
	using pointer = T *;
	using self = list_iterator<T>;

	list_iterator() = default;
	explicit list_iterator(list_node<T> *p) :
		list_iterator_base(p) {}

	self &operator++() {
		this->M_incr();
		return *this;
	}

	const self operator++(int) {
		self tmp = *this;
		++*this;
		return tmp;
	}

	self &operator--() {
		this->M_decr();
		return *this;
	}

	const self operator--(int) {
		self tmp = *this;
		--*this;
		return tmp;
	}

	reference operator*() { return static_cast<list_node<T> *>(M_node)->data; }
	pointer operator->() { return &static_cast<list_node<T> *>(M_node)->data; }
};

//链表的实现
template<typename T, typename Alloc>
class list_base {
public:
	using value_type = T;
	using reference = T &;
	using pointer = T *;

	list_base() {
		node = get_node();
		node->next = node;
		node->prev = node;
	}
	~list_base() {
		clear();
		put_node(node);
	}

	void clear();

protected:
	list_node<T> *node;
	Alloc M_alloc;

	list_node<T> *get_node() {
		return M_alloc.allocate(1);
	}

	void put_node(list_node<T> *_p) {
		M_alloc.deallocate(_p, 1);
	}
};

template<typename T, typename Alloc>
void list_base<T, Alloc>::clear() {
	list_node<T> *pnode = (list_node<T> *) this->node->next;
	while (pnode != node) {
		list_node<T> *tmp = pnode;
		pnode = (list_node<T> *) pnode->next;
		std::destroy_at(&tmp->data);//析构
		put_node(tmp);//销毁
	}
	node->next = node;
	node->prev = node;
}

template<typename T, typename Alloc=std::allocator<list_node<T>>>
class list : protected list_base<T, Alloc> {
public:
	using _Base = list_base<T, Alloc>;
	using iterator = list_iterator<T>;
	using size_type = size_t;

	list() : _Base() {}
	list(const std::initializer_list<T> &il) : _Base() {
		for (const auto &elem:il)
			push_back(elem);
	}
	~list() = default;

	iterator begin() { return iterator((list_node<T> *) this->node->next); }
	iterator end() { return iterator(this->node); }
	size_type size() { return std::distance(begin(), end()); }
	bool empty() const { return end()->next == begin(); }

	void swap(list &rhs) { std::swap(this->node, rhs.node); }
	void insert(iterator iter, const T &val);
	void push_back(const T &val);
	void splice(iterator iter, list &lst);
//	void sort();

protected:
	void transfer(iterator position, iterator first, iterator last);

	list_node<T> *create_node(const T &x) {
		list_node<T> *tmp = _Base::get_node();
		_Base::M_alloc.construct(&tmp->data, x);
		return tmp;
	}
};

template<typename T, typename Alloc>
void list<T, Alloc>::insert(iterator iter, const T &val) {
	list_node<T> *tmp = create_node(val);
	tmp->next = iter.M_node;
	tmp->prev = iter.M_node->prev;
	iter.M_node->prev->next = tmp;
	iter.M_node->prev = tmp;
}

template<typename T, typename Alloc>
void list<T, Alloc>::push_back(const T &val) {
	insert(end(), val);
}

template<typename T, typename Alloc>
void list<T, Alloc>::transfer(iterator position, iterator first, iterator last) {
	last.M_node->prev->next = position.M_node;
	first.M_node->prev->next = last.M_node;
	position.M_node->prev->next = first.M_node;

	list_node_base *_tmp = position.M_node->prev;
	position.M_node->prev = last.M_node->prev;
	last.M_node->prev = first.M_node->prev;
	first.M_node->prev = _tmp;
}

template<typename T, typename Alloc>
void list<T, Alloc>::splice(iterator iter, list &lst) {
	transfer(iter, lst.begin(), lst.end());
}

int main() {
	list<int> mylist{1, 34, 3, 45, 5, 3};
	list<int> mylist2{34, 5, 6, 9};
	std::cout << "currlist size: " << mylist.size() << std::endl;
	mylist.splice(mylist.begin(), mylist2);
	for (const int &elem:mylist)
		std::cout << elem << ' ';
	std::cout << std::endl;
	std::cout << "currlist size: " << mylist.size() << std::endl;

	return 0;
}