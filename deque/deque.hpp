#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"

#include <cstddef>
#include <cstdint>

namespace sjtu { 

template<class T>
class deque {
	constexpr static size_t block_size = 16;
	struct block {
		void *buffer_ = nullptr;
		T *data_ = nullptr;
		size_t l = 0;
		size_t r = 0;
		
		block() {
			buffer_ = operator new(sizeof(T) * block_size);
			data_ = static_cast<T*>(buffer_);
		}

		block(const block& other) {
			buffer_ = operator new(sizeof(T) * block_size);
			data_ = static_cast<T*>(buffer_);
			l = other.l;
			r = other.r;
			for (size_t i = l; i < r; i++) {
				new (data_ + i) T(other.data_[i]);
			}
		}

		block &operator=(const block &other) {
			if (this == &other) return *this;
			for (size_t i = l; i < r; ++i) data_[i].~T();
			l = other.l;
			r = other.r;
			for (size_t i = l; i < r; ++i) new (data_ + i) T(other.data_[i]);
			return *this;
		}

		~block() {
			for (size_t i = l; i < r; ++i) data_[i].~T();
			operator delete(buffer_);
		}

		inline bool empty() const {
			return l == r;
		}

		inline bool full() const {
			return l == 0 && r == block_size;
		}

		inline size_t size() const {
			return r - l;
		}

		void push_front(const T& val) {
			if (empty()) {
				r = block_size;
				l = block_size - 1; 
				new (data_ + l) T(val);
				return;
			}
			if (l > 0) {
				l--;
				new (data_ + l) T(val);
				return;
			}
			if (r < block_size) {
				new (data_ + r) T(data_[r - 1]);
				for (size_t i = r - 1; i > 0; --i) {
					data_[i] = data_[i - 1];
				}
				data_[0] = val;
				++r;
			}
		}

		void pop_front() {
			if (!empty()) {
				data_[l].~T();
				l++;
				if (l == r) l = r = 0;
			}
		}

		void push_back(const T& val) {
			if (empty()) {
				l = r = 0;
			}
			if (r < block_size) {
				new (data_ + r) T(val);
				r++;
				return;
			}
			if (l > 0) {
				new (data_ + (l - 1)) T(data_[l]);
				for (size_t i = l; i + 1 < r; ++i) {
					data_[i] = data_[i + 1];
				}
				data_[r - 1].~T();
				--l;
				new (data_ + (r - 1)) T(val);
			}
		}

		void pop_back() {
			if (!empty()) {
				r--;
				data_[r].~T();
				if (l == r) l = r = 0;
			}
		}

		T& operator[](size_t index) {
			if (l + index >= r) throw index_out_of_bound();
			return data_[l + index];
		}

		const T& operator[](size_t index) const {
			if (l + index >= r) throw index_out_of_bound();
			return data_[l + index];
		}

		const T& front() const {
			if (l == r) throw container_is_empty();
			return data_[l];
		}

		const T& back() const {
			if (l == r) throw container_is_empty();
			return data_[r - 1];
		}
	};

	block *blocks_;
	size_t L;
	size_t R;
	size_t size_;
	size_t capacity_;

	void initialize() {
		capacity_ = 8;
		L = 3;
		R = 4;
		blocks_ = new block[capacity_];
		size_ = 0;
	}

	void expand() {
		size_t old_L = L;
		size_t old_R = R;
		block *old_blocks = blocks_;
		capacity_ <<= 1;
		block *new_blocks = new block[capacity_];
		size_t used = old_R - old_L;
		L = capacity_ / 4;
		R = L + used;
		for (size_t i = 0; i < used; ++i) {
			new_blocks[L + i] = old_blocks[old_L + i];
		}
		blocks_ = new_blocks;
		delete []old_blocks;
	}

	void release() {
		delete []blocks_;
	}

	inline size_t offset() const {
		return blocks_[L].r - blocks_[L].l;
	}

public:
	class const_iterator;
	class iterator {
		friend deque;
		friend const_iterator;
	private:
		/**
		 * TODO add data members
		 *   just add whatever you want.
		 */
		deque *fa_;
		size_t b_;
		size_t idx_;

		iterator(deque *fa, size_t b, size_t idx) : fa_(fa), b_(b), idx_(idx) {}

		inline int64_t uid() const {
			size_t off = fa_->offset();
			if (b_ == fa_->L) return static_cast<int64_t>(idx_);
			return static_cast<int64_t>(off + (b_ - fa_->L - 1) * block_size + idx_);
		}
	public:
		iterator() = default;
		/**
		 * return a new iterator which pointer n-next elements
		 *   even if there are not enough elements, the behaviour is **undefined**.
		 * as well as operator-
		 */
		iterator operator+(const int &n) const {
			//TODO
			if (n < 0) return *this - (-n);
			if (n == 0) return *this;
			size_t target = static_cast<size_t>(uid()) + static_cast<size_t>(n);
			if (target > fa_->size_) throw index_out_of_bound();
			if (target == fa_->size_) return iterator{fa_, fa_->R - 1, fa_->blocks_[fa_->R - 1].size()};
			size_t off = fa_->offset();
			if (target < off) return iterator{fa_, fa_->L, target};
			size_t x = target - off;
			size_t b = fa_->L + 1 + x / block_size;
			size_t idx = x % block_size;
			if (b < fa_->L || b >= fa_->R) throw index_out_of_bound();
			if (idx >= fa_->blocks_[b].size()) throw index_out_of_bound();
			return iterator{fa_, b, idx};
		}
		iterator operator-(const int &n) const {
			//TODO
			if (n < 0) return *this + (-n);
			if (n == 0) return *this;
			size_t cur = static_cast<size_t>(uid());
			if (static_cast<size_t>(n) > cur) throw index_out_of_bound();
			size_t target = cur - static_cast<size_t>(n);
			if (target == fa_->size_) return iterator{fa_, fa_->R - 1, fa_->blocks_[fa_->R - 1].size()};
			size_t off = fa_->offset();
			if (target < off) return iterator{fa_, fa_->L, target};
			size_t x = target - off;
			size_t b = fa_->L + 1 + x / block_size;
			size_t idx = x % block_size;
			if (b < fa_->L || b >= fa_->R) throw index_out_of_bound();
			if (idx >= fa_->blocks_[b].size()) throw index_out_of_bound();
			return iterator{fa_, b, idx};
		}
		// return th distance between two iterator,
		// if these two iterators points to different vectors, throw invaild_iterator.
		int operator-(const iterator &rhs) const {
			//TODO
			if (fa_ != rhs.fa_) throw invalid_iterator();
			return uid() - rhs.uid();
		}
		iterator operator+=(const int &n) {
			//TODO
			*this = *this + n;
			return *this;
		}
		iterator operator-=(const int &n) {
			//TODO
			*this = *this - n;
			return *this;
		}
		/**
		 * TODO iter++
		 */
		iterator operator++(int) {
			if (b_ == fa_->R - 1 && idx_ == fa_->blocks_[b_].size()) throw index_out_of_bound();
			iterator it = *this;
			idx_++;
			if (b_ < fa_->R - 1 && idx_ == fa_->blocks_[b_].size()) {
				b_++;
				idx_ = 0;
			}
			return it;
		}
		/**
		 * TODO ++iter
		 */
		iterator& operator++() {
			if (b_ == fa_->R - 1 && idx_ == fa_->blocks_[b_].size()) throw index_out_of_bound();
			idx_++;
			if (b_ < fa_->R - 1 && idx_ == fa_->blocks_[b_].size()) {
				b_++;
				idx_ = 0;
			}
			return *this;
		}
		/**
		 * TODO iter--
		 */
		iterator operator--(int) {
			if (b_ == fa_->L && idx_ == 0) throw index_out_of_bound();
			iterator it = *this;
			if (idx_ == 0) {
				b_--;
				idx_ = fa_->blocks_[b_].size() - 1;
			}
			else idx_--;
			return it;
		}
		/**
		 * TODO --iter
		 */
		iterator& operator--() {
			if (b_ == fa_->L && idx_ == 0) throw index_out_of_bound();
			if (idx_ == 0) {
				b_--;
				idx_ = fa_->blocks_[b_].size() - 1;
			}
			else idx_--;
			return *this;
		}
		/**
		 * TODO *it
		 */
		T& operator*() const {
			return fa_->blocks_[b_][idx_];
		}
		/**
		 * TODO it->field
		 */
		T* operator->() const noexcept {
			return &(fa_->blocks_[b_][idx_]);
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		bool operator==(const iterator &rhs) const {
			return fa_ == rhs.fa_ && b_ == rhs.b_ && idx_ == rhs.idx_;
		}
		bool operator==(const const_iterator &rhs) const {
			return fa_ == rhs.fa_ && b_ == rhs.b_ && idx_ == rhs.idx_;
		}
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {
			return !(*this == rhs);
		}
		bool operator!=(const const_iterator &rhs) const {
			return !(*this == rhs);
		}
	};
	class const_iterator {
		friend deque;
		friend iterator;
	private:
		/**
		 * TODO add data members
		 *   just add whatever you want.
		 */
		const deque *fa_;
		size_t b_;
		size_t idx_;

		const_iterator(const deque *fa, size_t b, size_t idx) : fa_(fa), b_(b), idx_(idx) {}

		inline int64_t uid() const {
			size_t off = fa_->offset();
			if (b_ == fa_->L) return static_cast<int64_t>(idx_);
			return static_cast<int64_t>(off + (b_ - fa_->L - 1) * block_size + idx_);
		}
	public:
		const_iterator() = default;
		const_iterator(const iterator &other) : fa_(other.fa_), b_(other.b_), idx_(other.idx_) {}
		/**
		 * return a new iterator which pointer n-next elements
		 *   even if there are not enough elements, the behaviour is **undefined**.
		 * as well as operator-
		 */
		const_iterator operator+(const int &n) const {
			//TODO
			if (n < 0) return *this - (-n);
			if (n == 0) return *this;
			size_t target = static_cast<size_t>(uid()) + static_cast<size_t>(n);
			if (target > fa_->size_) throw index_out_of_bound();
			if (target == fa_->size_) return const_iterator{fa_, fa_->R - 1, fa_->blocks_[fa_->R - 1].size()};
			size_t off = fa_->offset();
			if (target < off) return const_iterator{fa_, fa_->L, target};
			size_t x = target - off;
			size_t b = fa_->L + 1 + x / block_size;
			size_t idx = x % block_size;
			if (b < fa_->L || b >= fa_->R) throw index_out_of_bound();
			if (idx >= fa_->blocks_[b].size()) throw index_out_of_bound();
			return const_iterator{fa_, b, idx};
		}
		const_iterator operator-(const int &n) const {
			//TODO
			if (n < 0) return *this + (-n);
			if (n == 0) return *this;
			size_t cur = static_cast<size_t>(uid());
			if (static_cast<size_t>(n) > cur) throw index_out_of_bound();
			size_t target = cur - static_cast<size_t>(n);
			if (target == fa_->size_) return const_iterator{fa_, fa_->R - 1, fa_->blocks_[fa_->R - 1].size()};
			size_t off = fa_->offset();
			if (target < off) return const_iterator{fa_, fa_->L, target};
			size_t x = target - off;
			size_t b = fa_->L + 1 + x / block_size;
			size_t idx = x % block_size;
			if (b < fa_->L || b >= fa_->R) throw index_out_of_bound();
			if (idx >= fa_->blocks_[b].size()) throw index_out_of_bound();
			return const_iterator{fa_, b, idx};
		}
		// return th distance between two iterator,
		// if these two iterators points to different vectors, throw invaild_iterator.
		int operator-(const const_iterator &rhs) const {
			//TODO
			if (fa_ != rhs.fa_) throw invalid_iterator();
			return uid() - rhs.uid();
		}
		const_iterator operator+=(const int &n) {
			//TODO
			*this = *this + n;
			return *this;
		}
		const_iterator operator-=(const int &n) {
			//TODO
			*this = *this - n;
			return *this;
		}
		/**
		 * TODO iter++
		 */
		const_iterator operator++(int) {
			if (b_ == fa_->R - 1 && idx_ == fa_->blocks_[b_].size()) throw index_out_of_bound();
			const_iterator it = *this;
			idx_++;
			if (b_ < fa_->R - 1 && idx_ == fa_->blocks_[b_].size()) {
				b_++;
				idx_ = 0;
			}
			return it;
		}
		/**
		 * TODO ++iter
		 */
		const_iterator& operator++() {
			if (b_ == fa_->R - 1 && idx_ == fa_->blocks_[b_].size()) throw index_out_of_bound();
			idx_++;
			if (b_ < fa_->R - 1 && idx_ == fa_->blocks_[b_].size()) {
				b_++;
				idx_ = 0;
			}
			return *this;
		}
		/**
		 * TODO iter--
		 */
		const_iterator operator--(int) {
			if (b_ == fa_->L && idx_ == 0) throw index_out_of_bound();
			const_iterator it = *this;
			if (idx_ == 0) {
				b_--;
				idx_ = fa_->blocks_[b_].size() - 1;
			}
			else idx_--;
			return it;
		}
		/**
		 * TODO --iter
		 */
		const_iterator& operator--() {
			if (b_ == fa_->L && idx_ == 0) throw index_out_of_bound();
			if (idx_ == 0) {
				b_--;
				idx_ = fa_->blocks_[b_].size() - 1;
			}
			else idx_--;
			return *this;
		}
		/**
		 * TODO *it
		 */
		const T& operator*() const {
			return fa_->blocks_[b_][idx_];
		}
		/**
		 * TODO it->field
		 */
		const T* operator->() const noexcept {
			return &(fa_->blocks_[b_][idx_]);
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		bool operator==(const iterator &rhs) const {
			return fa_ == rhs.fa_ && b_ == rhs.b_ && idx_ == rhs.idx_;
		}
		bool operator==(const const_iterator &rhs) const {
			return fa_ == rhs.fa_ && b_ == rhs.b_ && idx_ == rhs.idx_;
		}
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {
			return !(*this == rhs);
		}
		bool operator!=(const const_iterator &rhs) const {
			return !(*this == rhs);
		}
	};
	/**
	 * TODO Constructors
	 */
	deque() {
		initialize();
	}
	deque(const deque &other) : size_(other.size_), L(other.L), R(other.R), capacity_(other.capacity_) {
		blocks_ = new block[capacity_];
		for (size_t i = 0; i < capacity_; i++) {
			blocks_[i] = other.blocks_[i];
		}
	}
	/**
	 * TODO Deconstructor
	 */
	~deque() {
		release();
	}
	/**
	 * TODO assignment operator
	 */
	deque &operator=(const deque &other) {
		if (this == &other) return *this;
		size_ = other.size_;
		L = other.L;
		R = other.R;
		capacity_ = other.capacity_;
		delete []blocks_;
		blocks_ = new block[capacity_];
		for (size_t i = 0; i < capacity_; i++) {
			blocks_[i] = other.blocks_[i];
		}
		return *this;
	}
	/**
	 * access specified element with bounds checking
	 * throw index_out_of_bound if out of bound.
	 */
	T & at(const size_t &pos) {
		if (pos >= size_) throw index_out_of_bound();
		size_t off = offset();
		if (pos < off) return blocks_[L][pos];
		size_t b = (pos - off) / block_size + L + 1;
		size_t idx = (pos - off) % block_size;
		if (b >= R) throw index_out_of_bound();
		return blocks_[b][idx];
	}
	const T & at(const size_t &pos) const {
		if (pos >= size_) throw index_out_of_bound();
		size_t off = offset();
		if (pos < off) return blocks_[L][pos];
		size_t b = (pos - off) / block_size + L + 1;
		size_t idx = (pos - off) % block_size;
		if (b >= R) throw index_out_of_bound();
		return blocks_[b][idx];
	}
	T & operator[](const size_t &pos) {
		return at(pos);
	}
	const T & operator[](const size_t &pos) const {
		return at(pos);
	}
	/**
	 * access the first element
	 * throw container_is_empty when the container is empty.
	 */
	const T & front() const {
		if (size_ == 0) throw container_is_empty();
		return blocks_[L].front();
	}
	/**
	 * access the last element
	 * throw container_is_empty when the container is empty.
	 */
	const T & back() const {
		if (size_ == 0) throw container_is_empty();
		return blocks_[R - 1].back();
	}
	/**
	 * returns an iterator to the beginning.
	 */
	iterator begin() {
		return iterator{this, L, 0};
	}
	const_iterator cbegin() const {
		return const_iterator{this, L, 0};
	}
	/**
	 * returns an iterator to the end.
	 */
	iterator end() {
		return iterator{this, R - 1, blocks_[R - 1].size()};
	}
	const_iterator cend() const {
		return const_iterator{this, R - 1, blocks_[R - 1].size()};
	}
	/**
	 * checks whether the container is empty.
	 */
	bool empty() const {
		return size_ == 0;
	}
	/**
	 * returns the number of elements
	 */
	size_t size() const {
		return size_;
	}
	/**
	 * clears the contents
	 */
	void clear() {
		release();
		initialize();
	}
	/**
	 * inserts elements at the specified locat on in the container.
	 * inserts value before pos
	 * returns an iterator pointing to the inserted value
	 *     throw if the iterator is invalid or it point to a wrong place.
	 */
	iterator insert(iterator pos, const T &value) {
		if (pos.fa_ != this) throw invalid_iterator();
		if (pos.b_ < L || pos.b_ >= R) throw invalid_iterator();
		size_t pos_size = blocks_[pos.b_].size();
		bool is_end = (pos.b_ == R - 1 && pos.idx_ == pos_size);
		if (!is_end && pos.idx_ >= pos_size) throw invalid_iterator();
		size_t rank = 0;
		size_t off = offset();
		if (pos.b_ == L) rank = pos.idx_;
		else rank = off + (pos.b_ - L - 1) * block_size + pos.idx_;
		if (rank == size_) {
			push_back(value);
			return iterator{this, R - 1, blocks_[R - 1].size() - 1};
		}
		if (rank < (size_ >> 1)) {
			T head_copy(front());
			push_front(head_copy);
			for (size_t i = 0; i < rank; ++i) {
				at(i) = at(i + 1);
			}
			at(rank) = value;
		} else {
			T tail_copy(back());
			push_back(tail_copy);
			for (size_t i = size_ - 1; i > rank; --i) {
				at(i) = at(i - 1);
			}
			at(rank) = value;
		}
		off = offset();
		if (rank < off) return iterator{this, L, rank};
		size_t x = rank - off;
		size_t b = L + 1 + x / block_size;
		size_t idx = x % block_size;
		return iterator{this, b, idx};
	}
	/**
	 * removes specified element at pos.
	 * removes the element at pos.
	 * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
	 * throw if the container is empty, the iterator is invalid or it points to a wrong place.
	 */
	iterator erase(iterator pos) {
		if (size_ == 0) throw container_is_empty();
		if (pos.fa_ != this) throw invalid_iterator();
		if (pos.b_ < L || pos.b_ >= R) throw invalid_iterator();
		size_t pos_size = blocks_[pos.b_].size();
		if (pos.idx_ >= pos_size) throw invalid_iterator();
		size_t rank = 0;
		size_t off = offset();
		if (pos.b_ == L) rank = pos.idx_;
		else rank = off + (pos.b_ - L - 1) * block_size + pos.idx_;
		if (rank < (size_ >> 1)) {
			for (size_t i = rank; i > 0; --i) {
				at(i) = at(i - 1);
			}
			pop_front();
		} else {
			for (size_t i = rank; i + 1 < size_; ++i) {
				at(i) = at(i + 1);
			}
			pop_back();
		}
		if (rank == size_) return end();
		off = offset();
		if (rank < off) return iterator{this, L, rank};
		size_t x = rank - off;
		size_t b = L + 1 + x / block_size;
		size_t idx = x % block_size;
		return iterator{this, b, idx};
	}
	/**
	 * adds an element to the end
	 */
	void push_back(const T &value) {
		if (blocks_[R - 1].full()) {
			if (R == capacity_ - 1) {
				expand();
			}
			R++;
		}
		blocks_[R - 1].push_back(value);
		size_++;
	}
	/**
	 * removes the last element
	 *     throw when the container is empty.
	 */
	void pop_back() {
		if (size_ == 0) throw container_is_empty();
		blocks_[R - 1].pop_back();
		size_--;
		if (blocks_[R - 1].empty()) {
			if (R - L > 1) R--;
		}
	}
	/**
	 * inserts an element to the beginning.
	 */
	void push_front(const T &value) {
		if (blocks_[L].full()) {
			if (L == 0) {
				expand();
			}
			L--;
		}
		blocks_[L].push_front(value);
		size_++;
	}
	/**
	 * removes the first element.
	 *     throw when the container is empty.
	 */
	void pop_front() {
		if (size_ == 0) throw container_is_empty();
		blocks_[L].pop_front();
		size_--;
		if (blocks_[L].empty()) {
			if (R - L > 1) L++;
		}
	}
};

}

#endif
