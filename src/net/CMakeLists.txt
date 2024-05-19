set(NET_SRC
    buffer.cpp
    channel.cpp
    event_loop.cpp
    ./poller/poller.cpp
    ./poller/epoller.cpp
    ./base/net_address.cpp
    ./base/socket.cpp)

add_library(net_lib STATIC ${NET_SRC})

add_subdirectory(./test)
