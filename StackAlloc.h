/*-
 * Copyright (c) 2013 Cosku Acay, http://www.coskuacay.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/*-
 * A template class that implements a simple stack structure.
 * This demostrates how to use alloctor_traits (specifically with MemoryPool)
 */

#ifndef STACK_ALLOC_H
#define STACK_ALLOC_H

#include <memory>

/*
 * |           |
 * | +-------+ |
 * | | data  | |
 * | +-------+ |
 * | | prev  | |
 * | +-------+ |
 * |     |     |
 * |     ▼     |
 * | +-------+ |
 * | | data  | |
 * | +-------+ |
 * | | prev  | |
 * | +-------+ |
 * +-----------+
 */
template <typename T> struct StackNode_ {
  T data;
  StackNode_ *prev;
};

/*
 * |           |
 * | +-------+ |
 * | | data  | | <---- head
 * | +-------+ |
 * | | prev  | |
 * | +-------+ |
 * |     |     |
 * |     ▼     |
 * | +-------+ |
 * | | data  | |
 * | +-------+ |
 * | | prev  | |
 * | +-------+ |
 * +-----------+
 */
/** T is the object to store in the stack, Alloc is the allocator to use */
template <class T, class Alloc = std::allocator<T>> class StackAlloc {
public:
  typedef StackNode_<T> Node;
  /**
   * @note  由于我们传入的参数是`T`，但是我们容器中实际装的元素是Node<T>
   *        因此就需要`rebind`。
   *
   * 1.
   * 为了防止`rebind<`被识别为小于号，因此要注明这是一个模板，因此使用`Alloc::template`
   * 2. 为了标记`other`是一个类型名称，因此需要使用`typename`
   * 3. 最后，将这个类型标记称为`allocator`
   */
  typedef typename Alloc::template rebind<Node>::other allocator;

  /** Default constructor */
  StackAlloc() { head_ = 0; }
  /** Default destructor */
  ~StackAlloc() { clear(); }

  /**
   * @note 换做是我的话，我可能会使用`inline`，同时使用`return !head_;`
   */
  /** Returns true if the stack is empty */
  bool empty() { return (head_ == 0); }

  /**
   * @note 使用一个size来记录栈的元素个数可能会更好一些？
   *        同时，用0来代替`nullptr`可读性有点低了
   */
  /** Deallocate all elements and empty the stack */
  void clear() {
    Node *curr = head_;
    while (curr != 0) {
      Node *tmp = curr->prev;
      allocator_.destroy(curr);
      allocator_.deallocate(curr, 1);
      curr = tmp;
    }
    head_ = 0;
  }

  /** Put an element on the top of the stack */
  void push(T element) {
    // 申请内存
    Node *newNode = allocator_.allocate(1);
    // 构造
    allocator_.construct(newNode, Node());
    newNode->data = element;
    newNode->prev = head_;
    head_ = newNode;
  }

  /**
   * @note 备注：在一般的实现中，`pop`通常被设计为`void`，这一点需要注意
   */
  /** Remove and return the topmost element on the stack */
  T pop() {
    T result = head_->data;
    Node *tmp = head_->prev;
    allocator_.destroy(head_);
    allocator_.deallocate(head_, 1);
    head_ = tmp;
    return result;
  }

  /** Return the topmost element */
  T top() { return (head_->data); }

private:
  allocator allocator_;
  Node *head_;
};

#endif // STACK_ALLOC_H
