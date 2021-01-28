//
// Created by yexin on 2021/1/19.
//

#ifndef UNTITLED_AVL_H
#define UNTITLED_AVL_H

#include <initializer_list>
#include <algorithm>
#include <exception>
#include <iostream>

template<typename T>
struct TreeNode {
	TreeNode<T> *lchild, *rchild;
	int balance_factor;
	T value;
};

template<typename T>
inline int height(TreeNode<T> *h) {
	if (!h) return 0;
	return std::max(height(h->lchild), height(h->rchild)) + 1;
}

template<typename T>
inline int height_delta(TreeNode<T> *h) {
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
void inorder(const TreeNode<T> *h) {
	if (!h) return;
	if (h->lchild) inorder(h->lchild);
	std::cout << h->value << ' ';
	if (h->rchild) inorder(h->rchild);
}

template<typename T>
void rinorder(const TreeNode<T> *h) {
	if (!h) return;
	if (h->rchild) rinorder(h->rchild);
	std::cout << h->value << ' ';
	if (h->lchild) rinorder(h->lchild);
}

template<typename T>
class AVLTree {
public:
	enum { EH, LH, RH };

	AVLTree() : root(nullptr) {}
	AVLTree(const std::initializer_list<T> &il) :
		root(nullptr) { for (const auto &elem:il) insert(elem); }
	AVLTree(const AVLTree &rhs) = delete;
	AVLTree(AVLTree &&rhs) noexcept:
		root(std::move(rhs.root)) { rhs.root = nullptr; }
	~AVLTree() { clear(); }

	AVLTree &operator=(const AVLTree &rhs) = delete;
	AVLTree &operator=(AVLTree &&rhs) noexcept {
		root = std::move(rhs.root);
		rhs.root = nullptr;
	}

public:
	const TreeNode<T> *find(const T &val) { return search(root, val); }
	bool contains(const T &val) const { return find(val) != nullptr; }

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
	const T &min() const;
	const T &max() const;

private:
	static TreeNode<T> *left_rotate(TreeNode<T> *h);
	static TreeNode<T> *right_rotate(TreeNode<T> *h);
	static TreeNode<T> *left_right_rotate(TreeNode<T> *h);
	static TreeNode<T> *right_left_rotate(TreeNode<T> *h);

private:
	void clear(TreeNode<T> *h);
	TreeNode<T> *insert(TreeNode<T> *h, const T &val);

private:
	TreeNode<T> *create_node(const T &val);
	void destroy_node(TreeNode<T> *node) { delete node; }

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
	node->balance_factor = EH;
	node->value = val;
	return node;
}

template<typename T>
inline const T &AVLTree<T>::min() const {
	if (!root) throw std::exception();
	TreeNode<T> *t = root;
	while (t->lchild) t = t->lchild;
	return t->value;
}

template<typename T>
inline const T &AVLTree<T>::max() const {
	if (!root) throw std::exception();
	TreeNode<T> *t = root;
	while (t->rchild) t = t->rchild;
	return t->value;
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
	if (!h)return create_node(val);
	if (val < h->value) {
		if (h->balance_factor == RH) h->balance_factor = EH;
		else if (h->balance_factor == EH) h->balance_factor = LH;
		else if (h->balance_factor == LH) {}
		else throw std::exception();
		h->lchild = insert(h->lchild, val);
	}
	if (val >= h->value) {//大于等于表示可以重复插入相同的数据
		if (h->balance_factor == LH) h->balance_factor = EH;
		else if (h->balance_factor == EH) h->balance_factor = RH;
		else if (h->balance_factor == RH) {}
		else throw std::exception();
		h->rchild = insert(h->rchild, val);
	}

	if (std::abs(height_delta(h)) >= 2) {
		//“左左”不平衡情况，使用单右旋
		if (h->balance_factor == LH && h->lchild->balance_factor == LH) {
			h->balance_factor = EH;
			h->lchild->balance_factor = EH;
			return right_rotate(h);
		}
			//“左右”不平衡情况，使用单左旋+单右旋组合
		else if (h->balance_factor == LH && h->lchild->balance_factor == RH) {
			if (h->lchild->rchild->balance_factor == LH) {
				h->balance_factor = RH;
				h->lchild->balance_factor = EH;
			}
			else if (h->lchild->rchild->balance_factor == RH) {
				h->balance_factor = EH;
				h->lchild->balance_factor = LH;
			}
			else {
				h->balance_factor = EH;
				h->lchild->balance_factor = EH;
			}
			h->lchild->rchild->balance_factor = EH;
			return left_right_rotate(h);
		}
			//“右左”不平衡情况，使用单右旋+单左旋组合
		else if (h->balance_factor == RH && h->rchild->balance_factor == LH) {
			if (h->rchild->lchild->balance_factor == RH) {
				h->balance_factor = LH;
				h->rchild->balance_factor = EH;
			}
			else if (h->rchild->lchild->balance_factor == LH) {
				h->balance_factor = EH;
				h->rchild->balance_factor = RH;
			}
			else {
				h->balance_factor = EH;
				h->rchild->balance_factor = EH;
			}
			h->rchild->lchild->balance_factor = EH;
			return right_left_rotate(h);
		}
			//“右右”不平衡情况，使用单左旋
		else if (h->balance_factor == RH && h->rchild->balance_factor == RH) {
			h->balance_factor = EH;
			h->rchild->balance_factor = EH;
			return left_rotate(h);
		}
	}
	return h;
}

#endif //UNTITLED_AVL_H
