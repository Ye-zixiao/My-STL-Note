//
// Created by yexin on 2021/1/19.
//

#ifndef UNTITLED_AVL_H
#define UNTITLED_AVL_H

#include <initializer_list>
#include <algorithm>
#include <exception>

#define EH 0
#define LH 1
#define RH 2

template<typename T>
struct TreeNode {
	TreeNode<T> *lchild, *rchild;
	int flag;
	T value;
};

template<typename T>
int height(TreeNode<T> *h) {
	if (!h) return 0;
	return std::max(height(h->lchild), height(h->rchild)) + 1;
}

template<typename T>
int height_delta(TreeNode<T> *h) {
	if (!h) return 0;
	return height(h->lchild) - height(h->rchild);
}

template<typename T>
const TreeNode<T> *search(const TreeNode<T> *h, const T &val) {
	if (!h) return nullptr;
	if (val < h->value) return search(h->lchild, val);
	else if (val > h->value) return search(h->rchild, val);
	return h;
}

template<typename T>
class AVLTree {
public:
	using value_type = T;
	using reference = T &;
	using const_reference = const T &;
	using pointer = T *;
	using const_pointer = const T *;
	using node_type = TreeNode<T>;
	using node_pointer = TreeNode<T> *;

	AVLTree() : root(nullptr) {}
	AVLTree(const std::initializer_list<T> &il) : root(nullptr) {
		for (const auto &elem:il) insert(elem);
	}
	AVLTree(const AVLTree &rhs) = delete;
	AVLTree(AVLTree &&rhs) noexcept: root(std::move(rhs.root)) {
		rhs.root = nullptr;
	}
	~AVLTree() { clear(); }

	AVLTree &operator=(const AVLTree &rhs) = delete;
	AVLTree &operator=(AVLTree &&rhs) noexcept {
		root = std::move(rhs.root);
		rhs.root = nullptr;
	}
	bool iscontain(const T &val) const { return search(root, val) != nullptr; }
	void insert(const T &val) { root = insert(root, val); }
	void clear() {
		clear(root);
		root = nullptr;
	}
	void show() const {
		inorder(root);
		std::cout << std::endl;
	}
	void rshow() const {
		rinorder(root);
		std::cout << std::endl;
	}
	[[nodiscard]] int height() const { return ::height(root); }
	[[nodiscard]] T min() const;
	[[nodiscard]] T max() const;

private:
	TreeNode<T> *left_rotate(TreeNode<T> *h);
	TreeNode<T> *right_rotate(TreeNode<T> *h);
	TreeNode<T> *left_right_rotate(TreeNode<T> *h);
	TreeNode<T> *right_left_rotate(TreeNode<T> *h);

private:
	TreeNode<T> *create_node(const T &val);
	void destroy_node(TreeNode<T> *node) { delete node; }

	void inorder(TreeNode<T> *h) const;
	void rinorder(TreeNode<T> *h) const;
	void clear(TreeNode<T> *h);
	TreeNode<T> *insert(TreeNode<T> *h, const T &val);

private:
	TreeNode<T> *root;
};

//单左旋，针对“右右”不平衡情况
template<typename T>
TreeNode<T> *AVLTree<T>::left_rotate(TreeNode<T> *h) {
	TreeNode<T> *x = h->rchild;
	h->rchild = x->lchild;
	x->lchild = h;
	return x;
}

//单右旋，针对“左左”不平衡情况
template<typename T>
TreeNode<T> *AVLTree<T>::right_rotate(TreeNode<T> *h) {
	TreeNode<T> *x = h->lchild;
	h->lchild = x->rchild;
	x->rchild = h;
	return x;
}

//单左旋+单右旋，针对“左右”不平衡情况
template<typename T>
TreeNode<T> *AVLTree<T>::left_right_rotate(TreeNode<T> *h) {
	h->lchild = left_rotate(h->lchild);
	return right_rotate(h);
}

//单右旋+单左旋，针对“右左”不平衡情况
template<typename T>
TreeNode<T> *AVLTree<T>::right_left_rotate(TreeNode<T> *h) {
	h->rchild = right_rotate(h->rchild);
	return left_rotate(h);
}

template<typename T>
TreeNode<T> *AVLTree<T>::create_node(const T &val) {
	auto *node = new TreeNode<T>();
	node->lchild = node->rchild = nullptr;
	node->flag = EH;
	node->value = val;
	return node;
}

template<typename T>
T AVLTree<T>::min() const {
	if (!root) throw std::exception();
	TreeNode<T> *t = root;
	while (t->lchild) t = t->lchild;
	return t->value;
}

template<typename T>
T AVLTree<T>::max() const {
	if (!root) throw std::exception();
	TreeNode<T> *t = root;
	while (t->rchild) t = t->rchild;
	return t->value;
}

template<typename T>
void AVLTree<T>::inorder(TreeNode<T> *h) const {
	if (!h) return;
	if (h->lchild) inorder(h->lchild);
	std::cout << h->value << ' ';
	if (h->rchild) inorder(h->rchild);
}

template<typename T>
void AVLTree<T>::rinorder(TreeNode<T> *h) const {
	if (!h) return;
	if (h->rchild) rinorder(h->rchild);
	std::cout << h->value << ' ';
	if (h->lchild) rinorder(h->lchild);
}

template<typename T>
void AVLTree<T>::clear(TreeNode<T> *h) {
	if (!h) return;
	if (h->lchild) clear(h->lchild);
	if (h->rchild) clear(h->rchild);
	destroy_node(h);
}

template<typename T>
TreeNode<T> *AVLTree<T>::insert(TreeNode<T> *h, const T &val) {
	if (!h) return create_node(val);
	if (val < h->value) {
		if (h->flag == RH) h->flag = EH;
		else if (h->flag == EH) h->flag = LH;
		else if (h->flag == LH) {}
		else throw std::exception();
		h->lchild = insert(h->lchild, val);
	}
	if (val > h->value) {
		if (h->flag == LH) h->flag = EH;
		else if (h->flag == EH) h->flag = RH;
		else if (h->flag == RH) {}
		else throw std::exception();
		h->rchild = insert(h->rchild, val);
	}

	if (std::abs(height_delta(h)) >= 2) {
		//“左左”不平衡情况，使用单右旋
		if (h->flag == LH && h->lchild->flag == LH) {
			h->flag = EH;
			h->lchild->flag = EH;
			return right_rotate(h);
		}
			//“左右”不平衡情况，使用单左旋+单右旋组合
		else if (h->flag == LH && h->lchild->flag == RH) {
			if (h->lchild->rchild->flag == LH) {
				h->flag = RH;
				h->lchild->flag = EH;
			}
			else if (h->lchild->rchild->flag == RH) {
				h->flag = EH;
				h->lchild->flag = LH;
			}
			else throw std::exception();
			h->lchild->rchild->flag = EH;
			return left_right_rotate(h);
		}
			//“右左”不平衡情况，使用单右旋+单左旋组合
		else if (h->flag == RH && h->rchild->flag == LH) {
			if (h->rchild->lchild->flag == RH) {
				h->flag = LH;
				h->rchild->flag = EH;
			}
			else if (h->rchild->lchild->flag == LH) {
				h->flag = EH;
				h->rchild->flag = RH;
			}
			else throw std::exception();
			h->rchild->lchild->flag = EH;
			return right_left_rotate(h);
		}
			//“右右”不平衡情况，使用单左旋
		else if (h->flag == RH && h->rchild->flag == RH) {
			h->flag = EH;
			h->rchild->flag = EH;
			return left_rotate(h);
		}
	}
	return h;
}

#endif //UNTITLED_AVL_H
