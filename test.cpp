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
 * Provided to compare the default allocator to MemoryPool
 *
 * Check out StackAlloc.h for a stack implementation that takes an allocator as
 * a template argument. This may give you some idea about how to use MemoryPool.
 *
 * This code basically creates two stacks: one using the default allocator and
 * one using the MemoryPool allocator. It pushes a bunch of objects in them and
 * then pops them out. We repeat the process several times and time how long
 * this takes for each of the stacks.
 *
 * Do not forget to turn on optimizations (use -O2 or -O3 for GCC). This is a
 * benchmark, we want inlined code.
 */

#include <cassert>
#include <ctime>
#include <iostream>
#include <vector>

#if not defined(_C98) && not defined(_C11)
#define _C98
#endif

#ifdef _C98
#include "C-98/MemoryPool.h"
#endif
#ifdef _C11
#include "C-11/MemoryPool.h"
#endif
#include "StackAlloc.h"

/* Adjust these values depending on how much you trust your computer */
/**
 * @note 每一次迭代压栈的次数
 *
 * 这个值不要设置太大，因为这个测试程序是单线程的，就算计算机的性能很好，
 * 也会被如此大的计算量搞得计算时间很长
 */
#define ELEMS 1000000
/**
 * @note REPS = repeats
 */
#define REPS 50

int main() {
  /**
   * @note 作者在一开始就声明了计时器但没有初始化。
   *
   * 可能的一种想法是认为到需要的时候再去声明会浪费额外的时间。
   * 对汇编底层来说是不存在的问题。所以为了代码的可读性，通常在需要的时候
   * 再做声明
   */
  clock_t start;

  std::cout << "Copyright (c) 2013 Cosku Acay, http://www.coskuacay.com\n";
  std::cout << "Provided to compare the default allocator to MemoryPool.\n\n";

  /**
   * @note 压力测试代码
   *
   * 主要通过的方法就是，压栈`ELEMS`次，再弹栈`ELEMS`次，循环往复`REPS`次
   */
  /* Use the default allocator */
  StackAlloc<int, std::allocator<int>> stackDefault;
  start = clock();
  for (int j = 0; j < REPS; j++) {
    assert(stackDefault.empty());
    for (int i = 0; i < ELEMS / 4; i++) {
      // Unroll to time the actual code and not the loop
      stackDefault.push(i);
      stackDefault.push(i);
      stackDefault.push(i);
      stackDefault.push(i);
    }
    for (int i = 0; i < ELEMS / 4; i++) {
      // Unroll to time the actual code and not the loop
      stackDefault.pop();
      stackDefault.pop();
      stackDefault.pop();
      stackDefault.pop();
    }
  }
  std::cout << "Default Allocator Time: ";
  std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

  /* Use MemoryPool */
  StackAlloc<int, MemoryPool<int>> stackPool;
  start = clock();
  for (int j = 0; j < REPS; j++) {
    assert(stackPool.empty());
    for (int i = 0; i < ELEMS / 4; i++) {
      // Unroll to time the actual code and not the loop
      stackPool.push(i);
      stackPool.push(i);
      stackPool.push(i);
      stackPool.push(i);
    }
    for (int i = 0; i < ELEMS / 4; i++) {
      // Unroll to time the actual code and not the loop
      stackPool.pop();
      stackPool.pop();
      stackPool.pop();
      stackPool.pop();
    }
  }
  std::cout << "MemoryPool Allocator Time: ";
  std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

  std::cout << "Here is a secret: the best way of implementing a stack"
               " is a dynamic array.\n";

  /* Compare MemoryPool to std::vector */
  std::vector<int> stackVector;
  start = clock();
  for (int j = 0; j < REPS; j++) {
    assert(stackVector.empty());
    for (int i = 0; i < ELEMS / 4; i++) {
      // Unroll to time the actual code and not the loop
      stackVector.push_back(i);
      stackVector.push_back(i);
      stackVector.push_back(i);
      stackVector.push_back(i);
    }
    for (int i = 0; i < ELEMS / 4; i++) {
      // Unroll to time the actual code and not the loop
      stackVector.pop_back();
      stackVector.pop_back();
      stackVector.pop_back();
      stackVector.pop_back();
    }
  }
  std::cout << "Vector Time: ";
  std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

  std::cout << "The vector implementation will probably be faster.\n\n";
  std::cout
      << "MemoryPool still has a lot of uses though. Any type of tree"
         " and when you have multiple linked lists are some examples (they"
         " can all share the same memory pool).\n";

  return 0;
}
