#include <iostream>
#include "Timestamp.h"
#include "InetAddress.h"

int main(int argc, const char** argv) {
    std::cout << "fuck off!" << std::endl;
    std::cout << Timestamp::now().toString() << std::endl;
    InetAddress addr(8080);
    std::cout << addr.toIpPort() << std::endl;
    std::cout << addr.toIp() << std::endl;
    std::cout << addr.toPort() << std::endl;
    return 0;
}