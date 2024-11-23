// SPDX-License-Identifier: GPL-2.0-or-later

#include "config.h"

#include "futils.hpp"
#include "http.hpp"

#include <array>
#include <cerrno>
#include <chrono>
#include <cinttypes>
#include <thread>

////////////////////////////////////////
// platform specific code

#ifndef _WIN32
////////////////////////////////////////
// Unix or Mac

#define closesocket close

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define SOCKET_ERROR (-1)
#define WSAEWOULDBLOCK EINPROGRESS
#define WSAENOTCONN EAGAIN

static int WSAGetLastError() {
  return errno;
}
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

////////////////////////////////////////
// code
static constexpr auto httpTemplate =
    "%s %s HTTP/%s\r\n"  // $verb $page $version
    "User-Agent: exiv2http/1.0.0\r\n"
    "Accept: */*\r\n"
    "Host: %s\r\n"  // $servername
    "%s"            // $header
    "\r\n";

#define white(c) (((c) == ' ') || ((c) == '\t'))

#define FINISH (-999)
#define OK(s) (200 <= (s) && (s) < 300)

static constexpr std::array<const char*, 2> blankLines{
    "\r\n\r\n",  // this is the standard
    "\n\n",      // this is commonly sent by CGI scripts
};

static constexpr auto snooze = std::chrono::milliseconds::zero();
static auto sleep_ = std::chrono::milliseconds(1000);

static int forgive(int n, int& err) {
  err = WSAGetLastError();
  if (!n && !err)
    return FINISH;
#ifndef _WIN32
  if (n == 0)
    return FINISH;  // server hungup
#endif
  if (n == SOCKET_ERROR && (err == WSAEWOULDBLOCK || err == WSAENOTCONN))
    return 0;
  return n;
}

static int error(std::string& errors, const char* msg, const char* x = nullptr, const char* y = nullptr, int z = 0) {
  static const size_t buffer_size = 512;
  char buffer[buffer_size] = {};
  snprintf(buffer, buffer_size, msg, x, y, z);
  if (errno) {
    perror(buffer);
  } else {
    fprintf(stderr, "%s\n", buffer);
  }
  errors += std::string(msg) + '\n';
  return -1;
}

static void flushBuffer(const char* buffer, size_t start, int& end, std::string& file) {
  file += std::string(buffer + start, end - start);
  end = 0;
}

static Exiv2::Dictionary stringToDict(const std::string& s) {
  Exiv2::Dictionary result;
  std::string token;

  size_t i = 0;
  while (i < s.length()) {
    if (s[i] != ',') {
      if (s[i] != ' ')
        token += s[i];
    } else {
      result[token] = token;
      token.clear();
    }
    i++;
  }
  result[token] = token;
  return result;
}

static int makeNonBlocking(int sockfd) {
#if defined(_WIN32)
  ULONG ioctl_opt = 1;
  return ioctlsocket(sockfd, FIONBIO, &ioctl_opt);
#else
  int result = fcntl(sockfd, F_SETFL, O_NONBLOCK);
  return result >= 0 ? result : SOCKET_ERROR;
#endif
}

int Exiv2::http(Exiv2::Dictionary& request, Exiv2::Dictionary& response, std::string& errors) {
  request.try_emplace("verb", "GET");
  request.try_emplace("header");
  request.try_emplace("version", "1.0");
  request.try_emplace("port");

  std::string file;
  errors = "";
  int result = 0;

  ////////////////////////////////////
  // Windows specific code
#if defined(_WIN32)
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    return error(errors, "could not start WinSock");
#endif

  const char* servername = request["server"].c_str();
  const char* page = request["page"].c_str();
  const char* verb = request["verb"].c_str();
  const char* header = request["header"].c_str();
  const char* version = request["version"].c_str();
  const char* port = request["port"].c_str();

  const char* servername_p = servername;
  const char* port_p = port;
  std::string url = std::string("http://") + request["server"] + request["page"];

  // parse and change server if using a proxy
  const char* PROXI = "HTTP_PROXY";
  const char* proxi = "http_proxy";
  const char* PROXY = getenv(PROXI);
  const char* proxy = getenv(proxi);
  bool bProx = PROXY || proxy;
  const char* prox = bProx ? (proxy ? proxy : PROXY) : "";
  Exiv2::Uri Proxy = Exiv2::Uri::Parse(prox);

  // find the dictionary of no_proxy servers
  const char* NO_PROXI = "NO_PROXY";
  const char* no_proxi = "no_proxy";
  const char* NO_PROXY = getenv(NO_PROXI);
  const char* no_proxy = getenv(no_proxi);
  bool bNoProxy = NO_PROXY || no_proxy;
  auto no_prox = std::string(bNoProxy ? (no_proxy ? no_proxy : NO_PROXY) : "");
  Exiv2::Dictionary noProxy = stringToDict(no_prox + ",localhost,127.0.0.1");

  // if the server is on the no_proxy list ... ignore the proxy!
  if (noProxy.count(servername))
    bProx = false;

  if (bProx) {
    servername_p = Proxy.Host.c_str();
    port_p = Proxy.Port.c_str();
    page = url.c_str();
  }
  if (!port[0])
    port = "80";
  if (!port_p[0])
    port_p = "80";

  ////////////////////////////////////
  // open the socket
  auto sockfd = static_cast<int>(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
  if (sockfd < 0)
    return error(errors, "unable to create socket\n", nullptr, nullptr, 0);

  // connect the socket to the server
  int server = -1;

  // fill in the address
  sockaddr_in serv_addr = {};
  int serv_len = sizeof(serv_addr);

  // convert unknown servername into IP address
  // http://publib.boulder.ibm.com/infocenter/iseries/v5r3/index.jsp?topic=/rzab6/rzab6uafinet.htm
  if (inet_pton(AF_INET, servername_p, &serv_addr.sin_addr) != 0) {
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo* result;

    int res = getaddrinfo(servername_p, port_p, &hints, &result);
    if (res != 0) {
      closesocket(sockfd);
      return error(errors, "no such host: %s", gai_strerror(res));
    }

    std::memcpy(&serv_addr, result->ai_addr, serv_len);

    freeaddrinfo(result);
  }

  makeNonBlocking(sockfd);

  ////////////////////////////////////
  // and connect
  server = connect(sockfd, reinterpret_cast<const sockaddr*>(&serv_addr), serv_len);
  if (server == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
    closesocket(sockfd);
    return error(errors, "error - unable to connect to server = %s port = %s wsa_error = %d", servername_p,
                 std::to_string(serv_addr.sin_port).c_str(), WSAGetLastError());
  }

  char buffer[(32 * 1024) + 1];
  size_t buff_l = sizeof buffer - 1;

  ////////////////////////////////////
  // format the request
  int n = snprintf(buffer, buff_l, httpTemplate, verb, page, version, servername, header);
  buffer[n] = 0;
  response["requestheaders"] = std::string(buffer, n);

  ////////////////////////////////////
  // send the header (we'll have to wait for the connection by the non-blocking socket)
  while (sleep_ >= std::chrono::milliseconds::zero() &&
         send(sockfd, buffer, n, 0) == SOCKET_ERROR /* && WSAGetLastError() == WSAENOTCONN */) {
    std::this_thread::sleep_for(snooze);
    sleep_ -= snooze;
  }

  if (sleep_ < std::chrono::milliseconds::zero()) {
    closesocket(sockfd);
    return error(errors, "error - timeout connecting to server = %s port = %s wsa_error = %d", servername, port,
                 WSAGetLastError());
  }

  int end = 0;             // write position in buffer
  bool bSearching = true;  // looking for headers in the response
  int status = 200;        // assume happiness

  ////////////////////////////////////
  // read and process the response
  int err = 0;
  n = forgive(recv(sockfd, buffer, static_cast<int>(buff_l), 0), err);
  while (n >= 0 && OK(status)) {
    if (n) {
      end += n;
      buffer[end] = 0;

      size_t body = 0;  // start of body
      if (bSearching) {
        // search for the body
        for (auto&& line : blankLines) {
          if (!bSearching)
            break;
          const char* blankLinePos = strstr(buffer, line);
          if (blankLinePos) {
            bSearching = false;
            body = blankLinePos - buffer + strlen(line);
            const char* firstSpace = strchr(buffer, ' ');
            if (firstSpace) {
              status = atoi(firstSpace);
            }
          }
        }

        // parse response headers
        char* h = buffer;
        char C = ':';
        char N = '\n';
        int i = 0;  // initial byte in buffer
        while (buffer[i] == N)
          i++;
        h = strchr(h + i, N);
        if (!h) {
          status = 0;
          break;
        }
        h++;
        response[""] = std::string(buffer + i).substr(0, h - buffer - 2);
        const char* firstSpace = strchr(buffer, ' ');
        if (!firstSpace) {
          status = 0;
          break;
        }
        result = atoi(firstSpace);
        auto c = strchr(h, C);
        char* first_newline = strchr(h, N);
        while (c && first_newline && c < first_newline && h < buffer + body) {
          std::string key(h);
          std::string value(c + 1);
          key.resize(c - h);
          value.resize(first_newline - c - 1);
          response[key] = value;
          h = first_newline + 1;
          c = strchr(h, C);
          first_newline = strchr(h, N);
        }
      }

      // if the buffer's full and we're still searching - give up!
      // this handles the possibility that there are no headers
      if (bSearching && buff_l - end < 10) {
        bSearching = false;
        body = 0;
      }
      if (!bSearching && OK(status)) {
        flushBuffer(buffer, body, end, file);
      }
    }
    n = forgive(recv(sockfd, buffer + end, static_cast<int>(buff_l - end), 0), err);
    if (!n) {
      std::this_thread::sleep_for(snooze);
      sleep_ -= snooze;
      if (sleep_ < std::chrono::milliseconds::zero())
        n = FINISH;
    }
  }

  if (n != FINISH || !OK(status)) {
    snprintf(buffer, sizeof buffer, "wsa_error = %d,n = %d,sleep_ = %d status = %d", WSAGetLastError(), n,
             static_cast<int>(sleep_.count()), status);
    error(errors, buffer, nullptr, nullptr, 0);
  } else if (bSearching && OK(status)) {
    if (end) {
      //  we finished OK without finding headers, flush the buffer
      flushBuffer(buffer, 0, end, file);
    } else {
      closesocket(sockfd);
      return error(errors, "error - no response from server = %s port = %s wsa_error = %d", servername, port,
                   WSAGetLastError());
    }
  }

  ////////////////////////////////////
  // close sockets
  closesocket(server);
  closesocket(sockfd);
  response["body"] = file;
  return result;
}

// That's all Folks
