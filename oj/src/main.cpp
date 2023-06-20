#include "OJServer.h"

int main()
{
    EventLoop loop;
    OJServer server(&loop, InetAddress(8080), "OJServer");
    server.run();
    loop.loop();
    return 0;
}