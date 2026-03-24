#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

#include <iostream>
#include <stdexcept>

namespace sjtu {
template<typename T>
struct CycleList {
	struct Node {
		T data;
		Node *prev;
		Node *next;

		Node(): prev(nullptr), next(nullptr) {}
		Node(const T& val) : data(val), prev(nullptr), next(nullptr) {}
	};

	Node *head;
	size_t size_;

	CycleList(): head(nullptr), size_(0) {}

	~CycleList() {
		if (head == nullptr) {
			return;
		}
		Node *cur = head->next;
		while (cur != head) {
			Node *del = cur;
			cur = cur->next;
			delete del;
		}
		if (cur) {
			delete cur;
		}
	}

	CycleList(const CycleList &other) {
		size_ = other.size_;
		if (other.head == nullptr) {
			head = nullptr;
			return;
		}
		head = new Node(other.head->data);
		Node *cur = head, *oth_cur = other.head->next;
		while(oth_cur != other.head) {
			Node *new_node = new Node(oth_cur->data);
			cur->next = new_node;
			new_node->prev = cur;
			oth_cur = oth_cur->next;
			cur = cur->next;
		}
		cur->next = head;
		head->prev = cur;
	}

	void push(const T &val) {
		Node *new_node = new Node(val);
		size_++;
		if (head == nullptr) {
			new_node->prev = new_node;
			new_node->next = new_node;
			head = new_node;
			return;
		}
		Node *prev_node = head->prev, *next_node = head;
		prev_node->next = new_node;
		new_node->next = next_node;
		next_node->prev = new_node;
		new_node->prev = prev_node;
		head = new_node;
	}

	T front() const {
		if (head == nullptr) {
			throw container_is_empty();
		}
		return head->data;
	}

	void pop() {
		if (head == nullptr) {
			throw container_is_empty();
		}
		if (size_ == 1) {
			delete head;
			head = nullptr;
			size_ = 0;
			return;
		}
		size_--;
		Node *del = head;
		head->prev->next = head->next;
		head->next->prev = head->prev;
		head = head->next;
		delete del;
	}

	size_t size() const {
		return size_;
	}

	bool empty() const {
		return size_ == 0;
	}

	void merge(CycleList<T> &other) {
		if (this == &other) return;
		if (other.size_ == 0) return;
		size_ += other.size_;
		if (head == nullptr) {
			head = other.head;
			other.head = nullptr;
			other.size_ = 0;
			return;
		}
		Node *first_node = head, *last_node = head->prev;
		Node *oth_first = other.head, *oth_last = other.head->prev;
		oth_last->next = first_node;
		first_node->prev = oth_last;
		last_node->next = oth_first;
		oth_first->prev = last_node;
		other.head = nullptr;
		other.size_ = 0;
	}

	void debug() const {
		if (size_ == 0) return;
		Node *cur = head;
		do {
			cur = cur->next;
		} while (cur != head);
	}
};

/**
 * @brief a container like std::priority_queue which is a heap internal.
 * **Exception Safety**: The `Compare` operation might throw exceptions for certain data.
 * In such cases, any ongoing operation should be terminated, and the priority queue should be restored to its original state before the operation began.
 */
template<typename T, class Compare = std::less<T>>
class priority_queue {
	struct HeapNode {
		T data;
		HeapNode *parent = nullptr;
		HeapNode *child = nullptr;
		HeapNode *prev = nullptr;
		HeapNode *next = nullptr;
		size_t deg = 0;
		int mark = 0;

		HeapNode(const T& val) : data(val) {}
	};

	HeapNode *minode_ = nullptr;
	size_t size_;
	Compare comp_;

	HeapNode* insert_after(HeapNode *pos, const T& val) {
		if (pos == nullptr) {
			throw std::runtime_error("inserting after nullptr!");
		}
		HeapNode *new_node = new HeapNode(val);
		HeapNode *prev_node = pos;
		HeapNode *next_node = pos->next;
		prev_node->next = new_node;
		new_node->next = next_node;
		next_node->prev = new_node;
		new_node->prev = prev_node;
		return new_node;
	}

	HeapNode* erase_at(HeapNode *pos, HeapNode *& extracted) {
		if (pos == nullptr) {
			throw std::runtime_error("inserting after nullptr!");
		}
		if (pos->next == pos) {
			// deleting the last node in list
			delete pos;
			return nullptr;
		}
		HeapNode *del_node = pos;
		HeapNode *prev_node = pos->prev;
		HeapNode *next_node = pos->next;
		prev_node->next = next_node;
		next_node->prev = prev_node;
		extracted = del_node;
		return next_node;
	}

public:
	/**
	 * @brief default constructor
	 */
	priority_queue() {
		size_ = 0;
		minode_ = nullptr;
	}

	/**
	 * @brief copy constructor
	 * @param other the priority_queue to be copied
	 */
	priority_queue(const priority_queue &other) {}

	/**
	 * @brief deconstructor
	 */
	~priority_queue() {}

	/**
	 * @brief Assignment operator
	 * @param other the priority_queue to be assigned from
	 * @return a reference to this priority_queue after assignment
	 */
	priority_queue &operator=(const priority_queue &other) {}

	/**
	 * @brief get the top element of the priority queue.
	 * @return a reference of the top element.
	 * @throws container_is_empty if empty() returns true
	 * @note this function will ALWAYS assume that comp_ will not throw an exception.
	 * @note it is the caller's RESPONSIBILITY that the trivial comp_ works SAFELY.
	 */
	const T & top() const {
		
	}

	/**
	 * @brief push new element to the priority queue.
	 * @param e the element to be pushed
	 */
	void push(const T &e) {
		// simply throw it into root list
		if (minode_ == nullptr) {
			// root list is empty!
			minode_ = new HeapNode(e);
			minode_->next = minode_;
			minode_->prev = minode_;
			size_++;
			return;
		}
		// root list has something.
		bool is_less_than_min = false;
		try {
			is_less_than_min = comp_(e, minode_->data);
		}
		catch(...) {
			return;
		}
		size_++;
		// just throw it into root list.
		HeapNode *new_node = insert_after(minode_, e);
		if (is_less_than_min) {
			// update min node.
			minode_ = new_node;
		}
	}

	/**
	 * @brief delete the top element from the priority queue.
	 * @throws container_is_empty if empty() returns true
	 */
	void pop() {}

	/**
	 * @brief return the number of elements in the priority queue.
	 * @return the number of elements.
	 */
	size_t size() const {
		return size_;
	}

	/**
	 * @brief check if the container is empty.
	 * @return true if it is empty, false otherwise.
	 */
	bool empty() const {
		return size_ == 0;
	}

	/**
	 * @brief merge another priority_queue into this one.
	 * The other priority_queue will be cleared after merging.
	 * The complexity is at most O(logn).
	 * @param other the priority_queue to be merged.
	 */
	void merge(priority_queue &other) {
		// discard self merging.
		if (this == &other) return;
		if (other.size_ == 0) {
			// the other is empty.
			return;
		}
		if (minode_ == nullptr) {
			// this heap has no roots.
			// just steal the other's data.
			minode_ = other.minode_;
			size_ = other.size_;
			// now clear the other heap.
			other.minode_ = nullptr;
			other.size_ = 0;
			return;
		}
		bool is_less_than_other = false;
		try {
			is_less_than_other = comp_(minode_->data, other.minode_->data);
		}
		catch(...) {
			// quit since comp_ explodes.
			return;
		}
		// comp_ works well.
		size_ += other.size_;
		// now we merge the two root lists.
		HeapNode *first_node = minode_, *last_node = minode_->prev;
		HeapNode *oth_first = other.minode_, *oth_last = other.minode_->prev;
		oth_last->next = first_node;
		first_node->prev = oth_last;
		last_node->next = oth_first;
		oth_first->prev = last_node;
		// now clear the other heap.
		other.minode_ = nullptr;
		other.size_ = 0;
	}
};

}

#endif