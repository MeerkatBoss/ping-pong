#include "UdpClient.hpp"
#include "Scream.hpp"
#include "Command.hpp"

#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cassert>

#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>

static void send_stop(int socket, const struct sockaddr_in* addr);
static int send_ping(int socket, const struct sockaddr_in* addr);
static int send_scream(int socket, const struct sockaddr_in* addr);
static int receive_response(int socket, const struct sockaddr_in* addr);

static int make_socket(uint8_t ip_address[4], uint16_t port);

void connect_udp(uint8_t ip_address[4], uint16_t port) {
  int server = make_socket(ip_address, port + 1);

  constexpr size_t IpAddrMaxLength = 12 + 3;  // 12 digits and 3 dots
  static char addr_buffer[IpAddrMaxLength + 1] = "";
  int res = 0;

  snprintf(addr_buffer, IpAddrMaxLength,
      "%hhd.%hhd.%hhd.%hhd",
      ip_address[0], ip_address[1], ip_address[2], ip_address[3]
  );

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = port;
  res = inet_aton(addr_buffer, &address.sin_addr);
  assert(res == 1);

  Command command;
  while (std::cin >> command) {
    int res = 0;
    bool stopped = false;
    bool has_response = false;

    switch (command) {
    case Command::Unknown: 
      puts("Invalid command");
      break;
    case Command::Stop:
      send_stop(server, &address);
      stopped = true;
      break;
    case Command::Ping:
      res = send_ping(server, &address);
      has_response = true;
      break;
    case Command::Scream:
      res = send_scream(server, &address);
      has_response = true;
      break;
    default:
      break;
    }

    if (stopped) {
      break;
    }

    if (res < 0) {
      break;
    }

    if (has_response) {
      res = receive_response(server, &address);
      if (res < 0) {
        break;
      }
    }
  }

  close(server);
}

static int make_socket(uint8_t ip_address[4], uint16_t port) {
  constexpr size_t IpAddrMaxLength = 12 + 3;  // 12 digits and 3 dots
  static char addr_buffer[IpAddrMaxLength + 1] = "";
  int res = 0;

  snprintf(addr_buffer, IpAddrMaxLength,
      "%hhd.%hhd.%hhd.%hhd",
      ip_address[0], ip_address[1], ip_address[2], ip_address[3]
  );

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = port;
  res = inet_aton(addr_buffer, &address.sin_addr);
  assert(res == 1);

  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(fd >= 0);
  res = bind(fd, (const struct sockaddr*) &address, sizeof(address));
  assert(res == 0);

  return fd;
}

static void send_stop(int socket, const struct sockaddr_in* addr) {
  uint32_t zero = 0;
  sendto(
      socket, &zero, sizeof(zero), 0, 
      (const struct sockaddr*) addr, sizeof(*addr)
  );
}

static int send_ping(int socket, const struct sockaddr_in* addr) {
  uint32_t length = htonl(4);
  int res = sendto(
      socket, &length, sizeof(length), 0,
      (const struct sockaddr*) addr, sizeof(*addr)
  );
  if (res < 0) return res;
  res = sendto(
      socket, "ping", 4, 0,
      (const struct sockaddr*) addr, sizeof(*addr)
  );
  if (res < 0) return res;
  return 0;
}

static int send_scream(int socket, const struct sockaddr_in* addr) {
  uint32_t length = htonl((uint32_t) UdpScreamLength);
  int res = sendto(
      socket, &length, sizeof(length), 0,
      (const struct sockaddr*) addr, sizeof(*addr)
  );
  if (res < 0) return res;

  res = sendto(
      socket, UdpScream, UdpScreamLength, 0,
      (const struct sockaddr*) addr, sizeof(*addr)
  );
  if (res < 0) return res;

  return 0;
}

static int receive_response(int socket, const struct sockaddr_in* addr) {
  uint32_t length = 0;
  char buffer[4];

  bool received = false;
  int res = 0;

  do {
    struct sockaddr_in source_addr;
    socklen_t source_size = sizeof(source_addr);

    res = recvfrom(socket, &length, sizeof(length), 0,
        (struct sockaddr*) &source_addr, &source_size);
    if (res < 0) return res;

    if (
      memcmp(
        &source_addr.sin_addr, &addr->sin_addr, sizeof(struct in_addr)
      ) != 0
    ) {
      continue;
    }

    if (source_addr.sin_port != addr->sin_port) {
      continue;
    }

    received = true;
  } while (!received);

  assert(res == sizeof(length));
  assert(ntohl(length) == 4);

  received = false;
  do {
    struct sockaddr_in source_addr;
    socklen_t source_size = sizeof(source_addr);

    res = recvfrom(socket, &buffer, 4, 0,
        (struct sockaddr*) &source_addr, &source_size);
    if (res < 0) return res;

    if (
      memcmp(
        &source_addr.sin_addr, &addr->sin_addr, sizeof(struct in_addr)
      ) != 0
    ) {
      continue;
    }

    if (source_addr.sin_port != addr->sin_port) {
      continue;
    }
    received = true;
  } while (!received);

  if (res < 0) return res;
  assert(res == 4);

  printf("%.*s\n", 4, buffer);
  return 0;
}
