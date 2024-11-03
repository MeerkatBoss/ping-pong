#include "TcpClient.hpp"
#include "TcpServer.hpp"
#include "UdpClient.hpp"
#include "UdpServer.hpp"
#include <cstring>
#include <iostream>

int main(int argc, char** argv)
{
  if (argc != 2) {
    std::cout
      << "Usage:\n"
      << "ping-pong tcp-server|tcp-client|udp-server|udp-client\n";  
    return 1;
  }

  uint8_t addr[4] = {127, 0, 0, 1};
  if (strcmp("tcp-server", argv[1]) == 0) {
    listen_tcp(addr, 8080);
    return 0;
  }

  if (strcmp("tcp-client", argv[1]) == 0) {
    connect_tcp(addr, 8080);
    return 0;
  }
  if (strcmp("udp-server", argv[1]) == 0) {
    listen_udp(addr, 8080);
    return 0;
  }

  if (strcmp("udp-client", argv[1]) == 0) {
    connect_udp(addr, 8080);
    return 0;
  }

  std::cout
    << "Usage:\n"
    << "ping-pong tcp-server|tcp-client|udp-server|udp-client\n";  
  return 1;

  return 0;
}
