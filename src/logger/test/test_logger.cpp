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
    if(p!=nullptr){
        p->append(msg, len);
    }
}

void test_async_logger(){
    for(int i=0;i<10000000;i+=1){
        LOG_INFO<<"logger:"<<std::to_string(i);
    }
}

int main(int argc,char* argv[]){
    AsyncLog async_logger(basename(argv[0]),3*1024*1024);
    p=&async_logger;
    Logger::set_output(async_output);
    async_logger.start();
    test_async_logger();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    async_logger.stop();
    // return 0;
}