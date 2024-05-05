/*
 * @author vv12138vv
 * @description:
 * 参考nginx的一个服务于应用层的内存池，为每个连接提供服务，即一个连接对应一个内存池。
 *
 *
 */
#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <cstddef>
#include <cstdlib>
#include <iostream>


using byte = unsigned char;

extern const size_t MP_PAGE_SIZE;   // 4096Bytes,页大小
extern const size_t MP_ALIGNMENT;   // 16Bytes,内存对齐
// const size_t MP_MAX_SMALL_BLOCK = MP_PAGE_SIZE - 1;

// 小内存块管理信息
struct SmallBlock {
    byte* cur_usable_data_;   // 指向当前可用的数据区的首地址
    byte* data_end_;          // 指向数据区的末尾,开区间
    SmallBlock* next_;        // 链表指针
    size_t fail_count_;       // 分配失败计数
    size_t quote_count_;      // 引用计数
};

struct LargeBlock {
    byte* data_;         // data区
    LargeBlock* next_;   // 链表指针
};

struct Pool {
    LargeBlock* large_list_head_;
    SmallBlock* small_list_head_;
    SmallBlock* cur_block_;   // 指向可分配的SmallNode
};

class MemoryPool {
private:
    Pool* pool_;
    size_t page_size_;
    size_t align_;

    void create_pool();

    void destroy_pool();

    void* alloc_large_block(size_t size);

    void* alloc_small_block(size_t size);

public:
    explicit MemoryPool(size_t page_size = MP_PAGE_SIZE, size_t align = MP_ALIGNMENT)
        : page_size_(page_size)
        , align_(align) {
        pool_ = reinterpret_cast<Pool*>(malloc(sizeof(Pool)));
        pool_->large_list_head_ = nullptr;
        pool_->cur_block_ = nullptr;
        pool_->small_list_head_ = nullptr;
        create_pool();
    }

    ~MemoryPool() {
        destroy_pool();
        free(pool_);
    }

    void* vmalloc(size_t size);

    void free_block(void* p);

    void reset_pool();

    // 将指针对齐为align的整数倍
    static void* align_ptr(void* p, size_t align) { return reinterpret_cast<void*>((((size_t)p + (align - 1)) & ~(align - 1))); }

    static size_t align(size_t n, size_t align) { return ((n + (align - 1)) & ~(align - 1)); }
    const Pool* get_pool() const { return const_cast<const Pool*>(pool_); }
};

#endif