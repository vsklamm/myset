#define _SCL_SECURE_NO_WARNINGS

#ifndef SET_H
#define BITSEQ_H

#include <memory>
#include <cassert>
#include <algorithm>
#include <iterator>

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

		base_node(base_node * parent)
			: left(nullptr), right(nullptr), parent(parent)
		{}

		base_node(base_node* left, base_node* right, base_node * par)
			: left(left), right(right), parent(par)
		{}

		virtual ~base_node() {
			delete left;
			delete right;
		}
	};
	struct node : base_node {
		T value;

		node(T const& value)
			: set::base_node(), value(value)
		{}

		node(base_node * parent, T const& value)
			: base_node(parent), value(value)
		{}

		node(base_node* left, base_node* right, base_node* par, const T& data)
			: base_node(left, right, par), value(data)
		{}
	};

	base_node root;

	base_node * get_root() const;

public:

	set() : root() {};
	set(set const &other);
	set& operator=(set rhs) noexcept;
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

	void swap(set<T> &other) noexcept;

	/*
	* === === === === === === === === === === === === === ===
	*                      I T E R A T O R S
	* === === === === === === === === === === === === === === ===
	*/

	template <typename U>
	class Iterator {
	public:
		friend struct set;

		using difference_type = std::ptrdiff_t;
		using value_type = U;
		using pointer = U * ;
		using reference = U & ;
		using iterator_category = std::bidirectional_iterator_tag;

		Iterator() : Ptr_(nullptr)
		{}

		explicit Iterator(base_node* Ptr_) : Ptr_(Ptr_)
		{}

		template <typename V>
		Iterator(Iterator<V> const& other);

		Iterator& operator=(Iterator const& other) {
			Ptr_ = other.Ptr_;
			return *this;
		}

		pointer operator->() const {
			return &(static_cast<node *>(Ptr_))->value;
		}

		reference operator*() const {
			return (static_cast<node*>(Ptr_))->value;
		}

		Iterator& operator++() {
			Ptr_ = next_node(Ptr_);
			return *this;
		}

		Iterator operator++(int) {
			auto tmp(*this);
			++(*this);
			return tmp;
		}

		Iterator& operator--() {
			if (Ptr_->left) {
				Ptr_ = Ptr_->left;
				while (Ptr_->right)
					Ptr_ = Ptr_->right;
			}
			else {
				while (Ptr_->parent->left == Ptr_)
					Ptr_ = Ptr_->parent;
				Ptr_ = Ptr_->parent;
			}
			return *this;
		}

		Iterator operator--(int) {
			auto tmp(*this);
			--(*this);
			return tmp;
		}

		friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
			return lhs.Ptr_ == rhs.Ptr_;
		}
		friend bool operator!=(const Iterator& lhs, const Iterator& rhs) {
			return lhs.Ptr_ != rhs.Ptr_;
		}

	private:

		base_node * Ptr_;
	};

	using iterator = Iterator<const T>;
	using const_iterator = Iterator<const T>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	iterator begin() const;
	iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;
	reverse_iterator rbegin() const { return reverse_iterator(end()); }
	reverse_iterator rend() const { return reverse_iterator(begin()); }
	const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
	const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

	/*
	 * === === === === === === === === === === === === === === ===
	 *                 C O M M O N  M E T H O D S
	 * === === === === === === === === === === === === === === ===
	 */

	const_iterator find(T const &value) const {
		return find_dfs(root.left, value);	
	}

	const_iterator lower_bound(T const &value) const {
		const_iterator result = end();
		base_node * current = root.left;

		while (current != nullptr) {
			T cur_value = static_cast<node*>(current)->value;
			if (value < cur_value || (!(cur_value < value) && !(value < cur_value))) {
				if (result == end() || cur_value < *result) {
					result = const_iterator(current);
				}
				current = current->left;
			}
			else {
				current = current->right;
			}
		}
		return result;
	}
	const_iterator upper_bound(T const &value) const {
		const_iterator result = end();
		base_node * cur = root.left;

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
		return root.left == nullptr;
	}

	void clear() {
		//destroy(root.left);
		delete root.left;
		root.left = nullptr;
	}

	std::pair<iterator, bool> insert(T const &value)
	{
		if (root.left == nullptr) {
			root.left = new node(&root, value);
			return { iterator(root.left), true };
		}
		base_node * cur = root.left;
		while (true) {
			if (static_cast<node*>(cur)->value == value)
				return { iterator(cur), false };
			if (static_cast<node*>(cur)->value > value) {
				if (cur->left)
					cur = cur->left;
				else {
					cur->left = new node(cur, value);
					return { iterator(cur->left), true };
				}
			}
			if (static_cast<node*>(cur)->value < value) {
				if (cur->right)
					cur = cur->right;
				else {
					cur->right = new node(cur, value);
					return { iterator(cur->right), true };
				}
			}
		}
	}

	iterator erase(const_iterator pos) {
		iterator ret = pos;
		++ret;

		if (pos.Ptr_->left && pos.Ptr_->right) {
			auto next = pos;
			++next;
			const_iterator cur = detach(next);

			if (pos.Ptr_->parent->left == pos.Ptr_)
				pos.Ptr_->parent->left = cur.Ptr_;
			else
				pos.Ptr_->parent->right = cur.Ptr_;

			if (pos.Ptr_->right)
				pos.Ptr_->right->parent = cur.Ptr_;
			if (pos.Ptr_->left)
				pos.Ptr_->left->parent = cur.Ptr_;

			cur.Ptr_->parent = pos.Ptr_->parent;
			cur.Ptr_->left = pos.Ptr_->left;
			cur.Ptr_->right = pos.Ptr_->right;
		}
		else {
			detach(pos);
		}
		pos.Ptr_->right = pos.Ptr_->left = nullptr;
		delete pos.Ptr_;
		return ret;
	}

private:
	/*
	* === === === === === === === === === === === === === === ===
	*                L O C A L  O P E R A T I O N S
	* === === === === === === === === === === === === === === ===
	*/

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
		return find_dfs(cur->right, val);
	}

	static const_iterator detach(const_iterator iter)
	{
		if (!iter.Ptr_->left && !iter.Ptr_->right)
		{
			if (iter.Ptr_->parent->left == iter.Ptr_)
				iter.Ptr_->parent->left = nullptr;
			else
				iter.Ptr_->parent->right = nullptr;
		}
		else if (!iter.Ptr_->left && iter.Ptr_->right)
		{
			if (iter.Ptr_->parent->left == iter.Ptr_)
				iter.Ptr_->parent->left = iter.Ptr_->right;
			else
				iter.Ptr_->parent->right = iter.Ptr_->right;
			iter.Ptr_->right->parent = iter.Ptr_->parent;
		}
		else if (iter.Ptr_->left && !iter.Ptr_->right)
		{
			if (iter.Ptr_->parent->left == iter.Ptr_)
				iter.Ptr_->parent->left = iter.Ptr_->left;
			else
				iter.Ptr_->parent->right = iter.Ptr_->left;
			iter.Ptr_->left->parent = iter.Ptr_->parent;
		}
		return iter;
	}

	static base_node * minimum(base_node * cur) {
		if (cur->left == nullptr)
			return cur;
		return minimum(cur->left);
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
};

template<typename T>
void set<T>::swap(set<T> &other) noexcept
{
	if (root.left && other.root.left)
		std::swap(root.left->parent, other.root.left->parent);
	else if (root.left)
		root.left->parent = &other.root;
	else if (other.root.left)
		other.root.left->parent = &root;
	std::swap(root.left, other.root.left);
}

template <typename T>
void swap(set<T> &lhs, set<T> &rhs) noexcept {
	lhs.swap(rhs);
}

template<typename T>
set<T>::set(const set &other) : root() {
	for (auto x : other) {
		insert(x);
	}
}

template<typename T>
set<T>& set<T>::operator=(set<T> rhs) noexcept {
	swap(rhs);
	return *this;
}

template<typename T>
set<T>::~set() {
	//destroy(real_root());
	delete root.left;
	root.left = nullptr;
}

template<typename T>
typename set<T>::iterator set<T>::begin() const {
	base_node * cur = get_root();
	while (cur->left)
		cur = cur->left;
	return set<T>::iterator(cur);
}

template<typename T>
typename set<T>::iterator set<T>::end() const {
	return set<T>::iterator(get_root());
}

template<typename T>
typename set<T>::const_iterator set<T>::cbegin() const {
	return set::const_iterator(begin());
}

template<typename T>
typename set<T>::const_iterator set<T>::cend() const {
	return set::const_iterator(end());
}

template<typename T>
typename set<T>::base_node *set<T>::get_root() const {
	return const_cast<typename set<T>::base_node*>(&root);
}

#endif // SET_H