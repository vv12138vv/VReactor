#include "net_address.h"
#include <cstring>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>



NetAddress::NetAddress(uint16_t port, const std::string& ip) {
    ::memset(&address_, 0, sizeof(address_));
    address_.sin_family = AF_INET;
    address_.sin_port = htons(port);
    if (ip.empty()) {
        address_.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_pton(AF_INET, ip.c_str(), &(address_.sin_addr));
    }
}


std::string NetAddress::get_ip() const {
    char buf[16] = {0};
    inet_ntop(AF_INET, &address_.sin_addr, buf, sizeof(buf));
    return buf;
}

uint16_t NetAddress::get_port() const {
    return ntohs(address_.sin_port);
}

std::string NetAddress::get_ip_port() const {
    return get_ip() + ':' + std::to_string(get_port());
}

void NetAddress::set_address(const sockaddr_in& sock_addr) {
    address_ = sock_addr;
}