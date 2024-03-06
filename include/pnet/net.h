#ifndef PNET_NET_H_
#define PNET_NET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum {
  PNET_TCP,
  PNET_UDP
} pprot_e;

#ifdef _WIN32
  #undef UNICODE
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
  #include <WinSock2.h>
  #include <WS2tcpip.h>
  #include <tchar.h>
  #include <process.h>
  #pragma comment (lib, "Ws2_32.lib")
  static WSADATA sWsaData;
#else

#endif

typedef enum {
  #ifdef _WIN32
    PNET_ADDRINUSE  = WSAEADDRINUSE,
    PNET_NOTSOCKET  = WSAENOTSOCK,
    PNET_DISCONNECT = WSAEDISCON,
    PNET_CONNRESET  = WSAECONNRESET,
  #else

  #endif
  PNET_SUCCESS    = 0,
} perror_e;

bool pnet_init(pprot_e protocol);
void pnet_close();

typedef struct message {
  char *buffer;
  size_t bufferSize;
} pmsg_t;

pmsg_t *pmsg_create(const char *msg);
void    pmsg_free  (pmsg_t *);

typedef struct client {
  #ifdef _WIN32
    SOCKET sock;
    HANDLE thread;
    struct sockaddr_in addr;
    int addrLen;
    char ip[INET_ADDRSTRLEN];
  #else

  #endif
  uint16_t port;
} pclient_t;

pclient_t *pclient_create ();
perror_e   pclient_connect(pclient_t *, const char *ip, uint16_t port);
perror_e   pclient_send   (pclient_t *, pmsg_t *msg);
perror_e   pclient_recieve(pclient_t *, pmsg_t **msg);
void       pclient_free   (pclient_t *);

typedef struct server {
  #ifdef _WIN32
    SOCKET sock;
    HANDLE listenThread;
  #else

  #endif
  pclient_t **clients;
  size_t clientsCapacity, clientsSize;
  void(*clientFunction)(pclient_t *clnt);
} pserver_t;

pserver_t *pserver_create           ();
void       pserver_setClientFunction(pserver_t *, void(*func)(pclient_t *clnt));
perror_e   pserver_bind             (pserver_t *, uint16_t port);
perror_e   pserver_listen           (pserver_t *);
perror_e   pserver_recieve          (pserver_t *, pmsg_t **msg, pclient_t **client);
void       pserver_free             (pserver_t *);

#ifdef __cplusplus
}
#endif

#endif