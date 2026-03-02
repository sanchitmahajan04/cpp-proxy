#pragma once

class EpollProxy {
public:
    EpollProxy(int port = 8080);
    ~EpollProxy();
    void run();
private:
    int port_;
};
