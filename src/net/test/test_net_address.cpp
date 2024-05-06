#include "net_address.h"
#include <iostream>
#include <iterator>
#include<assert.h>
#include <netinet/in.h>

int main() {
    NetAddress net_address(4040);
    std::cout << net_address.get_ip() << std::endl;
    std::cout << net_address.get_port() << std::endl;
    std::cout << net_address.get_ip_port() << std::endl;
    assert(net_address.get_ip()=="127.4.3.1");
    assert(net_address.get_port()==4040);
    assert(net_address.get_ip_port()=="127.4.3.1:4040");
    return 0;
}