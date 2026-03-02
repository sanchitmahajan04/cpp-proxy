#include "thread/ThreadProxy.h"
#include "common/Logger.h"
#include <iostream>

int main() {
    init_logger("proxy.log");
    ThreadProxy proxy(8080);
    proxy.run();
    return 0;
}
