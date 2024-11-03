#include "TcpClient.hpp"
#include "Command.hpp"
#include "Scream.hpp"

#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cassert>

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

static int make_socket(uint8_t ip_address[4], uint16_t port);

static void send_stop(int socket);
static int send_ping(int socket);
static int send_scream(int socket, char* scream, size_t scream_length);
static int receive_response(int socket);

void connect_tcp(uint8_t ip_address[4], uint16_t port) {
  int server = make_socket(ip_address, port);
  char* scream = new char[TcpScreamLength];
  std::fill(scream, scream + TcpScreamLength, 'A');

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
      send_stop(server);
      stopped = true;
      break;
    case Command::Ping:
      res = send_ping(server);
      has_response = true;
      break;
    case Command::Scream:
      res = send_scream(server, scream, TcpScreamLength);
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
      res = receive_response(server);
      if (res < 0) {
        break;
      }
    }
  }

  close(server);
  delete[] scream;
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

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(fd >= 0);
  res = connect(fd, (const struct sockaddr*) &address, sizeof(address));
  assert(res == 0);

  return fd;
}

static void send_stop(int socket) {
  uint32_t zero = 0;
  write(socket, &zero, sizeof(zero));
}

static int send_ping(int socket) {
  uint32_t length = htonl(4);
  int res = write(socket, &length, sizeof(length));
  if (res < 0) return res;
  res = write(socket, "ping", 4);
  if (res < 0) return res;
  return 0;
}

static int send_scream(int socket, char* scream, size_t scream_length) {
  uint32_t length = htonl((uint32_t) scream_length);
  int res = write(socket, &length, sizeof(length));
  if (res < 0) return res;

  size_t remaining = scream_length;
  while (remaining > 0) {
    res = write(socket, scream, remaining);
    if (res < 0) return res;
    scream += res;
    remaining -= res;
  }

  return 0;
}

static int receive_response(int socket) {
  uint32_t length = 0;
  char buffer[4];

  int res = recv(socket, &length, sizeof(length), MSG_WAITALL);
  if (res < 0) return res;
  assert(res == sizeof(length));
  assert(ntohl(length) == 4);

  res = recv(socket, &buffer, 4, MSG_WAITALL);
  if (res < 0) return res;
  assert(res == 4);

  printf("%.*s\n", 4, buffer);
  return 0;
}
