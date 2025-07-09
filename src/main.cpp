//use ctrl + shift + b to build and run using custom task.json
#include "server.hpp"

int main() {
    ProxyServer server(8080); // Run proxy on port 8080
    server.start();
    return 0;
}
