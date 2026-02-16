/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
    class Key,
    class T,
    class Compare = std::less <Key>
> class map {
 public:
  /**
   * the internal type of data.
   * it should have a default constructor, a copy constructor.
   * You can use sjtu::map as value_type by typedef.
   */
  typedef pair<const Key, T> value_type;

private:
  struct tree_node {
    value_type keypair;
    tree_node *ls;
    tree_node *rs;
    int height;

    tree_node(const value_type& kp, int h = 0) : keypair(kp), height(h), ls(nullptr), rs(nullptr) {}
    tree_node(const Key& key, const T& t, int h = 0) : keypair(key, t), height(0), ls(nullptr), rs(nullptr) {}
  };

  tree_node *root_;
  size_t size_;
  Compare comp_;

  int height(tree_node *t) {
    return t ? t->height : -1;
  }

  void pushup(tree_node *t) {
    t->height = (height(t->ls) > height(t->rs)) ? height(t->ls) + 1 : height(t->rs) + 1;
  }

  void release(tree_node *t) {
    if (t->ls) {
      release(t->ls);
    }
    if (t->rs) {
      release(t->rs);
    }
    delete t;
  }

  void copy(tree_node *dst, tree_node *src) {
    if (src->ls) {
      dst->ls = new tree_node(*(src->ls));
      copy(dst->ls, src->ls);
    }
    if (src->rs) {
      dst->rs = new tree_node(*(src->rs));
      copy(dst->rs, src->rs);
    }
  }

  void LL(tree_node *&t) {
    tree_node *l=t->ls;
    t->ls=l->rs;
    l->rs=t;
    pushup(t);
    pushup(l);
    t=l;
  }

  void RR(tree_node *&t) {
    tree_node *r=t->rs;
    t->rs=r->ls;
    r->ls=t;
    pushup(t);
    pushup(r);
    t=r;
  }

  void LR(tree_node *&t) {
    RR(t->ls);
    LL(t);
  }

  void RL(tree_node *&t){
    LL(t->rs);
    RR(t);
  }

  void insert(const value_type& keypair, tree_node *&t) {
    if (t == nullptr) {
      t = new tree_node(keypair);
      size_++;
      return;
    }
    bool is_less, is_greater;
    is_less = comp_(keypair.first, t->keypair.first);
    is_greater = comp_(t->keypair.first, keypair.first);
    if (is_less) {
      insert(keypair, t->ls);
      if (height(t->ls) - height(t->rs) == 2) {
        if (comp_(keypair.first, t->ls->keypair.first)) {
          LL(t);
        }
        else {
          LR(t);
        }
      }
    }
    else if (is_greater) {
      insert(keypair, t->rs);
      if (height(t->rs) - height(t->ls) == 2) {
        if (comp_(t->rs->keypair.first, keypair.first)) {
          RR(t);
        }
        else {
          RL(t);
        }
      }
    }
    pushup(t);
  }

  bool erase(const Key& k, tree_node *&t) {
    bool stop = 0;
    int subt;
    if (t == nullptr) {
      return 1;
    }
    if (comp_(k, t->keypair.first)) {
      subt = 1;
      stop = erase(k, t->ls);
    }
    else if (comp_(t->keypair.first, k)) {
      subt = 2;
      stop = erase(k, t->rs);
    }
    else if (t->ls && t->rs) {
      tree_node *parent = t;
      tree_node *succ = t->rs;
      while (succ->ls) {
        parent = succ;
        succ = succ->ls;
      }
      if (parent != t) {
        parent->ls = succ->rs;
        succ->rs = t->rs;
      }
      succ->ls = t->ls;
      delete t;
      t = succ;
      size_--;
      pushup(t);
      return 0;
    }
    else {
      tree_node *del = t;
      t = t->ls ? t->ls : t->rs;
      delete del;
      size_--;
      return 0;
    }
    if (stop) {
      return 1;
    }
    int bf;
    if (subt == 1) {
      bf = height(t->ls) - height(t->rs);
      if (bf == 0) {
        return 1;
      }
      if (bf == 1) {
        return 0;
      }
      if (bf == -1) {
        int bfr = height(t->rs->ls) - height(t->rs->rs);
        if (bfr == 0) {
          RR(t);
          return 1;
        }
        if (bfr == -1) {
          RR(t);
          return 0;
        }
        if (bfr == 1) {
          RL(t);
          return 0;
        }
      }
    }
    else {
      bf = height(t->ls) - height(t->rs);
      if (bf == 0) {
        return 1;
      }
      if (bf == -1) {
        return 0;
      }
      if (bf == 1) {
        int bfl = height(t->ls->ls) - height(t->ls->rs);
        if (bfl == 0) {
          LL(t);
          return 1;
        }
        if (bfl == 1) {
          LL(t);
          return 0;
        }
        if (bfl == -1) {
          LR(t);
          return 0;
        }
      }
    }
    return 1;
  }

  tree_node* find(const Key& k, tree_node *t) const {
    if (t == nullptr) {
      return nullptr;
    }
    if (comp_(k, t->keypair.first)) {
      return find(k, t->ls);
    }
    else if (comp_(t->keypair.first, k)) {
      return find(k, t->rs);
    }
    else {
      return t;
    }
  }

  tree_node* previous_element(tree_node *t) const {
    if (t == nullptr) {
      tree_node *cur = root_;
      if (cur == nullptr) {
        return nullptr;
      }
      while (cur->rs) {
        cur = cur->rs;
      }
      return cur;
    }
    tree_node *temp = t->ls;
    if (temp == nullptr) {
      tree_node *ret = nullptr;
      tree_node *cur = root_;
      while (cur) {
        if (comp_(cur->keypair.first, t->keypair.first)) {
          ret = cur;
          cur = cur->rs;
        }
        else if (comp_(t->keypair.first, cur->keypair.first)) {
          cur = cur->ls;
        }
        else break;
      }
      return ret;
    }
    while (temp->rs) {
      temp = temp->rs;
    }
    return temp;
  }

  tree_node* next_element(tree_node *t) const {
    if (t == nullptr) {
      return nullptr;
    }
    tree_node *temp = t->rs;
    if (temp == nullptr) {
      tree_node *ret = nullptr;
      tree_node *cur = root_;
      while (cur) {
        if (comp_(t->keypair.first, cur->keypair.first)) {
          ret = cur;
          cur = cur->ls;
        }
        else if (comp_(cur->keypair.first, t->keypair.first)) {
          cur = cur->rs;
        }
        else break;
      }
      return ret;
    }
    while (temp->ls) {
      temp = temp->ls;
    }
    return temp;
  }

public:
  /**
   * see BidirectionalIterator at CppReference for help.
   *
   * if there is anything wrong throw invalid_iterator.
   *     like it = map.begin(); --it;
   *       or it = map.end(); ++end();
   */
  class const_iterator;
  class iterator {
    friend class map;
    friend class const_iterator;
   private:
    /**
     * TODO add data members
     *   just add whatever you want.
     */
    tree_node *ptr_;
    const map *fa_;

   public:
    iterator() {
      // TODO
      ptr_ = nullptr;
      fa_ = nullptr;
    }

    iterator(const iterator &other) {
      // TODO
      ptr_ = other.ptr_;
      fa_ = other.fa_;
    }

    /**
     * TODO iter++
     */
    iterator operator++(int) {
      if (fa_ == nullptr || ptr_ == nullptr) {
        throw invalid_iterator();
      }
      iterator temp = *this;
      ptr_ = fa_->next_element(ptr_);
      return temp;
    }

    /**
     * TODO ++iter
     */
    iterator &operator++() {
      if (fa_ == nullptr || ptr_ == nullptr) {
        throw invalid_iterator();
      }
      ptr_ = fa_->next_element(ptr_);
      return *this;
    }

    /**
     * TODO iter--
     */
    iterator operator--(int) {
      iterator temp = *this;
      if (fa_ == nullptr) {
        throw invalid_iterator();
      }
      if (ptr_ == nullptr) {
        ptr_ = fa_->previous_element(nullptr);
        if (ptr_ == nullptr) {
          throw invalid_iterator();
        }
        return temp;
      }
      tree_node *pre = fa_->previous_element(ptr_);
      if (pre == nullptr) {
        throw invalid_iterator();
      }
      ptr_ = pre;
      return temp;
    }

    /**
     * TODO --iter
     */
    iterator &operator--() {
      if (fa_ == nullptr) {
        throw invalid_iterator();
      }
      if (ptr_ == nullptr) {
        ptr_ = fa_->previous_element(nullptr);
        if (ptr_ == nullptr) {
          throw invalid_iterator();
        }
        return *this;
      }
      tree_node *pre = fa_->previous_element(ptr_);
      if (pre == nullptr) {
        throw invalid_iterator();
      }
      ptr_ = pre;
      return *this;
    }

    /**
     * a operator to check whether two iterators are same (pointing to the same memory).
     */
    value_type &operator*() const {
      if (ptr_ == nullptr) {
        throw invalid_iterator();
      }
      return ptr_->keypair;
    }

    bool operator==(const iterator &rhs) const {
      return ptr_ == rhs.ptr_ && fa_ == rhs.fa_;
    }

    bool operator==(const const_iterator &rhs) const {
      return ptr_ == rhs.ptr_ && fa_ == rhs.fa_;
    }

    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator &rhs) const {
      return ptr_ != rhs.ptr_ || fa_ != rhs.fa_;
    }

    bool operator!=(const const_iterator &rhs) const {
      return ptr_ != rhs.ptr_ || fa_ != rhs.fa_;
    }

    /**
     * for the support of it->first.
     * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
     */
    value_type *operator->() const
    {
      if (ptr_ == nullptr) {
        throw invalid_iterator();
      }
      return &(ptr_->keypair);
    }
  };
  class const_iterator {
    // it should has similar member method as iterator.
    //  and it should be able to construct from an iterator.
    friend class map;
    friend class iterator;
   private:
    // data members.
    tree_node *ptr_;
    const map *fa_;

   public:
    const_iterator() {
      // TODO
      ptr_ = nullptr;
      fa_ = nullptr;
    }

    const_iterator(const const_iterator &other) {
      // TODO
      ptr_ = other.ptr_;
      fa_ = other.fa_;
    }

    const_iterator(const iterator &other) {
      // TODO
      ptr_ = other.ptr_;
      fa_ = other.fa_;
    }
    // And other methods in iterator.
    // And other methods in iterator.
    // And other methods in iterator.
    /**
     * TODO iter++
     */
    const_iterator operator++(int) {
      if (fa_ == nullptr || ptr_ == nullptr) {
        throw invalid_iterator();
      }
      const_iterator temp = *this;
      ptr_ = fa_->next_element(ptr_);
      return temp;
    }

    /**
     * TODO ++iter
     */
    const_iterator &operator++() {
      if (fa_ == nullptr || ptr_ == nullptr) {
        throw invalid_iterator();
      }
      ptr_ = fa_->next_element(ptr_);
      return *this;
    }

    /**
     * TODO iter--
     */
    const_iterator operator--(int) {
      const_iterator temp = *this;
      if (fa_ == nullptr) {
        throw invalid_iterator();
      }
      if (ptr_ == nullptr) {
        ptr_ = fa_->previous_element(nullptr);
        if (ptr_ == nullptr) {
          throw invalid_iterator();
        }
        return temp;
      }
      tree_node *pre = fa_->previous_element(ptr_);
      if (pre == nullptr) {
        throw invalid_iterator();
      }
      ptr_ = pre;
      return temp;
    }

    /**
     * TODO --iter
     */
    const_iterator &operator--() {
      if (fa_ == nullptr) {
        throw invalid_iterator();
      }
      if (ptr_ == nullptr) {
        ptr_ = fa_->previous_element(nullptr);
        if (ptr_ == nullptr) {
          throw invalid_iterator();
        }
        return *this;
      }
      tree_node *pre = fa_->previous_element(ptr_);
      if (pre == nullptr) {
        throw invalid_iterator();
      }
      ptr_ = pre;
      return *this;
    }

    /**
     * a operator to check whether two iterators are same (pointing to the same memory).
     */
    const value_type &operator*() const {
      if (ptr_ == nullptr) {
        throw invalid_iterator();
      }
      return ptr_->keypair;
    }

    bool operator==(const iterator &rhs) const {
      return ptr_ == rhs.ptr_ && fa_ == rhs.fa_;
    }

    bool operator==(const const_iterator &rhs) const {
      return ptr_ == rhs.ptr_ && fa_ == rhs.fa_;
    }

    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator &rhs) const {
      return ptr_ != rhs.ptr_ || fa_ != rhs.fa_;
    }

    bool operator!=(const const_iterator &rhs) const {
      return ptr_ != rhs.ptr_ || fa_ != rhs.fa_;
    }

    /**
     * for the support of it->first.
     * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
     */
    const value_type *operator->() const
    {
      if (ptr_ == nullptr) {
        throw invalid_iterator();
      }
      return &(ptr_->keypair);
    }
  };

  /**
   * TODO two constructors
   */
  map() : root_(nullptr), size_(0), comp_() {}

  map(const map &other) : size_(other.size_), comp_() {
    if (other.root_ == nullptr) {
      root_ = nullptr;
      return;
    }
    root_ = new tree_node(*other.root_);
    copy(root_, other.root_);
  }

  /**
   * TODO assignment operator
   */
  map &operator=(const map &other) {
    if (this == &other) {
      return *this;
    }
    if (root_) {
      release(root_);
    }
    size_ = other.size_;
    if (other.root_ == nullptr) {
      root_ = nullptr;
      return *this;
    }
    root_ = new tree_node(*other.root_);
    copy(root_, other.root_);
    return *this;
  }

  /**
   * TODO Destructors
   */
  ~map() {
    if (root_) {
      release(root_);
    }
  }

  /**
   * TODO
   * access specified element with bounds checking
   * Returns a reference to the mapped value of the element with key equivalent to key.
   * If no such element exists, an exception of type `index_out_of_bound'
   */
  T &at(const Key &key) {
    tree_node *pos = find(key, root_);
    if (pos == nullptr) {
      throw index_out_of_bound();
    }
    return pos->keypair.second;
  }

  const T &at(const Key &key) const {
    tree_node *pos = find(key, root_);
    if (pos == nullptr) {
      throw index_out_of_bound();
    }
    return pos->keypair.second;
  }

  /**
   * TODO
   * access specified element
   * Returns a reference to the value that is mapped to a key equivalent to key,
   *   performing an insertion if such key does not already exist.
   */
  T &operator[](const Key &key) {
    tree_node *pos = find(key, root_);
    if (pos == nullptr) {
      insert(pair(key, T()), root_);
      return find(key, root_)->keypair.second;
    }
    return pos->keypair.second;
  }

  /**
   * behave like at() throw index_out_of_bound if such key does not exist.
   */
  const T &operator[](const Key &key) const {
    tree_node *pos = find(key, root_);
    if (pos == nullptr) {
      throw index_out_of_bound();
    }
    return pos->keypair.second;
  }

  /**
   * return a iterator to the beginning
   */
  iterator begin() {
    tree_node *temp = root_;
    if (temp == nullptr) {
      iterator it;
      it.fa_ = this;
      return it;
    }
    while (temp->ls) {
      temp = temp->ls;
    }
    iterator it;
    it.fa_ = this;
    it.ptr_ = temp;
    return it;
  }

  const_iterator cbegin() const {
    tree_node *temp = root_;
    if (temp == nullptr) {
      const_iterator it;
      it.fa_ = this;
      return it;
    }
    while (temp->ls) {
      temp = temp->ls;
    }
    const_iterator it;
    it.fa_ = this;
    it.ptr_ = temp;
    return it;
  }

  /**
   * return a iterator to the end
   * in fact, it returns past-the-end.
   */
  iterator end() {
    iterator it;
    it.fa_ = this;
    return it;
  }

  const_iterator cend() const {
    const_iterator it;
    it.fa_ = this;
    return it;
  }

  /**
   * checks whether the container is empty
   * return true if empty, otherwise false.
   */
  bool empty() const {
    return size_ == 0;
  }

  /**
   * returns the number of elements.
   */
  size_t size() const {
    return size_;
  }

  /**
   * clears the contents
   */
  void clear() {
    if (root_) {
      release(root_);
    }
    size_ = 0;
    root_ = nullptr;
  }

  /**
   * insert an element.
   * return a pair, the first of the pair is
   *   the iterator to the new element (or the element that prevented the insertion),
   *   the second one is true if insert successfully, or false.
   */
  pair<iterator, bool> insert(const value_type &value) {
    tree_node *pos = find(value.first, root_);
    if (pos) {
      iterator it;
      it.fa_ = this;
      it.ptr_ = pos;
      return pair(it, false);
    }
    insert(value, root_);
    pos = find(value.first, root_);
    iterator it;
    it.fa_ = this;
    it.ptr_ = pos;
    return pair(it, true);
  }

  /**
   * erase the element at pos.
   *
   * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
   */
  void erase(iterator pos) {
    if (pos.fa_ != this || pos.ptr_ == nullptr) {
      throw invalid_iterator();
    }
    erase(pos.ptr_->keypair.first, root_);
  }

  /**
   * Returns the number of elements with key
   *   that compares equivalent to the specified argument,
   *   which is either 1 or 0
   *     since this container does not allow duplicates.
   * The default method of check the equivalence is !(a < b || b > a)
   */
  size_t count(const Key &key) const {
    return find(key, root_) ? 1 : 0;
  }

  /**
   * Finds an element with key equivalent to key.
   * key value of the element to search for.
   * Iterator to an element with key equivalent to key.
   *   If no such element is found, past-the-end (see end()) iterator is returned.
   */
  iterator find(const Key &key) {
    tree_node *pos = find(key, root_);
    iterator it;
    it.fa_ = this;
    it.ptr_ = pos;
    return it;
  }

  const_iterator find(const Key &key) const {
    tree_node *pos = find(key, root_);
    const_iterator it;
    it.fa_ = this;
    it.ptr_ = pos;
    return it;
  }
};

}

#endif
