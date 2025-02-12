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

#ifndef MEMORY_BLOCK_TCC
#define MEMORY_BLOCK_TCC

/**
 * @note 内存对齐
 *
 * 作者使用每一个块的前8个字节存放指针，指向前一块内存
 * 这个函数就是为了对齐内存，从而使剩下的内存能够合适得
 * 存放对象
 *
 * @return 指针偏移量，告诉数据应当在一个偏移量之后
 * 开始存储
 */
template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::size_type
MemoryPool<T, BlockSize>::padPointer(data_pointer_ p,
                                     size_type align) const noexcept {
  uintptr_t result = reinterpret_cast<uintptr_t>(p);
  return ((align - result) % align);
}

/**
 * @note 既然这个类没有任何的虚函数，也不是虚继承，甚至不是派生类
 *        用`memcpy`显然效果会更好一些
 */
template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool() noexcept {
  currentBlock_ = nullptr;
  currentSlot_ = nullptr;
  lastSlot_ = nullptr;
  freeSlots_ = nullptr;
}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool &memoryPool) noexcept
    : MemoryPool() {}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(MemoryPool &&memoryPool) noexcept {
  currentBlock_ = memoryPool.currentBlock_;
  memoryPool.currentBlock_ = nullptr;
  currentSlot_ = memoryPool.currentSlot_;
  lastSlot_ = memoryPool.lastSlot_;
  freeSlots_ = memoryPool.freeSlots;
}

template <typename T, size_t BlockSize>
template <class U>
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool<U> &memoryPool) noexcept
    : MemoryPool() {}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize> &
MemoryPool<T, BlockSize>::operator=(MemoryPool &&memoryPool) noexcept {
  if (this != &memoryPool) {
    std::swap(currentBlock_, memoryPool.currentBlock_);
    currentSlot_ = memoryPool.currentSlot_;
    lastSlot_ = memoryPool.lastSlot_;
    freeSlots_ = memoryPool.freeSlots;
  }
  return *this;
}

/**
 * @note 作者似乎只在析构函数中释放了没有分配的内存
 */
template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::~MemoryPool() noexcept {
  slot_pointer_ curr = currentBlock_;
  while (curr != nullptr) {
    slot_pointer_ prev = curr->next;
    operator delete(reinterpret_cast<void *>(curr));
    curr = prev;
  }
}

template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::pointer
MemoryPool<T, BlockSize>::address(reference x) const noexcept {
  return &x;
}

template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::const_pointer
MemoryPool<T, BlockSize>::address(const_reference x) const noexcept {
  return &x;
}

template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::allocateBlock() {
  // Allocate space for the new block and store a pointer to the previous one
  /*
   * @note newBlock是很大的一片内存，这里用char*(data_pointer_)来记录首地址
   */
  data_pointer_ newBlock =
      reinterpret_cast<data_pointer_>(operator new(BlockSize));
  // 在申请的新块中记录当前块的地址
  reinterpret_cast<slot_pointer_>(newBlock)->next = currentBlock_;
  // 更新当前块指针，使其指向新申请的内存
  currentBlock_ = reinterpret_cast<slot_pointer_>(newBlock);
  // Pad block body to staisfy the alignment requirements for elements
  /**
   * @note 真正的数据从newBlock + sizeof(slot_pointer_)开始记录
   *
   * 那么之前的地址用来干什么了？用来记录前一个块的地址，方便释放
   */
  data_pointer_ body = newBlock + sizeof(slot_pointer_);
  // 计算偏移量
  size_type bodyPadding = padPointer(body, alignof(slot_type_));
  // 指向能用的第一块内存
  currentSlot_ = reinterpret_cast<slot_pointer_>(body + bodyPadding);
  // 指向当前块能用的最后一块内存
  lastSlot_ = reinterpret_cast<slot_pointer_>(newBlock + BlockSize -
                                              sizeof(slot_type_) + 1);
}

template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::pointer
MemoryPool<T, BlockSize>::allocate(size_type n, const_pointer hint) {
  if (freeSlots_ != nullptr) {
    pointer result = reinterpret_cast<pointer>(freeSlots_);
    freeSlots_ = freeSlots_->next;
    return result;
  } else {
    if (currentSlot_ >= lastSlot_)
      allocateBlock();
    return reinterpret_cast<pointer>(currentSlot_++);
  }
}

template <typename T, size_t BlockSize>
inline void MemoryPool<T, BlockSize>::deallocate(pointer p, size_type n) {
  if (p != nullptr) {
    reinterpret_cast<slot_pointer_>(p)->next = freeSlots_;
    freeSlots_ = reinterpret_cast<slot_pointer_>(p);
  }
}

template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::size_type
MemoryPool<T, BlockSize>::max_size() const noexcept {
  size_type maxBlocks = -1 / BlockSize;
  return (BlockSize - sizeof(data_pointer_)) / sizeof(slot_type_) * maxBlocks;
}

template <typename T, size_t BlockSize>
template <class U, class... Args>
inline void MemoryPool<T, BlockSize>::construct(U *p, Args &&...args) {
  /**
   * @note placement new
   */
  new (p) U(std::forward<Args>(args)...);
}

template <typename T, size_t BlockSize>
template <class U>
inline void MemoryPool<T, BlockSize>::destroy(U *p) {
  p->~U();
}

template <typename T, size_t BlockSize>
template <class... Args>
inline typename MemoryPool<T, BlockSize>::pointer
MemoryPool<T, BlockSize>::newElement(Args &&...args) {
  pointer result = allocate();
  construct<value_type>(result, std::forward<Args>(args)...);
  return result;
}

template <typename T, size_t BlockSize>
inline void MemoryPool<T, BlockSize>::deleteElement(pointer p) {
  if (p != nullptr) {
    p->~value_type();
    deallocate(p);
  }
}

#endif // MEMORY_BLOCK_TCC
