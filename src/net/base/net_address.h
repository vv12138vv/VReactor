#ifndef NET_ADDRESS_H
#define NET_ADDRESS_H

#include <arpa/inet.h>
#include <cstdint>
#include <netinet/in.h>
#include <string>

class NetAddress {
private:
    sockaddr_in address_;

public:
    NetAddress() = delete;
    NetAddress(uint16_t port, const std::string& ip = "");
    explicit NetAddress(const sockaddr_in& sock_addr)
        : address_(sock_addr) {}

    uint16_t get_port();
    std::string get_ip();
    std::string get_ip_port();
    void set_address(const sockaddr_in& sock_addr);
    const sockaddr_in* get_sockaddr() const { return &address_; }
};



#endif