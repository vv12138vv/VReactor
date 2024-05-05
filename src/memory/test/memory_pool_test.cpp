#include "memory_pool.h"
#include<string>
#include<vector>
#include<chrono>


std::string info(const MemoryPool& memory_pool){
    std::string res;
    auto pool=memory_pool.get_pool();
    int i=1;
    res+="==============SmallBlock==============\n";
    for(auto it=pool->small_list_head_;it!= nullptr;it=it->next_){
        std::string part;
        size_t used=it->cur_usable_data_-(reinterpret_cast<byte*>(it)+sizeof (SmallBlock));
        size_t remnt=it->data_end_-it->cur_usable_data_;
        part+="the "+std::to_string(i)+" block: used: "+std::to_string(used)+" remnant: "+std::to_string(remnt);
        i+=1;
        res+=part+'\n';
    }
    i=1;
    res+="==============LargeBlock==============\n";
    for(auto it=pool->large_list_head_;it!= nullptr;it=it->next_){
        if(it->data_!= nullptr){
            std::string part;
            part+="the "+std::to_string(i)+" block";
            res+=part+'\n';
        }
        i+=1;
    }
    return res;
}

void test(){
    std::cout<<"sizeof(Pool):"<<sizeof(Pool)<<" sizeof(SmallBlock):"<<sizeof(SmallBlock)<<std::endl;
    std::vector<void*> memories(50);
    MemoryPool memory_pool;
    for(int i=0;i<50;i+=1){
        memories[i]=memory_pool.vmalloc(512);
    }
    std::cout<<info(memory_pool)<<std::endl;
    for(int i=0;i<50;i+=1){
        memory_pool.free_block(memories[i]);
    }
    std::cout<<info(memory_pool)<<std::endl;
    for(int i=0;i<50;i+=1){
        memories[i]=memory_pool.vmalloc(5120);
    }
    std::cout<<info(memory_pool)<<std::endl;
    for(int i=0;i<50;i+=1){
        memory_pool.free_block(memories[i]);
    }
    std::cout<<info(memory_pool)<<std::endl;
}

void test2(){
    const int alloc_count=10000;
    const int alloc_size=16;
    void* memory[alloc_count];
    auto start=std::chrono::steady_clock::now();
    for(int i=0;i<alloc_count;i+=1){
        memory[i]= malloc(alloc_size);
    }
    auto end=std::chrono::steady_clock::now();
    std::cout<<"malloc time:"<<std::chrono::duration_cast<std::chrono::microseconds>(end-start).count()<<"us"<<std::endl;

    start=std::chrono::steady_clock::now();
    for(int i=0;i<alloc_count;i+=1){
        free(memory[i]);
    }
    end=std::chrono::steady_clock::now();
    std::cout<<"free time:"<<std::chrono::duration_cast<std::chrono::microseconds>(end-start).count()<<"us"<<std::endl;

    MemoryPool memory_pool;

    start=std::chrono::steady_clock::now();
    for(int i=0;i<alloc_count;i+=1){
        memory[i]=memory_pool.vmalloc(alloc_size);
    }
    end=std::chrono::steady_clock::now();
    std::cout<<"vmalloc time:"<<std::chrono::duration_cast<std::chrono::microseconds>(end-start).count()<<"us"<<std::endl;


    start=std::chrono::steady_clock::now();
    for(int i=0;i<alloc_count;i+=1){
        memory_pool.free_block(memory[i]);
    }
    end=std::chrono::steady_clock::now();
    std::cout<<"free_block time:"<<std::chrono::duration_cast<std::chrono::microseconds>(end-start).count()<<"us"<<std::endl;

}

int main(){
    test2();

}