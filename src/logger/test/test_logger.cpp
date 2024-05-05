#include"logger.h"
#include"async_log.h"
#include <cassert>
#include <string>



void test_logger(){
    LOG_TRACE<<"test trace";
    LOG_DEBUG<<"test debug";
    LOG_INFO<<"test info";
    LOG_WARN<<"test warn";
    LOG_ERROR<<"test error";
}

AsyncLog* p=nullptr;


void async_output(const char* msg,size_t len){
    assert(p!=nullptr);
    p->append(msg, len);
}
void async_flush(){
    assert(p!=nullptr);
}

void test_async_logger(){
    for(int i=0;i<1000000;i+=1){
        LOG_INFO<<"logger:"<<std::to_string(i);
        if(i==999999){
            LOG_INFO<<"END";
        }
    }
}

int main(int argc,char* argv[]){
    AsyncLog async_logger(basename(argv[0]),3*1024*1024);
    p=&async_logger;
    Logger::set_output(async_output);
    async_logger.start();
    test_async_logger();
    return 0;
}