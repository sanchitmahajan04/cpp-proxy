#include "epoll/EpollProxy.h"
#include "common/Logger.h"
#include <iostream>

int main() {
    init_logger("proxy.log");
    EpollProxy proxy(8080);
    proxy.run();
    return 0;
}
