#ifndef SERVER_HPP
#define SERVER_HPP

class ProxyServer {
    private:
        int port_;
    public:
        ProxyServer(int port);

        void start();

};

#endif // SERVER_HPP
