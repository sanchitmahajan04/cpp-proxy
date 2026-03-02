#pragma once
#include <string>

class ThreadProxy {
public:
    ThreadProxy(int port = 8080);
    ~ThreadProxy();
    void run();
private:
    int port_;
    static void handle_client(int client_socket, std::string client_ip);
};
