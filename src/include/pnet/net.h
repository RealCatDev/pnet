#ifndef PNET_NET_H_
#define PNET_NET_H_

#include <stdint.h>
#include <stdbool.h>

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
  PNET_ADDRINUSE  = -3,
  PNET_NOSOCKET   = -2,
  PNET_DISCONNECT = -1,
  PNET_SUCCESS    = 0,
} perror_e;

bool pnet_init();
void pnet_close();

typedef struct message {
  char *buffer;
  size_t bufferSize;
} pmsg_t;

pmsg_t *pmsg_create(const char *msg);
void    pmsg_free  (pmsg_t *this);

typedef struct client {
  #ifdef _WIN32
    SOCKET sock;
    HANDLE thread;
  #else

  #endif
} pclient_t;

pclient_t *pclient_create ();
perror_e   pclient_connect(pclient_t *this, const char *ip, int port);
perror_e   pclient_send   (pclient_t *this, pmsg_t *msg);
perror_e   pclient_recieve(pclient_t *this, pmsg_t **msg);
void       pclient_free   (pclient_t *this);

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
void       pserver_setClientFunction(pserver_t *this, void(*func)(pclient_t *clnt));
perror_e   pserver_bind             (pserver_t *this, int port);
perror_e   pserver_listen           (pserver_t *this);
void       pserver_free             (pserver_t *this);

#endif