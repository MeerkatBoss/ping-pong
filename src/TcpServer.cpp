#include "TcpServer.hpp"
#include "Scream.hpp"

#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cassert>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

static constexpr size_t BacklogSize = 16;
static int make_listen_socket(uint8_t ip_address[4], uint16_t port);

static void serve_client(int socket);

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

void listen_tcp(uint8_t ip_address[4], uint16_t port) {
  int listener = make_listen_socket(ip_address, port);

  setup_interrupt_handler();

  for (;;) {
    errno = 0;
    int client = accept(listener, NULL, NULL);
    if (client == -1 && errno == EINTR) {
      break;
    }
    assert(client >= 0);

    serve_client(client);
    if (errno == EINTR) {
      break;
    }
  }

  puts("");
  puts("Server stopped");
}

static int make_listen_socket(uint8_t ip_address[4], uint16_t port) {
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
  res = bind(fd, (const struct sockaddr*) &address, sizeof(address));
  assert(res == 0);
  res = listen(fd, BacklogSize);
  assert(res == 0);

  return fd;
}

static void serve_client(int socket) {
  constexpr size_t buffer_size = TcpScreamLength;
  char* buffer = new char[buffer_size];
  uint32_t message_size = 0;

  bool conn_closed = false;

  while (!conn_closed) {
    int res = recv(socket, &message_size, sizeof(message_size), MSG_WAITALL);
    
    if (res <= 0) {
      break;
    }

    assert(res == sizeof(message_size));

    size_t offset = 0;
    size_t read_length = message_size;
    if (message_size > buffer_size) {
      break;
    }

    while (read_length > 0) {
      res = recv(socket, buffer + offset, read_length, MSG_WAITALL);

      if (res <= 0) {
        break;
      }

      offset += (size_t) res;
      read_length -= (size_t) res;
    }

    if (read_length > 0) {
      break;
    }

    printf("%*s\n", (int) message_size, buffer);
    if (message_size <= 4) {
      uint32_t length = htonl(4);
      res = write(socket, &length, 4);
      assert(res == 4);
      res = write(socket, "pong", 4);
      assert(res == 4);
    }
    else {
      uint32_t length = htonl(4);
      res = write(socket, &length, 4);
      assert(res == 4);
      res = write(socket, "tldr", 4);
      assert(res == 4);
    }
  }

  close(socket);
  delete[] buffer;
}
