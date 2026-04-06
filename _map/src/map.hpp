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

  struct tree_node {
    pair<const Key, T> p;
    tree_node *parent, *left, *right;
    bool is_red;

    tree_node(const Key& k, const T& v, bool r = false):
      p(k, v), is_red(r), parent(nullptr), left(nullptr), right(nullptr) {}
  };

  tree_node *root_;
  size_t size_;
  Compare comp_;

  // RoL operation
  void rotate_left(tree_node *x) {
    tree_node *y = x->right;
    if (y == nullptr) {
      return;
    }
    tree_node *b = y->left;
    //      |
    //      X
    //     / \
    //    a   Y
    //       / \
    //      b   c
    y->left = x;
    x->right = b;
    if (b != nullptr) {
      b->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nullptr) {
      root_ = y;
    }
    else if (x == x->parent->left) {
      x->parent->left = y;
    }
    else {
      x->parent->right = y;
    }
    x->parent = y;
    //      |
    //      Y
    //     / \
    //    X   c
    //   / \
    //  a   b
  }

  // RoR operation
  void rotate_right(tree_node *x) {
    tree_node *y = x->left;
    if (y == nullptr) {
      return;
    }
    tree_node *b = y->right;
    //      |
    //      X
    //     / \
    //    Y   c
    //   / \
    //  a   b
    y->right = x;
    x->left = b;
    if (b != nullptr) {
      b->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nullptr) {
      root_ = y;
    }
    else if (x == x->parent->left) {
      x->parent->left = y;
    }
    else {
      x->parent->right = y;
    }
    x->parent = y;
    //      |
    //      Y
    //     / \
    //    a   X
    //       / \
    //      b   c
  }

  // find the node with the maximum key less than x's.
  tree_node *predecessor(tree_node *x) const {
    if (x->left != nullptr) {
      // if there is a left son, find the rightmost node in it
      tree_node *cur = x->left;
      while (cur->right != nullptr) {
        cur = cur->right;
      }
      return cur;
    }
    // otherwise, find the lowest ancestor that x is in its right subtree
    tree_node *cur = x;
    tree_node *fa = x->parent;
    while (fa != nullptr && cur == fa->left) {
      cur = fa;
      fa = cur->parent;
    }
    return fa;
  }

  // find the node with the minimum key greater than x's.
  tree_node *successor(tree_node *x) const {
    if (x->right != nullptr) {
      // if there is a right son, find the leftmost node in it
      tree_node *cur = x->right;
      while (cur->left != nullptr) {
        cur = cur->left;
      }
      return cur;
    }
    // otherwise, find the lowest ancestor that x is in its left subtree
    tree_node *cur = x;
    tree_node *fa = x->parent;
    while (fa != nullptr && cur == fa->right) {
      cur = fa;
      fa = cur->parent;
    }
    return fa;
  }

  // fix up node z after insertion
  void insert_fixup(tree_node *z) {
    if (z->parent == nullptr) {
      // z is currently the root
      // simply mark i black
      z->is_red = false;
      return;
    }
    while (z->parent != nullptr && z->parent->is_red) {
      tree_node *x = z->parent;
      // father is red, there must be a grandfather node
      if (x->parent == nullptr) {
        // this should not happen
        throw "rbtree invalid: root is red!";
      }
      if (x == x->parent->left) {
        // father is grandfather's left son
        // y is the uncle
        tree_node *y = x->parent->right;
        if (y != nullptr && y->is_red) {
          // there exists an uncle
          // uncle is also red, assign them black
          x->is_red = false;
          y->is_red = false;
          // assign the grandfather red
          x->parent->is_red = true;
          // maintain the grandparent
          z = x->parent;
        }
        else {
          // there is no uncle, or the uncle is black
          if (z == x->right) {
            // the current node is the right son
            //      G(Black)
            //     /
            //    X(Red)
            //     \
            //      Z(Red)
            // perform RoL on X
            rotate_left(x);
            //      G(Black)
            //     /
            //    Z(Red)
            //   /
            //  X(Red)
            // now the X and Z nodes need to swap their roles
            z = x;
            x = z->parent;
          }
          // the current node is the left son
          //      G(Black)
          //     /
          //    X(Red)
          //   /
          //  Z(Red)
          // perform RoR on G
          tree_node *g = x->parent;
          rotate_right(g);
          //       X(Red)
          //      / \
          //(Red)Z   G(Black)
          // re-color X and G
          x->is_red = false;
          g->is_red = true;
          //       X(Black)
          //      / \
          //(Red)Z   G(Red)
          // now we are done!
          break;
        }
      }
      else {
        // father is grandfather's right son
        // y is the uncle
        tree_node *y = x->parent->left;
        if (y != nullptr && y->is_red) {
          // there exists an uncle
          // uncle is also red, assign them black
          x->is_red = false;
          y->is_red = false;
          // assign the grandfather red
          x->parent->is_red = true;
          // maintain the grandparent
          z = x->parent;
        }
        else {
          // there is no uncle, or the uncle is black
          if (z == x->left) {
            // the current node is the left son
            //    G(Black)
            //     \
            //      X(Red)
            //     /
            //   Z(Red)
            // perform RoR on X
            rotate_right(x);
            //    G(Black)
            //     \
            //      Z(Red)
            //       \
            //        X(Red)
            // now the X and Z nodes need to swap their roles
            z = x;
            x = z->parent;
          }
          // the current node is the right son
          //    G(Black)
          //     \
          //      X(Red)
          //       \
          //        Z(Red)
          // perform RoL on G
          tree_node *g = x->parent;
          rotate_left(g);
          //         X(Red)
          //        / \
          //(Black)G   Z(Red)
          // re-color X and G
          x->is_red = false;
          g->is_red = true;
          //         X(Black)
          //        / \
          //  (Red)G   Z(Red)
          // now we are done!
          break;
        }
      }
    }
    // finally, ensure the root is black
    root_->is_red = false;
  }

  // insert node z into the tree
  void insert(tree_node *z) {
    z->is_red = true;
    tree_node *x = root_;
    tree_node *y = nullptr;
    while (x) {
      y = x;
      if (comp_(x->p.first, z->p.first)) {
        // current node's key < inserting node's key
        // insert the node to the right subtree
        x = x->right;
      }
      else {
        // insert the node to the left subtree
        x = x->left;
      }
    }
    // x is nullptr, do NOT dereference x!
    if (y == nullptr) {
      // the tree is empty
      root_ = z;
    }
    else if (comp_(y->p.first, z->p.first)) {
      // the leaf's key < inserting node's key
      // insert the node as the right son of the key
      y->right = z;
    }
    else {
      // insert the node as the left son of the key
      y->left = z;
    }
    z->parent = y;
    insert_fixup(z);
  }

  // transplant node v onto node u's position, replacing node u
  // note that the transplant includes v's whole subtree
  void transplant(tree_node *u, tree_node *v) {
    if (u == nullptr) {
      // you must not transplant onto a null pointer
      throw "transplanting onto null pointer!";
    }
    if (u->parent == nullptr) {
      // u is currently the root
      root_ = v;
    }
    else if (u == u->parent->left) {
      // u is the left son
      u->parent->left = v;
    }
    else {
      // u is the right son
      u->parent->right = v;
    }
    if (v == nullptr) {
      return;
    }
    // update v's parent
    v->parent = u->parent;
  }

  // fix up node x after deletion
  // p is x's parent
  void erase_fixup(tree_node *x, tree_node *p) {
    while (x != root_ && (x == nullptr || x->is_red == false)) {
      // x is a black node (or a null pointer)
      if (p == nullptr) {
        // this can only happen when the tree becomes empty
        break;
      }
      if (x == p->left) {
        // x is the left son
        // w is the sibling of x
        tree_node *w = p->right;
        if (w != nullptr && w->is_red) {
          // p is black, w is red
          //                  P(Black)
          //                 / \
          //  (Double Black)X   W(Red)
          //                   / \
          //           (Black)a   b(Black)
          w->is_red = false;
          p->is_red = true;
          rotate_left(p);
          //                  W(Black)
          //                 / \
          //           (Red)P   b(Black)
          //               / \
          //       (Black)X   a(Black)
          // after rotation, update sibling
          w = p->right;
        }
        bool w_left_black =
            (w == nullptr || w->left == nullptr || w->left->is_red == false);
        bool w_right_black =
            (w == nullptr || w->right == nullptr || w->right->is_red == false);
        if (w_left_black && w_right_black) {
          // sibling is black, and both nephews are black
          //                  P(?)
          //                 / \
          //  (Double Black)X   W(Black)
          //                   / \
          //           (Black)a   b(Black)
          if (w != nullptr) {
            w->is_red = true;
          }
          // pass the extra blackness to parent
          x = p;
          //              P(? + Black)
          //             / \
          //     (Black)X   W(Red)
          //               / \
          //       (Black)a   b(Black)
          p = x->parent;
        }
        else {
          if (w == nullptr) {
            // this branch is theoretically unreachable
            // but keep it for safety in null-based implementation
            x = p;
            p = x->parent;
            continue;
          }
          if (w->right == nullptr || w->right->is_red == false) {
            // sibling is black, near nephew is red, far nephew is black
            //                  P(?)
            //                 / \
            //  (Double Black)X   W(Black)
            //                   / \
            //             (Red)a   c(Black)
            //                 / \
            //         (Black)b   d(Black)
            if (w->left != nullptr) {
              w->left->is_red = false;
            }
            w->is_red = true;
            rotate_right(w);
            //                  P(?)
            //                 / \
            //  (Double Black)X   a(Red, new w)
            //                   / \
            //           (Black)b   W(Black)
            //                     / \
            //             (Black)d   c(Black)
            // needs further adjustment in the following steps
            w = p->right;
          }
          // sibling is black, and far nephew is red
          //                  P(?)
          //                 / \
          //  (Double Black)X   W(Black)
          //                   / \
          //               (?)a   b(Red)
          w->is_red = p->is_red;
          p->is_red = false;
          if (w->right != nullptr) {
            w->right->is_red = false;
          }
          rotate_left(p);
          //              W(?)
          //             / \
          //     (Black)P   b(Black)
          //           / \
          //   (Black)X   a(?)
          // fix is done
          x = root_;
          break;
        }
      }
      else {
        // x is the right son
        // w is the sibling of x
        // all the graphs are symmetric
        tree_node *w = p->left;
        if (w != nullptr && w->is_red) {
          // p is black, w is red
          w->is_red = false;
          p->is_red = true;
          rotate_right(p);
          // after rotation, update sibling
          w = p->left;
        }
        bool w_left_black =
            (w == nullptr || w->left == nullptr || w->left->is_red == false);
        bool w_right_black =
            (w == nullptr || w->right == nullptr || w->right->is_red == false);
        if (w_left_black && w_right_black) {
          // sibling is black, and both nephews are black
          if (w != nullptr) {
            w->is_red = true;
          }
          // pass the extra blackness to parent
          x = p;
          p = x->parent;
        }
        else {
          if (w == nullptr) {
            // this branch is theoretically unreachable,
            // but keep it for safety in null-based implementation
            x = p;
            p = x->parent;
            continue;
          }
          if (w->left == nullptr || w->left->is_red == false) {
            // sibling is black, near nephew is red, far nephew is black
            if (w->right != nullptr) {
              w->right->is_red = false;
            }
            w->is_red = true;
            rotate_left(w);
            w = p->left;
          }
          // sibling is black, and far nephew is red
          w->is_red = p->is_red;
          p->is_red = false;
          if (w->left != nullptr) {
            w->left->is_red = false;
          }
          rotate_right(p);
          // fix is done
          x = root_;
          break;
        }
      }
    }
    if (x != nullptr) {
      // ensure x is black after fixup
      x->is_red = false;
    }
    if (root_ != nullptr) {
      // ensure the root is black
      root_->is_red = false;
    }
  }

  // delete node z in the tree
  // note that this function just isolates node z out of the tree
  // manual memory free of node z is required
  void erase(tree_node *z) {
    if (z == nullptr) {
      // deleting a null pointer does not make any sense
      return;
    }
    // record the initial color
    bool original_color = z->is_red;
    // x is the child for replacement
    tree_node *x = nullptr;
    tree_node *p = nullptr;
    if (z->left == nullptr) {
      // z has no left children
      // transplant right child onto z
      tree_node *old_parent = z->parent;
      x = z->right;
      p = old_parent;
      transplant(z, z->right);
    }
    else if (z->right == nullptr) {
      // z has no right children
      // transplant left child onto z
      tree_node *old_parent = z->parent;
      x = z->left;
      p = old_parent;
      transplant(z, z->left);
    }
    else {
      // z has two children
      // transplant the successor onto z
      tree_node *y = successor(z);
      original_color = y->is_red;
      x = y->right;
      if (y->parent == z) {
        // y is z's right son
        // Z
        //  \
        //   Y
        //    \
        //     X
        p = y;
      }
      else {
        // y is somewhere deeper in z's right subtree
        //      Z
        //       \
        //      (...)
        //       /
        //      Y
        //       \
        //        X
        // first transplant y's right subtree onto y
        // this isolates y (note that y has no left subtree)
        p = y->parent;
        transplant(y, y->right);
        // the put y onto z's right subtree's parent
        y->right = z->right;
        z->right->parent = y;
        //      Z Y
        //       \|
        //      (...)
        //       /
        //      X
      }
      // then transplant y onto z, isolating z
      transplant(z, y);
      y->left = z->left;
      y->left->parent = y;
      y->is_red = z->is_red;
    }
    if (!original_color) {
      // we need to fix up if we deleted a black node
      erase_fixup(x, p);
    }
  }

  bool key_equal(const Key &a, const Key &b) const {
    return !comp_(a, b) && !comp_(b, a);
  }

  tree_node *find_node(const Key &key) const {
    tree_node *cur = root_;
    while (cur != nullptr) {
      if (comp_(key, cur->p.first)) {
        cur = cur->left;
      }
      else if (comp_(cur->p.first, key)) {
        cur = cur->right;
      }
      else {
        return cur;
      }
    }
    return nullptr;
  }

  tree_node *minimum(tree_node *x) const {
    if (x == nullptr) {
      return nullptr;
    }
    while (x->left != nullptr) {
      x = x->left;
    }
    return x;
  }

  tree_node *maximum(tree_node *x) const {
    if (x == nullptr) {
      return nullptr;
    }
    while (x->right != nullptr) {
      x = x->right;
    }
    return x;
  }

  void destroy(tree_node *x) {
    if (x == nullptr) {
      return;
    }
    destroy(x->left);
    destroy(x->right);
    delete x;
  }

  tree_node *clone(tree_node *x, tree_node *parent) {
    if (x == nullptr) {
      return nullptr;
    }
    tree_node *ret = new tree_node(x->p.first, x->p.second, x->is_red);
    ret->parent = parent;
    ret->left = clone(x->left, ret);
    ret->right = clone(x->right, ret);
    return ret;
  }

 public:
  /**
   * the internal type of data.
   * it should have a default constructor, a copy constructor.
   * You can use sjtu::map as value_type by typedef.
   */
  typedef pair<const Key, T> value_type;
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
      ptr_ = nullptr;
      fa_ = nullptr;
    }

    iterator(const iterator &other) {
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
      iterator tmp = *this;
      ptr_ = fa_->successor(ptr_);
      return tmp;
    }

    /**
     * TODO ++iter
     */
    iterator &operator++() {
      if (fa_ == nullptr || ptr_ == nullptr) {
        throw invalid_iterator();
      }
      ptr_ = fa_->successor(ptr_);
      return *this;
    }

    /**
     * TODO iter--
     */
    iterator operator--(int) {
      if (fa_ == nullptr) {
        throw invalid_iterator();
      }
      iterator tmp = *this;
      if (ptr_ == nullptr) {
        ptr_ = fa_->maximum(fa_->root_);
        if (ptr_ == nullptr) {
          throw invalid_iterator();
        }
        return tmp;
      }
      tree_node *pre = fa_->predecessor(ptr_);
      if (pre == nullptr) {
        throw invalid_iterator();
      }
      ptr_ = pre;
      return tmp;
    }

    /**
     * TODO --iter
     */
    iterator &operator--() {
      if (fa_ == nullptr) {
        throw invalid_iterator();
      }
      if (ptr_ == nullptr) {
        ptr_ = fa_->maximum(fa_->root_);
        if (ptr_ == nullptr) {
          throw invalid_iterator();
        }
        return *this;
      }
      tree_node *pre = fa_->predecessor(ptr_);
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
      if (fa_ == nullptr || ptr_ == nullptr) {
        throw invalid_iterator();
      }
      return ptr_->p;
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
      return !(*this == rhs);
    }

    bool operator!=(const const_iterator &rhs) const {
      return !(*this == rhs);
    }

    /**
     * for the support of it->first.
     * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
     */
    value_type *operator->() const
    noexcept {
      return &(ptr_->p);
    }
  };
  class const_iterator {
    // it should has similar member method as iterator.
    //  and it should be able to construct from an iterator.
   private:
    // data members.
    friend class map;
    friend class iterator;
    tree_node *ptr_;
    const map *fa_;
   public:
    const_iterator() {
      ptr_ = nullptr;
      fa_ = nullptr;
    }

    const_iterator(const const_iterator &other) {
      ptr_ = other.ptr_;
      fa_ = other.fa_;
    }

    const_iterator(const iterator &other) {
      ptr_ = other.ptr_;
      fa_ = other.fa_;
    }
    // And other methods in iterator.
    // And other methods in iterator.
    // And other methods in iterator.
    const_iterator operator++(int) {
      if (fa_ == nullptr || ptr_ == nullptr) {
        throw invalid_iterator();
      }
      const_iterator tmp = *this;
      ptr_ = fa_->successor(ptr_);
      return tmp;
    }

    const_iterator &operator++() {
      if (fa_ == nullptr || ptr_ == nullptr) {
        throw invalid_iterator();
      }
      ptr_ = fa_->successor(ptr_);
      return *this;
    }

    const_iterator operator--(int) {
      if (fa_ == nullptr) {
        throw invalid_iterator();
      }
      const_iterator tmp = *this;
      if (ptr_ == nullptr) {
        ptr_ = fa_->maximum(fa_->root_);
        if (ptr_ == nullptr) {
          throw invalid_iterator();
        }
        return tmp;
      }
      tree_node *pre = fa_->predecessor(ptr_);
      if (pre == nullptr) {
        throw invalid_iterator();
      }
      ptr_ = pre;
      return tmp;
    }

    const_iterator &operator--() {
      if (fa_ == nullptr) {
        throw invalid_iterator();
      }
      if (ptr_ == nullptr) {
        ptr_ = fa_->maximum(fa_->root_);
        if (ptr_ == nullptr) {
          throw invalid_iterator();
        }
        return *this;
      }
      tree_node *pre = fa_->predecessor(ptr_);
      if (pre == nullptr) {
        throw invalid_iterator();
      }
      ptr_ = pre;
      return *this;
    }

    const value_type &operator*() const {
      if (fa_ == nullptr || ptr_ == nullptr) {
        throw invalid_iterator();
      }
      return ptr_->p;
    }

    bool operator==(const iterator &rhs) const {
      return ptr_ == rhs.ptr_ && fa_ == rhs.fa_;
    }

    bool operator==(const const_iterator &rhs) const {
      return ptr_ == rhs.ptr_ && fa_ == rhs.fa_;
    }

    bool operator!=(const iterator &rhs) const {
      return !(*this == rhs);
    }

    bool operator!=(const const_iterator &rhs) const {
      return !(*this == rhs);
    }

    const value_type *operator->() const {
      if (fa_ == nullptr || ptr_ == nullptr) {
        throw invalid_iterator();
      }
      return &(ptr_->p);
    }
  };

  /**
   * TODO two constructors
   */
  map() : root_(nullptr), size_(0), comp_() {}

  map(const map &other) : root_(nullptr), size_(other.size_), comp_(other.comp_) {
    root_ = clone(other.root_, nullptr);
  }

  /**
   * TODO assignment operator
   */
  map &operator=(const map &other) {
    if (this == &other) {
      return *this;
    }
    clear();
    comp_ = other.comp_;
    size_ = other.size_;
    root_ = clone(other.root_, nullptr);
    return *this;
  }

  /**
   * TODO Destructors
   */
  ~map() {
    clear();
  }

  /**
   * TODO
   * access specified element with bounds checking
   * Returns a reference to the mapped value of the element with key equivalent to key.
   * If no such element exists, an exception of type `index_out_of_bound'
   */
  T &at(const Key &key) {
    tree_node *node = find_node(key);
    if (node == nullptr) {
      throw index_out_of_bound();
    }
    return node->p.second;
  }

  const T &at(const Key &key) const {
    tree_node *node = find_node(key);
    if (node == nullptr) {
      throw index_out_of_bound();
    }
    return node->p.second;
  }

  /**
   * TODO
   * access specified element
   * Returns a reference to the value that is mapped to a key equivalent to key,
   *   performing an insertion if such key does not already exist.
   */
  T &operator[](const Key &key) {
    tree_node *node = find_node(key);
    if (node != nullptr) {
      return node->p.second;
    }
    tree_node *z = new tree_node(key, T());
    insert(z);
    ++size_;
    return z->p.second;
  }

  /**
   * behave like at() throw index_out_of_bound if such key does not exist.
   */
  const T &operator[](const Key &key) const {
    return at(key);
  }

  /**
   * return a iterator to the beginning
   */
  iterator begin() {
    iterator it;
    it.fa_ = this;
    it.ptr_ = minimum(root_);
    return it;
  }

  const_iterator cbegin() const {
    const_iterator it;
    it.fa_ = this;
    it.ptr_ = minimum(root_);
    return it;
  }

  /**
   * return a iterator to the end
   * in fact, it returns past-the-end.
   */
  iterator end() {
    iterator it;
    it.fa_ = this;
    it.ptr_ = nullptr;
    return it;
  }

  const_iterator cend() const {
    const_iterator it;
    it.fa_ = this;
    it.ptr_ = nullptr;
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
    destroy(root_);
    root_ = nullptr;
    size_ = 0;
  }

  /**
   * insert an element.
   * return a pair, the first of the pair is
   *   the iterator to the new element (or the element that prevented the insertion),
   *   the second one is true if insert successfully, or false.
   */
  pair<iterator, bool> insert(const value_type &value) {
    tree_node *node = find_node(value.first);
    if (node != nullptr) {
      iterator it;
      it.fa_ = this;
      it.ptr_ = node;
      return pair<iterator, bool>(it, false);
    }
    tree_node *z = new tree_node(value.first, value.second);
    insert(z);
    ++size_;
    iterator it;
    it.fa_ = this;
    it.ptr_ = z;
    return pair<iterator, bool>(it, true);
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
    tree_node *node = pos.ptr_;
    erase(node);
    delete node;
    --size_;
  }

  /**
   * Returns the number of elements with key
   *   that compares equivalent to the specified argument,
   *   which is either 1 or 0
   *     since this container does not allow duplicates.
   * The default method of check the equivalence is !(a < b || b > a)
   */
  size_t count(const Key &key) const {
    return find_node(key) == nullptr ? 0 : 1;
  }

  /**
   * Finds an element with key equivalent to key.
   * key value of the element to search for.
   * Iterator to an element with key equivalent to key.
   *   If no such element is found, past-the-end (see end()) iterator is returned.
   */
  iterator find(const Key &key) {
    iterator it;
    it.fa_ = this;
    it.ptr_ = find_node(key);
    return it;
  }

  const_iterator find(const Key &key) const {
    const_iterator it;
    it.fa_ = this;
    it.ptr_ = find_node(key);
    return it;
  }
};

}

#endif