#define _SCL_SECURE_NO_WARNINGS

#ifndef SET_H
#define BITSEQ_H

#include <memory>

#include <cassert>
#include <algorithm>
#include <iterator>

template <typename T>
class set;

template <typename T>
void swap(set<T>&, set<T>&) noexcept;

template <typename T>
struct set {

private:

	struct base_node {
		base_node* left;
		base_node* right;
		base_node *parent;

		base_node()
			: left(nullptr), right(nullptr), parent(nullptr)
		{}

		base_node(base_node* left, base_node* right, base_node * par)
			: left(left), right(right), parent(par)
		{}

		virtual ~base_node() = default;
	};
	struct node : base_node {
		T value;

		node(base_node* left, base_node* right, base_node* par, const T& data)
			: base_node(left, right, par), value(data) {}

		friend void swap(node *frst, node *sec) {
			node f_node(*frst);
			node sec_node(*sec);
			if (sec_node.parent != nullptr) {
				if (sec_node.parent->left == sec) {
					sec_node.parent->left = frst;
				}
				else {
					sec_node.parent->right = frst;
				}
			}
			if (f_node.parent != nullptr) {
				if (f_node.parent->left == frst) {
					f_node.parent->left = sec;
				}
				else {
					f_node.parent->right = sec;
				}
			}
			if (f_node.right != nullptr)
				f_node.right->parent = sec;

			if (sec_node.right != nullptr)
				sec_node.right->parent = frst;

			if (f_node.left != nullptr)
				f_node.left->parent = sec;

			if (sec_node.left != nullptr)
				sec_node.left->parent = frst;

			std::swap(sec->parent, frst->parent);
			std::swap(sec->right, frst->right);
			std::swap(sec->left, frst->left);
		}
	};

	base_node root_;
	base_node * root = &root_;

public:

	set() = default;
	set(set const&);
	set& operator=(set const& rhs) noexcept;
	~set();

	base_node * destroy(base_node * cur_node) {
		if (cur_node != nullptr) {
			if (cur_node->left == nullptr && cur_node->right == nullptr) {
				delete cur_node;
				return nullptr;
			}
			cur_node->left = destroy(cur_node->left);
			cur_node->right = destroy(cur_node->right);

			if (cur_node->left == nullptr && cur_node->right == nullptr) {
				delete cur_node;
				return nullptr;
			}
			return cur_node;
		}
		else {
			return cur_node;
		}
	}

	friend void swap<T>(set& lhs, set& rhs) noexcept;

	template <typename U>
	class Iterator {
	public:
		friend struct set;

		using difference_type = std::ptrdiff_t;
		using value_type = U;
		using pointer = U * ;
		using reference = U & ;
		using iterator_category = std::bidirectional_iterator_tag;

		template <typename V>
		Iterator(const Iterator<V>& other,
			typename std::enable_if<std::is_same<U, const V>::value>::type* = nullptr);

		pointer operator->() const {
			return &(static_cast<node *>(Ptr_))->value;
		}

		Iterator& operator++() {
			Ptr_ = next_node(Ptr_);
			return *this;
		}
		const Iterator operator++(int) {
			base_node *cur = Ptr_;
			Ptr_ = next_node(Ptr_);
			return iterator(cur);
		}
		Iterator& operator--() {
			Ptr_ = prev_node(Ptr_);
			return *this;
		}
		const Iterator operator--(int) {
			base_node *cur = Ptr_;
			Ptr_ = prev_node(Ptr_);
			return iterator(cur);
		}

		U& operator*() const;
		friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
			return lhs.Ptr_ == rhs.Ptr_;
		}
		friend bool operator!=(const Iterator& lhs, const Iterator& rhs) {
			return lhs.Ptr_ != rhs.Ptr_;
		}

	private:
		explicit Iterator(base_node* ptr);

		base_node * Ptr_;
	};

	using iterator = Iterator<T>;
	using const_iterator = Iterator<const T>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;
	reverse_iterator rbegin() { return reverse_iterator(end()); }
	const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
	reverse_iterator rend() { return reverse_iterator(begin()); }
	const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

	const_iterator find(T const &value) const {
		return find_dfs(root->left, value);
	}

	const_iterator lower_bound(T const &value) const {
		const_iterator result = end();
		base_node * cur = root->left;

		while (cur != nullptr) {
			T cur_value = static_cast<node*>(cur)->value;
			if (value < cur_value || (!(cur_value < value) && !(value < cur_value))) {
				if (result == end() || cur_value < *result) {
					result = const_iterator(cur);
				}
				cur = cur->left;
			}
			else {
				cur = cur->right;
			}
		}
		return result;
	}
	const_iterator upper_bound(T const &value) const {
		const_iterator result = end();
		base_node * cur = root->left;

		while (cur != nullptr) {
			T cur_value = static_cast<node*>(cur)->value;
			if (value < cur_value) {
				if (result == end() || cur_value < *result) {
					result = const_iterator(cur);
				}
				cur = cur->left;
			}
			else {
				cur = cur->right;
			}
		}
		return result;
	}

	bool empty() const {
		return root->left == nullptr;
	}

	void clear() {
		destroy(root->left);
	}

	std::pair<iterator, bool> insert(T const &value)
	{
		if (root->left != nullptr) {
			node * cur = static_cast<node *>(root->left);
			return ins(cur, value);
		}
		base_node * new_node = new node(nullptr, nullptr, root, value);
		root->left = new_node;
		return std::pair<iterator, bool>(iterator(new_node), true);
	}

	iterator erase(const_iterator pos) {
		return erase_impl(static_cast<node *>(pos.Ptr_));
	}

private:
	// === === === local functions === === ===

	static std::pair<iterator, bool> ins(base_node * cur, T const & value)
	{
		if (!(value < static_cast<node*>(cur)->value) && !(static_cast<node*>(cur)->value < value)) {
			return std::pair<iterator, bool>(iterator(cur), false);
		}
		if (value < static_cast<node*>(cur)->value) {
			if (cur->left != nullptr) {
				return ins(cur->left, value);
			}
			base_node * new_node = new node(nullptr, nullptr, cur, value);
			cur->left = new_node;
			return std::pair<iterator, bool>(iterator(new_node), true);			
		}
		else {
			if (cur->right != nullptr) {
				return ins(cur->right, value);
			}
			base_node *new_node = new node(nullptr, nullptr, cur, value);
			cur->right = new_node;
			return std::pair<iterator, bool>(iterator(new_node), true);
		}
	}

	const_iterator find_dfs(base_node * cur, T const &val) const {
		if (cur == nullptr) {
			return end();
		}
		T cur_value = static_cast<node*>(cur)->value;
		if (!(cur_value < val) && !(val < cur_value)) {
			return const_iterator(cur);
		}
		if (val < static_cast<node*>(cur)->value) {
			return find_dfs(cur->left, val);
		}
		else {
			return find_dfs(cur->right, val);
		}
	}

	iterator erase_impl(base_node * erased_node) {
		iterator tmp_it(erased_node);
		tmp_it++;
		if (erased_node->left == nullptr) {
			if (erased_node->parent != nullptr) {
				if (erased_node->parent->right == erased_node) {
					erased_node->parent->right = erased_node->left;
				}
				else {
					erased_node->parent->left = erased_node->left;
				}
			}
			if (erased_node->left != nullptr) {
				erased_node->left->parent = erased_node->parent;
			}
			delete erased_node;
		}
		else if (erased_node->left == nullptr) {
			if (erased_node->parent != nullptr) {
				if (erased_node->parent->right == erased_node) {
					erased_node->parent->right = erased_node->right;
				}
				else {
					erased_node->parent->left = erased_node->right;
				}
			}
			erased_node->right->parent = erased_node->parent;
			delete erased_node;
		}
		else {
			base_node *goer = erased_node->left;
			while (goer->right != nullptr) {
				goer = goer->right;
			}
			swap(static_cast<node *>(goer), static_cast<node *>(erased_node));
			erase_impl(erased_node);
		}
		return tmp_it;
	}

	static base_node * minimum(base_node * cur) {
		if (cur->left == nullptr)
			return cur;
		return minimum(cur->left);
	}

	static base_node * maximum(base_node * cur) {
		if (cur->right == nullptr)
			return cur;
		return minimum(cur->right);
	}

	static base_node * next_node(base_node * cur) {
		if (cur->right != nullptr)
			return minimum(cur->right);

		base_node *y = cur->parent;
		while (y != nullptr && cur == y->right) {
			cur = y;
			y = y->parent;
		}
		return y;
	}

	static base_node * prev_node(base_node * cur) {
		if (cur->left != nullptr)
			return maximum(cur->left);

		base_node * y = cur->parent;
		while (y != nullptr && cur == y->left) {
			cur = y;
			y = y->parent;
		}
		return y;
	}

	base_node * real_root() {
		return root->left;
	}
};

template <typename T>
void swap(set<T>& lhs, set<T>& rhs) noexcept {
	typename set<T>::base_node * lp = lhs.root->left;
	typename set<T>::base_node * rp = rhs.root->left;
	std::swap(lhs.root, rhs.root);
	std::swap(lp, rp);
	if (lp) lp->parent = rhs.root;
	if (rp) rp->parent = lhs.root;
}

// ================================================================
// ================================================================

template <typename T>
template <typename U>
set<T>::Iterator<U>::Iterator(typename set<T>::base_node* ptr)
	: Ptr_(ptr)
{}

template <typename T>
template <typename U>
template <typename V>
set<T>::Iterator<U>::Iterator(const Iterator<V> &other,
	typename std::enable_if<std::is_same<U, const V>::value>::type*)
	: Ptr_(other.Ptr_)
{}

template<typename T>
template<typename U>
U& set<T>::Iterator<U>::operator*() const {
	return static_cast<node*>(Ptr_)->value;
}

// =================================================================
// =================================================================

template<typename T>
set<T>::set(const set &other) : set() {
}

template<typename T>
set<T>& set<T>::operator=(set const& rhs) noexcept {
	set tmp = rhs;
	swap(*this, tmp);
	return *this;
}

template<typename T>
set<T>::~set() {
	destroy(real_root());
}

template<typename T>
typename set<T>::iterator set<T>::begin() {
	if (root->left) {
		return iterator(minimum(root));
	}
	return iterator(root);
}
template<typename T>
typename set<T>::const_iterator set<T>::begin() const {
	if (root->left) {
		return const_iterator(minimum(root));
	}
	return const_iterator(root);
}
template<typename T>
typename set<T>::iterator set<T>::end() {
	if (root->left) {
		return iterator(maximum(root));
	}
	return iterator(root);
}
template<typename T>
typename set<T>::const_iterator set<T>::end() const {
	if (root->left) {
		return const_iterator(maximum(root));
	}
	return const_iterator(root);
}

#endif // SET_H
