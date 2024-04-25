#include "memory.h"
#include <cstddef>
#include <cstdlib>
#include <iostream>

// void MemoryPool::create_pool() {
//   pool_ = nullptr;
//   // 请求内存页
//   //  int res=posix_memalign((void**)&pool_,MP_ALIGNMENT,page_size_);
//   //  if(res!=0){
//   //      std::cerr<<"creat pool failed\n";
//   //      return;
//   //  }

//   // 大块节点待需要时再分配
//   pool_->large_list_head_ = nullptr;
//   // 初始化第一个SmallNode
//   pool_->small_list_head_ = reinterpret_cast<SmallBlock *>(
//       reinterpret_cast<byte *>(pool_) + sizeof(Pool));
//   pool_->small_list_head_->cur_usable_data_ =
//       reinterpret_cast<byte *>(pool_) + sizeof(Pool) + sizeof(SmallBlock);
//   pool_->small_list_head_->data_end_ =
//       reinterpret_cast<byte *>(pool_) + page_size_;
//   pool_->small_list_head_->fail_count_ = 0;
//   pool_->small_list_head_->quote_count_ = 0;
//   pool_->cur_block_ = pool_->small_list_head_;
//   return;
// }

void MemoryPool::create_pool() {
  // 创建第一个SmallBlcok
  SmallBlock *new_block = nullptr;
#ifdef __linux__
  int ret = posix_memalign((void **)&new_block, align_, page_size_);
  if (ret != 0) {
    std::cerr << "create first new small block failed\n";
    return;
  }
#elif _WIN32
  void *addr = _aligned_malloc(page_size_, align_);
  if (addr == nullptr) {
    std::cerr << "create first new small block failed\n";
    return;
  }
  new_block = reinterpret_cast<SmallBlock *>(addr);
#endif
  new_block->cur_usable_data_ =
      reinterpret_cast<byte *>(new_block) + sizeof(SmallBlock);
  new_block->data_end_ = reinterpret_cast<byte *>(new_block) + page_size_;
  new_block->fail_count_ = 0;
  new_block->quote_count_ = 0;
  new_block->next_ = nullptr;
  pool_->small_list_head_ = new_block;
  pool_->cur_block_ = new_block;
}

void *MemoryPool::malloc(size_t size) {
  if (size <= 0) {
    return nullptr;
  }
  size_t page_remnant = page_size_ - sizeof(SmallBlock);
  if (size > page_remnant) {
    // 分配大块内存
    return alloc_large_block(size);
  }
  byte *address = nullptr;
  SmallBlock *cur_block = pool_->cur_block_;
  while (cur_block != nullptr) { // 遍历小内存节点，查找可以分配的位置
    // 将cur_node->last的指向处格式化为16的倍数
    //  address=reinterpret_cast<byte*>(mp_align_ptr(cur_block->cur_usable_data_,MP_ALIGNMENT));

    // 若该node的位置足够
    if (cur_block->data_end_ - address >= size) {
      cur_block->quote_count_ += 1;
      cur_block->cur_usable_data_ = address + size;
      return address;
    }
    cur_block = cur_block->next_;
  }
  return alloc_small_block(size);
}

void *MemoryPool::alloc_small_block(size_t size) {
  SmallBlock *new_block = nullptr;
#ifdef linux
  int ret =
      posix_memalign(reinterpret_cast<void **>(&new_block), align_, page_size_);
  if (ret != 0) {
    std::cerr << "alloc new small block failed\n";
    return nullptr;
  }
#elif _WIN32
  void *addr = _aligned_malloc(page_size_, align_);
  if (addr == nullptr) {
    std::cerr << "create first new small block failed\n";
    return nullptr;
  }
  new_block = reinterpret_cast<SmallBlock *>(addr);
#endif
  // 新块的控制信息初始化
  new_block->data_end_ = reinterpret_cast<byte *>(new_block) + page_size_;
  new_block->next_ = nullptr;
  new_block->fail_count_ = 0;
  // todo 内存分配函数
  byte *start_address = reinterpret_cast<byte *>(
      mp_align_ptr(page + sizeof(SmallBlock), MP_ALIGNMENT));

  new_block->cur_usable_data_ = start_address + size;
  new_block->quote_count_ += 1;

  SmallBlock *cur_block = pool_->cur_block_;
  SmallBlock *cur = cur_block;
  while (cur->next_ != nullptr) {
    if (cur->fail_count_ >= 5) {
      cur_block = cur->next_;
    }
    cur->fail_count_ += 1;
    cur = cur->next_;
  }
  cur->next_ = new_block;
  pool_->cur_block_ = cur_block;

  return start_address;
}

void *MemoryPool::alloc_large_block(size_t size) {
  // 申请数据空间
  byte *address = nullptr;
  int ret = posix_memalign((void **)&address, MP_ALIGNMENT, size);
  if (ret != 0) {
    std::cerr << "allock new large block failed\n";
    return nullptr;
  }
  int count = 0;
  LargeBlock *large_block = pool_->large_list_head_;
  // 先在已有的大内存控制块中查找
  while (large_block != nullptr) {
    if (large_block->data_ == nullptr) {
      large_block->size_ = size;
      large_block->data_ = address;
      return address;
    }
    if (count > 3) { // 防止过多的遍历
      break;
    }
    large_block = large_block->next_;
    count += 1;
  }
  // 分配大块控制块,大块控制块也位于smallblock的数据区中
  large_block = reinterpret_cast<LargeBlock *>(malloc(sizeof(LargeBlock)));
  if (large_block == nullptr) {
    free(address);
    return nullptr;
  }
  large_block->size_ = size;
  large_block->data_ = address;
  large_block->next_ = pool_->large_list_head_;
  pool_->large_list_head_ = large_block;
  return address;
}

void MemoryPool::free_block(void *p) {
  LargeBlock *large_block = pool_->large_list_head_;
  while (large_block != nullptr) {
    if (large_block->data_ == p) {
      free(large_block->data_);
      large_block->size_ = 0;
      large_block->data_ = nullptr;
      return;
    }
    large_block = large_block->next_;
  }
}

void MemoryPool::reset_pool() {
  SmallBlock *small_block = pool_->small_list_head_;
  LargeBlock *large_block = pool_->large_list_head_;

  // 清空大块内存数据区
  while (large_block != nullptr) {
    if (large_block->data_ != nullptr) {
      free(large_block->data_);
      large_block->data_ = nullptr;
    }
    large_block = large_block->next_;
  }
  pool_->large_list_head_ = nullptr;
  while (small_block != nullptr) {
    small_block->cur_usable_data_ =
        reinterpret_cast<byte *>(small_block) + sizeof(SmallBlock);
    small_block->fail_count_ = 0;
    small_block->quote_count_ = 0;
    small_block = small_block->next_;
  }
}

void MemoryPool::destroy_pool() {
  LargeBlock *large_block = pool_->large_list_head_;
  while (large_block != nullptr) {
    if (large_block->data_ != nullptr) {
      free(large_block->data_);
    }
    large_block = large_block->next_;
  }
  SmallBlock *small_block = pool_->small_list_head_->next_;
  SmallBlock *temp = nullptr;
  while (small_block != nullptr) {
    temp = small_block->next_;
    free(small_block);
    small_block = temp;
  }
  free(pool_);
}