#ifndef SERVER_HPP
#define SERVER_HPP

class ProxyServer {
public:
    ProxyServer(int port);
    void start();
private:
    int port_;
};

#endif // SERVER_HPP
