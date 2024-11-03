#include "UdpServer.hpp"
#include "Scream.hpp"

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

static int make_socket(uint8_t ip_address[4], uint16_t port);

static void serve_client(int socket, const struct sockaddr_in* addr);

static void interrupt_handler(int) { /* Enter handler but do nothing */ }
static void setup_interrupt_handler() {
  int res = 0;
  struct sigaction action;
  action.sa_handler = &interrupt_handler;
  action.sa_flags = 0;
  res = sigemptyset(&action.sa_mask);
  assert(res == 0);

  res = sigaction(SIGINT, &action, NULL);
  assert(res == 0);
}

void listen_udp(uint8_t ip_address[4], uint16_t port) {
  int socket = make_socket(ip_address, port);

  setup_interrupt_handler();

  struct sockaddr_in addr;
  socklen_t addr_size = sizeof(addr);
  for (;;) {
    errno = 0;
    uint32_t length = 0;
    int res = recvfrom(socket, &length, sizeof(length), MSG_PEEK, (struct sockaddr*) &addr, &addr_size);
    if (res == -1 && errno == EINTR) {
      break;
    }
    if (res < 0) {
      perror(strerror(errno));
    }
    assert(res >= 0);

    puts("Connected!");
    serve_client(socket, &addr);
    puts("Disconnected!");
    if (errno == EINTR) {
      break;
    }
  }
  close(socket);

  puts("");
  puts("Server stopped");
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

static void serve_client(
    int socket,
    const struct sockaddr_in *addr
) {
  char buffer[UdpScreamLength] = "";
  uint32_t message_size = 0;
  bool conn_closed = false;


  while (!conn_closed) {
    struct sockaddr_in source_addr;
    socklen_t source_size = sizeof(source_addr);
    int res = recvfrom(socket, &message_size, sizeof(message_size), 0,
        (struct sockaddr*) &source_addr, &source_size);
    
    if (res <= 0) {
      break;
    }

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

    assert(res == sizeof(message_size));

    if (message_size == 0) {
      break;
    }
    message_size = ntohl(message_size);

    size_t offset = 0;
    size_t read_length = message_size;
    if (message_size > UdpScreamLength) {
      break;
    }

    while (read_length > 0) {
      res = recv(socket, buffer + offset, read_length, 0);

      if (res <= 0) {
        break;
      }

      offset += (size_t) res;
      read_length -= (size_t) res;
    }

    if (read_length > 0) {
      break;
    }

    printf("%u: %.*s\n", message_size, (int) message_size, buffer);
    if (message_size <= 4) {
      uint32_t length = htonl(4);
      res = sendto(socket, &length, sizeof(length), 0,
          (const struct sockaddr*) addr, sizeof(*addr));
      assert(res == sizeof(length));
      res = sendto(socket, "pong", 4, 0,
          (const struct sockaddr*) addr, sizeof(*addr));
      assert(res == 4);
    }
    else {
      uint32_t length = htonl(4);
      res = sendto(socket, &length, sizeof(length), 0,
          (const struct sockaddr*) addr, sizeof(*addr));
      assert(res == sizeof(length));
      res = sendto(socket, "tldr", 4, 0,
          (const struct sockaddr*) addr, sizeof(*addr));
      assert(res == 4);
    }
  }
}
