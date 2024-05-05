#include "memory_pool.h"
#include <cstddef>
#include <cstdlib>

const size_t MP_PAGE_SIZE = 4096;   // 4096Bytes,页大小
const size_t MP_ALIGNMENT = 16;     // 16Bytes,内存对齐

void MemoryPool::create_pool() {
    // 创建第一个SmallBlcok
    SmallBlock* new_block = nullptr;
#ifdef __linux__
    void* addr = aligned_alloc(align_, page_size_);
    if (addr == nullptr) {
        std::cerr << "create first new small block failed\n";
        return;
    }
    new_block = reinterpret_cast<SmallBlock*>(addr);
#elif _WIN32
    void* addr = _aligned_malloc(page_size_, align_);
    if (addr == nullptr) {
        std::cerr << "create first new small block failed\n";
        return;
    }
    new_block = reinterpret_cast<SmallBlock*>(addr);
#endif
    new_block->cur_usable_data_ = reinterpret_cast<byte*>(new_block) + sizeof(SmallBlock);
    new_block->data_end_ = reinterpret_cast<byte*>(new_block) + page_size_;
    new_block->fail_count_ = 0;
    new_block->quote_count_ = 0;
    new_block->next_ = nullptr;
    pool_->small_list_head_ = new_block;
    pool_->cur_block_ = new_block;
}

void* MemoryPool::vmalloc(size_t size) {
    if (size <= 0) {
        return nullptr;
    }
    size_t page_remnant = page_size_ - sizeof(SmallBlock);
    if (size > page_remnant) {
        // 分配大块内存
        return alloc_large_block(size);
    }
    byte* address = nullptr;
    SmallBlock* cur_block = pool_->cur_block_;
    while (cur_block != nullptr) {   // 遍历小内存节点，查找可以分配的位置
        // 将cur_node->last的指向处格式化为16的倍数
        address = reinterpret_cast<byte*>(align_ptr(cur_block->cur_usable_data_, align_));
        // 若该block的位置足够
        if (static_cast<size_t>(cur_block->data_end_ - address) >= size) {
            cur_block->quote_count_ += 1;
            cur_block->cur_usable_data_ = address + size;
            return address;
        }
        cur_block = cur_block->next_;
    }
    return alloc_small_block(size);
}

void* MemoryPool::alloc_small_block(size_t size) {
    SmallBlock* new_block = nullptr;
#ifdef linux
    void* addr = aligned_alloc(align_, page_size_);
    if (addr == nullptr) {
        std::cerr << "create new small block failed\n";
        return nullptr;
    }
    new_block = reinterpret_cast<SmallBlock*>(addr);
#elif _WIN32
    void* addr = _aligned_malloc(page_size_, align_);
    if (addr == nullptr) {
        std::cerr << "create new small block failed\n";
        return nullptr;
    }
    new_block = reinterpret_cast<SmallBlock*>(addr);
#endif
    // 新块的控制信息初始化
    new_block->data_end_ = reinterpret_cast<byte*>(new_block) + page_size_;
    new_block->next_ = nullptr;
    new_block->fail_count_ = 0;
    //数据区起始address
    byte* start_address = reinterpret_cast<byte*>(align_ptr(reinterpret_cast<byte*>(new_block) + sizeof(SmallBlock), align_));

    new_block->cur_usable_data_ = start_address + size;
    new_block->quote_count_ += 1;

    SmallBlock* cur_block = pool_->cur_block_;
    SmallBlock* cur = cur_block;
    while (cur->next_ != nullptr) {   //每次调用这个函数表示现有的内存块小块分配失败，所有小块的fail次数+1，表示不满足用户的需求
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

void* MemoryPool::alloc_large_block(size_t size) {
    // 申请数据空间
    byte* data = nullptr;
#ifdef linux
    void* address = aligned_alloc(align_, page_size_);
    if (address == nullptr) {
        std::cerr << "create new large block failed\n";
        return nullptr;
    }
    data = reinterpret_cast<byte*>(address);
#elif _WIN32
    void* address = _aligned_malloc(size, align_);
    if (address == nullptr) {
        std::cerr << "create new large block failed\n";
        return nullptr;
    }
    data = reinterpret_cast<byte*>(address);
#endif
    int count = 0;
    LargeBlock* large_block = pool_->large_list_head_;
    // 先在已有的大内存控制块中查找
    while (large_block != nullptr) {
        if (large_block->data_ == nullptr) {
            large_block->data_ = data;
            return data;
        }
        if (count > 3) {   // 防止过多的遍历
            break;
        }
        large_block = large_block->next_;
        count += 1;
    }
    // 分配大块控制块,大块控制块也位于smallblock的数据区中
    large_block = reinterpret_cast<LargeBlock*>(vmalloc(sizeof(LargeBlock)));
    if (large_block == nullptr) {
#ifdef __linux__
        free(data);
#elif _WIN32
        _aligned_free(data);
#endif
        std::cerr << "create large control block failed\n";
        return nullptr;
    }
    large_block->data_ = data;
    large_block->next_ = pool_->large_list_head_;
    pool_->large_list_head_ = large_block;
    return data;
}

void MemoryPool::free_block(void* p) {
    LargeBlock* large_block = pool_->large_list_head_;
    while (large_block != nullptr) {
        if (large_block->data_ == reinterpret_cast<byte*>(p)) {
#ifdef linux
            free(large_block->data_);
#elif _WIN32
            _aligned_free(large_block->data_);
#endif
            large_block->data_ = nullptr;
            return;
        }
        large_block = large_block->next_;
    }
}

void MemoryPool::reset_pool() {
    SmallBlock* small_block = pool_->small_list_head_;
    LargeBlock* large_block = pool_->large_list_head_;

    // 清空大块内存数据区
    while (large_block != nullptr) {
        if (large_block->data_ != nullptr) {
#ifdef __linux__
            free(large_block->data_);
#elif _WIN32
            _aligned_free(large_block->data_);
#endif
            large_block->data_ = nullptr;
        }
        large_block = large_block->next_;
    }
    pool_->large_list_head_ = nullptr;
    //清除小内存块区，只重置控制块信息，内部数据等待覆盖
    while (small_block != nullptr) {
        small_block->cur_usable_data_ = reinterpret_cast<byte*>(small_block) + sizeof(SmallBlock);
        small_block->fail_count_ = 0;
        small_block->quote_count_ = 0;
        small_block = small_block->next_;
    }
}

void MemoryPool::destroy_pool() {
    LargeBlock* large_block = pool_->large_list_head_;
    while (large_block != nullptr) {
        if (large_block->data_ != nullptr) {
#ifdef __linux__
            free(large_block->data_);
#elif _WIN32
            _aligned_free(large_block->data_);
#endif
            large_block->data_ = nullptr;
        }
        large_block = large_block->next_;
    }
    SmallBlock* small_block = pool_->small_list_head_;
    SmallBlock* temp = nullptr;
    while (small_block != nullptr) {
        temp = small_block->next_;
#ifdef linux
        free(small_block);
#elif _WIN32
        _aligned_free(small_block);
#endif
        small_block = temp;
    }
}