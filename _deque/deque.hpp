#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"

#include <cstddef>
#include <cmath>

namespace sjtu {

template <class T> class deque {
private:
  constexpr static size_t min_size = 16;

  struct node {
      node *prev_;
      node *next_;
      void *data_;
      bool has_value_;

      node() {
        data_ = operator new(sizeof(T));
        has_value_ = false;
      }

      node(const T& t) {
        data_ = operator new(sizeof(T));
        new (data_) T(t);
        has_value_ = true;
      }

      T& val() {
        return *(static_cast<T*>(data_));
      }

      const T& const_val() {
        return *(static_cast<T*>(data_));
      }

      ~node() {
        if (has_value_) {
          static_cast<T*>(data_)->~T();
        }
        operator delete(data_);
      }
    };
  
  struct block {
    block *prev_ = nullptr;
    block *next_ = nullptr;
    size_t size_ = 0;

    node *head_;
    node *tail_;

    block() {
      head_ = new node;
      tail_ = new node;
      head_->prev_ = nullptr;
      head_->next_ = tail_;
      tail_->prev_ = head_;
      tail_->next_ = nullptr;
    }

    block(const block& other) : size_(other.size_) {
      head_ = new node;
      tail_ = new node;
      head_->prev_ = nullptr;
      node *prev = head_;
      node *cur = other.head_->next_;
      while (cur != other.tail_) {
        node *n = new node(cur->val());
        prev->next_ = n;
        n->prev_ = prev;
        prev = n;
        cur = cur->next_;
      }
      prev->next_ = tail_;
      tail_->prev_ = prev;
      tail_->next_ = nullptr;
    }

    ~block() {
      node *cur = head_;
      while (cur) {
        node *del = cur;
        cur = cur->next_;
        delete del;
      }
    }

    node* front() {
      return head_->next_;
    }

    node* back() {
      return tail_->prev_;
    }

    node* at(size_t pos) {
      if (pos > size_) throw index_out_of_bound();
      if (pos >= size_ / 2) {
        node *cur = tail_;
        for (size_t i = 0; i < size_ - pos; i++) {
          cur = cur->prev_;
        }
        return cur;
      }
      node *cur = head_;
      for (size_t i = 0; i < pos + 1; i++) {
        cur = cur->next_;
      }
      return cur;
    }
  };
  
  size_t size_ = 0;

  block *head_ = nullptr;
  block *tail_ = nullptr;

  size_t blen() {
    size_t l = sqrt(size_);
    return l < min_size ? min_size : l;
  }

public:
  class const_iterator;
  class iterator {
    friend class deque;
    friend class const_iterator;
  private:
    /**
     * add data members.
     * just add whatever you want.
     */
    deque *fa_;
    block *b_;
    node *n_;

    iterator(deque *fa, block *b, node *n) : fa_(fa), b_(b), n_(n) {}

  public:
    iterator() = default;
    /**
     * return a new iterator which points to the n-next element.
     * if there are not enough elements, the behaviour is undefined.
     * same for operator-.
     */
    iterator operator+(const int &n) const {
      long long offset = 0;
      node *cur = b_->head_->next_;
      while (cur != n_) {
        cur = cur->next_;
        ++offset;
      }
      block *tb = b_;
      long long target = offset + static_cast<long long>(n);
      while (target < 0) {
        tb = tb->prev_;
        target += static_cast<long long>(tb->size_);
      }
      while (tb != fa_->tail_ && target >= static_cast<long long>(tb->size_)) {
        target -= static_cast<long long>(tb->size_);
        tb = tb->next_;
      }
      return iterator(fa_, tb, tb->at(static_cast<size_t>(target)));
    }
    iterator operator-(const int &n) const {
      long long offset = 0;
      node *cur = b_->head_->next_;
      while (cur != n_) {
        cur = cur->next_;
        ++offset;
      }
      block *tb = b_;
      long long target = offset - static_cast<long long>(n);
      while (target < 0) {
        tb = tb->prev_;
        target += static_cast<long long>(tb->size_);
      }
      while (tb != fa_->tail_ && target >= static_cast<long long>(tb->size_)) {
        target -= static_cast<long long>(tb->size_);
        tb = tb->next_;
      }
      return iterator(fa_, tb, tb->at(static_cast<size_t>(target)));
    }

    /**
     * return the distance between two iterators.
     * if they point to different vectors, throw
     * invaild_iterator.
     */
    int operator-(const iterator &rhs) const {
      if (fa_ != rhs.fa_) throw invalid_iterator();
      auto index_of = [](block *b, node *n, deque *fa) -> long long {
        long long idx = 0;
        block *cur_b = fa->head_->next_;
        while (cur_b != b) {
          idx += static_cast<long long>(cur_b->size_);
          cur_b = cur_b->next_;
        }
        long long off = 0;
        node *cur_n = b->head_->next_;
        while (cur_n != n) {
          cur_n = cur_n->next_;
          ++off;
        }
        return idx + off;
      };
      return static_cast<int>(index_of(b_, n_, fa_) - index_of(rhs.b_, rhs.n_, rhs.fa_));
    }
    iterator &operator+=(const int &n) {
      *this = iterator(*this + n);
      return *this;
    }
    iterator &operator-=(const int &n) {
      *this = iterator(*this - n);
      return *this;
    }

    /**
     * iter++
     */
    iterator operator++(int) {
      iterator it = *this;
      if (fa_ == nullptr || b_ == nullptr || n_ == nullptr || b_ == fa_->tail_ || n_ == b_->tail_) {
        throw invalid_iterator();
      }
      n_ = n_->next_;
      if (n_ == b_->tail_) {
        b_ = b_->next_;
        n_ = b_->head_->next_;
      }
      return it;
    }
    /**
     * ++iter
     */
    iterator &operator++() {
      if (fa_ == nullptr || b_ == nullptr || n_ == nullptr || b_ == fa_->tail_ || n_ == b_->tail_) {
        throw invalid_iterator();
      }
      n_ = n_->next_;
      if (n_ == b_->tail_) {
        b_ = b_->next_;
        n_ = b_->head_->next_;
      }
      return *this;
    }
    /**
     * iter--
     */
    iterator operator--(int) {
      iterator it = *this;
      if (fa_ == nullptr || b_ == nullptr || n_ == nullptr) {
        throw invalid_iterator();
      }
      if (b_ == fa_->head_->next_ && n_ == b_->head_->next_) {
        throw invalid_iterator();
      }
      n_ = n_->prev_;
      if (n_ == b_->head_) {
        b_ = b_->prev_;
        n_ = b_->tail_->prev_;
      }
      return it;
    }
    /**
     * --iter
     */
    iterator &operator--() {
      if (fa_ == nullptr || b_ == nullptr || n_ == nullptr) {
        throw invalid_iterator();
      }
      if (b_ == fa_->head_->next_ && n_ == b_->head_->next_) {
        throw invalid_iterator();
      }
      n_ = n_->prev_;
      if (n_ == b_->head_) {
        b_ = b_->prev_;
        n_ = b_->tail_->prev_;
      }
      return *this;
    }

    /**
     * *it
     */
    T &operator*() const {
      if (fa_ == nullptr || b_ == nullptr || n_ == nullptr || b_ == fa_->tail_ || n_ == b_->tail_ || n_ == b_->head_) {
        throw invalid_iterator();
      }
      return n_->val();
    }
    /**
     * it->field
     */
    T *operator->() const {
      if (fa_ == nullptr || b_ == nullptr || n_ == nullptr || b_ == fa_->tail_ || n_ == b_->tail_ || n_ == b_->head_) {
        throw invalid_iterator();
      }
      return &(n_->val());
    }

    /**
     * check whether two iterators are the same (pointing to the same
     * memory).
     */
    bool operator==(const iterator &rhs) const {
      return fa_ == rhs.fa_ && b_ == rhs.b_ && n_ == rhs.n_;
    }
    bool operator==(const const_iterator &rhs) const {
      return fa_ == rhs.fa_ && b_ == rhs.b_ && n_ == rhs.n_;
    }
    /**
     * some other operator for iterators.
     */
    bool operator!=(const iterator &rhs) const {
      return !(*this == rhs);
    }
    bool operator!=(const const_iterator &rhs) const {
      return !(*this == rhs);
    }
  };

  class const_iterator {
    friend class deque;
    friend class iterator;
    /**
     * it should has similar member method as iterator.
     * you can copy them, but with care!
     * and it should be able to be constructed from an iterator.
     */
     private:
    /**
     * add data members.
     * just add whatever you want.
     */
    const deque *fa_;
    block *b_;
    node *n_;

    const_iterator(const deque *fa, block *b, node *n) : fa_(fa), b_(b), n_(n) {}

  public:
    const_iterator() = default;
    /**
     * return a new iterator which points to the n-next element.
     * if there are not enough elements, the behaviour is undefined.
     * same for operator-.
     */
    const_iterator operator+(const int &n) const {
      long long offset = 0;
      node *cur = b_->head_->next_;
      while (cur != n_) {
        cur = cur->next_;
        ++offset;
      }
      block *tb = b_;
      long long target = offset + static_cast<long long>(n);
      while (target < 0) {
        tb = tb->prev_;
        target += static_cast<long long>(tb->size_);
      }
      while (tb != fa_->tail_ && target >= static_cast<long long>(tb->size_)) {
        target -= static_cast<long long>(tb->size_);
        tb = tb->next_;
      }
      return const_iterator(fa_, tb, tb->at(static_cast<size_t>(target)));
    }
    const_iterator operator-(const int &n) const {
      long long offset = 0;
      node *cur = b_->head_->next_;
      while (cur != n_) {
        cur = cur->next_;
        ++offset;
      }
      block *tb = b_;
      long long target = offset - static_cast<long long>(n);
      while (target < 0) {
        tb = tb->prev_;
        target += static_cast<long long>(tb->size_);
      }
      while (tb != fa_->tail_ && target >= static_cast<long long>(tb->size_)) {
        target -= static_cast<long long>(tb->size_);
        tb = tb->next_;
      }
      return const_iterator(fa_, tb, tb->at(static_cast<size_t>(target)));
    }

    /**
     * return the distance between two iterators.
     * if they point to different vectors, throw
     * invaild_iterator.
     */
    int operator-(const const_iterator &rhs) const {
      if (fa_ != rhs.fa_) throw invalid_iterator();
      auto index_of = [](block *b, node *n, const deque *fa) -> long long {
        long long idx = 0;
        block *cur_b = fa->head_->next_;
        while (cur_b != b) {
          idx += static_cast<long long>(cur_b->size_);
          cur_b = cur_b->next_;
        }
        long long off = 0;
        node *cur_n = b->head_->next_;
        while (cur_n != n) {
          cur_n = cur_n->next_;
          ++off;
        }
        return idx + off;
      };
      return static_cast<int>(index_of(b_, n_, fa_) - index_of(rhs.b_, rhs.n_, rhs.fa_));
    }
    const_iterator &operator+=(const int &n) {
      *this = const_iterator(*this + n);
      return *this;
    }
    const_iterator &operator-=(const int &n) {
      *this = const_iterator(*this - n);
      return *this;
    }

    /**
     * iter++
     */
    const_iterator operator++(int) {
      const_iterator it = *this;
      if (fa_ == nullptr || b_ == nullptr || n_ == nullptr || b_ == fa_->tail_ || n_ == b_->tail_) {
        throw invalid_iterator();
      }
      n_ = n_->next_;
      if (n_ == b_->tail_) {
        b_ = b_->next_;
        n_ = b_->head_->next_;
      }
      return it;
    }
    /**
     * ++iter
     */
    const_iterator &operator++() {
      if (fa_ == nullptr || b_ == nullptr || n_ == nullptr || b_ == fa_->tail_ || n_ == b_->tail_) {
        throw invalid_iterator();
      }
      n_ = n_->next_;
      if (n_ == b_->tail_) {
        b_ = b_->next_;
        n_ = b_->head_->next_;
      }
      return *this;
    }
    /**
     * iter--
     */
    const_iterator operator--(int) {
      const_iterator it = *this;
      if (fa_ == nullptr || b_ == nullptr || n_ == nullptr) {
        throw invalid_iterator();
      }
      if (b_ == fa_->head_->next_ && n_ == b_->head_->next_) {
        throw invalid_iterator();
      }
      n_ = n_->prev_;
      if (n_ == b_->head_) {
        b_ = b_->prev_;
        n_ = b_->tail_->prev_;
      }
      return it;
    }
    /**
     * --iter
     */
    const_iterator &operator--() {
      if (fa_ == nullptr || b_ == nullptr || n_ == nullptr) {
        throw invalid_iterator();
      }
      if (b_ == fa_->head_->next_ && n_ == b_->head_->next_) {
        throw invalid_iterator();
      }
      n_ = n_->prev_;
      if (n_ == b_->head_) {
        b_ = b_->prev_;
        n_ = b_->tail_->prev_;
      }
      return *this;
    }

    /**
     * *it
     */
    const T &operator*() const {
      if (fa_ == nullptr || b_ == nullptr || n_ == nullptr || b_ == fa_->tail_ || n_ == b_->tail_ || n_ == b_->head_) {
        throw invalid_iterator();
      }
      return n_->const_val();
    }
    /**
     * it->field
     */
    const T *operator->() const {
      if (fa_ == nullptr || b_ == nullptr || n_ == nullptr || b_ == fa_->tail_ || n_ == b_->tail_ || n_ == b_->head_) {
        throw invalid_iterator();
      }
      return &(n_->const_val());
    }

    /**
     * check whether two iterators are the same (pointing to the same
     * memory).
     */
    bool operator==(const iterator &rhs) const {
      return fa_ == rhs.fa_ && b_ == rhs.b_ && n_ == rhs.n_;
    }
    bool operator==(const const_iterator &rhs) const {
      return fa_ == rhs.fa_ && b_ == rhs.b_ && n_ == rhs.n_;
    }
    /**
     * some other operator for iterators.
     */
    bool operator!=(const iterator &rhs) const {
      return !(*this == rhs);
    }
    bool operator!=(const const_iterator &rhs) const {
      return !(*this == rhs);
    }
  };

  /**
   * constructors.
   */
  deque() {
    head_ = new block;
    tail_ = new block;
    head_->next_ = tail_;
    tail_->prev_ = head_;
    size_ = 0;
  }
  deque(const deque &other) : size_(other.size_) {
    head_ = new block;
    tail_ = new block;
    block *cur = other.head_->next_;
    block *prev = head_;
    while (cur != other.tail_) {
      block *data_block = new block(*cur);
      data_block->prev_ = prev;
      prev->next_ = data_block;
      prev = data_block;
      cur = cur->next_;
    }
    prev->next_ = tail_;
    tail_->prev_ = prev;
  }

  /**
   * deconstructor.
   */
  ~deque() {
    block *cur = head_;
    while (cur) {
      block *del = cur;
      cur = cur->next_;
      delete del;
    }
  }

  /**
   * assignment operator.
   */
  deque &operator=(const deque &other) {
    if (this == &other) {
      return *this;
    }
    block *cur = head_->next_;
    while (cur != tail_) {
      block *del = cur;
      cur = cur->next_;
      delete del;
    }
    cur = other.head_->next_;
    block *prev = head_;
    while (cur != other.tail_) {
      block *data_block = new block(*cur);
      data_block->prev_ = prev;
      prev->next_ = data_block;
      prev = data_block;
      cur = cur->next_;
    }
    prev->next_ = tail_;
    tail_->prev_ = prev;
    size_ = other.size_;
    return *this;
  }

  /**
   * access a specified element with bound checking.
   * throw index_out_of_bound if out of bound.
   */
  T &at(const size_t &pos) {
    if (pos >= size_) {
      throw index_out_of_bound();
    }
    size_t remain = pos;
    block *cur = head_->next_;
    while (cur != tail_) {
      if (remain >= cur->size_) {
        remain -= cur->size_;
        cur = cur->next_;
        continue;
      }
      return cur->at(remain)->val();
    }
    throw index_out_of_bound();
  }
  const T &at(const size_t &pos) const {
    if (pos >= size_) {
      throw index_out_of_bound();
    }
    size_t remain = pos;
    block *cur = head_->next_;
    while (cur != tail_) {
      if (remain >= cur->size_) {
        remain -= cur->size_;
        cur = cur->next_;
        continue;
      }
      return cur->at(remain)->val();
    }
    throw index_out_of_bound();
  }
  T &operator[](const size_t &pos) {
    return at(pos);
  }
  const T &operator[](const size_t &pos) const {
    return at(pos);
  }

  /**
   * access the first element.
   * throw container_is_empty when the container is empty.
   */
  const T &front() const {
    if (size_ == 0) throw container_is_empty();
    return head_->next_->front()->val();
  }
  /**
   * access the last element.
   * throw container_is_empty when the container is empty.
   */
  const T &back() const {
    if (size_ == 0) throw container_is_empty();
    return tail_->prev_->back()->val();
  }

  /**
   * return an iterator to the beginning.
   */
  iterator begin() {
    return iterator(this, head_->next_, head_->next_->head_->next_);
  }
  const_iterator cbegin() const {
    return const_iterator(this, head_->next_, head_->next_->head_->next_);
  }

  /**
   * return an iterator to the end.
   */
  iterator end() {
    return iterator(this, tail_, tail_->head_->next_);
  }
  const_iterator cend() const {
    return const_iterator(this, tail_, tail_->head_->next_);
  }

  /**
   * check whether the container is empty.
   */
  bool empty() const {
    return size_ == 0;
  }

  /**
   * return the number of elements.
   */
  size_t size() const {
    return size_;
  }

  /**
   * clear all contents.
   */
  void clear() {
    *this = deque();
  }

  /**
   * insert value before pos.
   * return an iterator pointing to the inserted value.
   * throw if the iterator is invalid or it points to a wrong place.
   */
  iterator insert(iterator pos, const T &value) {
    if (pos.fa_ != this) throw invalid_iterator();
    if (pos.b_ == head_) throw invalid_iterator();

    if (pos.b_ == tail_) {
      push_back(value);
      return end() - 1;
    }

    node *n = pos.n_;
    block *b = pos.b_;
    node *nn = new node(value);
    n->prev_->next_ = nn;
    nn->prev_ = n->prev_;
    n->prev_ = nn;
    nn->next_ = n;
    b->size_++;
    size_++;
    int l = blen();
    if (b->size_ >= 2 * l) {
      node *sep = b->at(static_cast<size_t>(l - 1));
      block *nb = new block;
      nb->prev_ = b;
      nb->next_ = b->next_;
      b->next_->prev_ = nb;
      b->next_ = nb;
      nb->head_->next_ = sep->next_;
      nb->tail_->prev_ = b->tail_->prev_;
      sep->next_->prev_ = nb->head_;
      b->tail_->prev_->next_ = nb->tail_;
      sep->next_ = b->tail_;
      b->tail_->prev_ = sep;
      size_t moved = b->size_ - l;
      nb->size_ = moved;
      b->size_ = l;
      node *cur = b->head_->next_;
      while (1) {
        if (cur == nn) return iterator(this, b, cur);
        cur = cur->next_;
        if (cur == b->tail_) break;
      }
      cur = nb->head_->next_;
      while (1) {
        if (cur == nn) return iterator(this, nb, cur);
        cur = cur->next_;
        if (cur == nb->tail_) break;
      }
    }
    return iterator(this, b, nn);
  }

  /**
   * remove the element at pos.
   * return an iterator pointing to the following element. if pos points to
   * the last element, return end(). throw if the container is empty,
   * the iterator is invalid, or it points to a wrong place.
   */
  iterator erase(iterator pos) {
    if (size_ == 0) {
      throw container_is_empty();
    }
    if (pos.fa_ != this || pos.b_ == head_ || pos.b_ == tail_ || pos.n_ == pos.b_->tail_) {
      throw invalid_iterator();
    }
    node *del = pos.n_;
    block *b = pos.b_;
    del->prev_->next_ = del->next_;
    del->next_->prev_ = del->prev_;
    node *n = del->next_;
    delete del;
    b->size_--;
    size_--;
    if (b->size_ == 0) {
      b->next_->prev_ = b->prev_;
      b->prev_->next_ = b->next_;
      block *nb = b->next_;
      delete b;
      return iterator(this, nb, nb->head_->next_);
    }
    if (n == b->tail_) {
      b = b->next_;
      n = b->head_->next_;
    }
    return iterator(this, b, n);
  }

  /**
   * add an element to the end.
   */
  void push_back(const T &value) {
    block *b = tail_->prev_;
    if (b->size_ >= blen() || b == head_) {
      block *nb = new block;
      b->next_ = nb;
      nb->next_ = tail_;
      tail_->prev_ = nb;
      nb->prev_ = b;
      node *n = new node(value);
      nb->head_->next_ = n;
      n->next_ = nb->tail_;
      nb->tail_->prev_ = n;
      n->prev_ = nb->head_;
      nb->size_ = 1;
      size_++;
      return;
    }
    node *n = new node(value);
    node *p = b->back();
    p->next_ = n;
    n->next_ = b->tail_;
    b->tail_->prev_ = n;
    n->prev_ = p;
    b->size_++;
    size_++;
  }

  /**
   * remove the last element.
   * throw when the container is empty.
   */
  void pop_back() {
    if (!size_) throw container_is_empty();
    block *b = tail_->prev_;
    node *n = b->back();
    n->prev_->next_ = b->tail_;
    b->tail_->prev_ = n->prev_;
    delete n;
    b->size_--;
    size_--;
    if (b->size_ == 0) {
      b->prev_->next_ = tail_;
      tail_->prev_ = b->prev_;
      delete b;
    }
  }

  /**
   * insert an element to the beginning.
   */
  void push_front(const T &value) {
    block *b = head_->next_;
    if (b->size_ > blen() || b == tail_) {
      block *nb = new block;
      b->prev_ = nb;
      nb->prev_ = head_;
      head_->next_ = nb;
      nb->next_ = b;
      node *n = new node(value);
      nb->head_->next_ = n;
      n->next_ = nb->tail_;
      nb->tail_->prev_ = n;
      n->prev_ = nb->head_;
      nb->size_ = 1;
      size_++;
      return;
    }
    node *n = new node(value);
    node *p = b->front();
    p->prev_ = n;
    n->prev_ = b->head_;
    b->head_->next_ = n;
    n->next_ = p;
    b->size_++;
    size_++;
  }

  /**
   * remove the first element.
   * throw when the container is empty.
   */
  void pop_front() {
    if (!size_) throw container_is_empty();
    block *b = head_->next_;
    node *n = b->front();
    n->next_->prev_ = b->head_;
    b->head_->next_ = n->next_;
    delete n;
    b->size_--;
    size_--;
    if (b->size_ == 0) {
      head_->next_ = b->next_;
      b->next_->prev_ = head_;
      delete b;
    }
  }
};

} // namespace sjtu

#endif
