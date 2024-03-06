#include "pnet/net.h"

#include <stdio.h>
#include <stdlib.h>

static pprot_e sProtocol = PNET_TCP;

#ifdef _WIN32
  SOCKET psock_create() {
    return socket(AF_INET, sProtocol == PNET_TCP ? SOCK_STREAM : SOCK_DGRAM, sProtocol == PNET_TCP ? IPPROTO_TCP : IPPROTO_UDP);
  }
#else
#endif

bool pnet_init(pprot_e protocol) {
  sProtocol = protocol;
  if (sProtocol != protocol) return false; // idk :3
  #ifdef _WIN32
    return WSAStartup(MAKEWORD(2, 2), &sWsaData) == 0;
  #else
  #endif
  return true;
}

void pnet_close() {
  #ifdef _WIN32
    WSACleanup();
  #else
  #endif
}

pmsg_t *pmsg_create(const char *msg) {
  pmsg_t *this = (pmsg_t *)malloc(sizeof(pmsg_t));
  this->buffer = (char*)malloc(sizeof(char) * (this->bufferSize = strlen(msg)));
  strcpy(this->buffer, msg);

  return this;
}

void pmsg_free(pmsg_t *this) {
  free(this->buffer);

  free(this);
}

pclient_t *pclient_create () {
  pclient_t *this = (pclient_t *)malloc(sizeof(pclient_t));
  #ifdef _WIN32
    this->sock = INVALID_SOCKET;
  #else
  
  #endif
  return this;
}

perror_e pclient_connect(pclient_t *this, const char *ip, uint16_t port) {
  #ifdef _WIN32
    this->sock = psock_create();
    if (this->sock == INVALID_SOCKET) return WSAGetLastError();

    struct sockaddr_in serv_addr;
    this->addrLen = sizeof(serv_addr);
    
    ZeroMemory(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) != 1) return WSAGetLastError();

    this->addr = serv_addr;
    if (sProtocol == PNET_TCP) {
      if (connect(this->sock, (struct sockaddr*)&this->addr, this->addrLen) == SOCKET_ERROR) return WSAGetLastError();
    }
  #else
  #endif

  return PNET_SUCCESS;
}

perror_e psend(SOCKET sock, struct sockaddr *addr, int addrLen, char *buf, int len) {
  #ifdef _WIN32
    if (sProtocol == PNET_TCP) {
      if (send(sock, buf, len, 0) <= 0) return WSAGetLastError();
    } else {
      if (sendto(sock, buf, len, 0, addr, addrLen) <= 0) return WSAGetLastError();
    }
  #else
  #endif
  return PNET_SUCCESS;
}

perror_e pclient_send(pclient_t *this, pmsg_t *msg) {
  perror_e err = psend(this->sock, (struct sockaddr*)&this->addr, this->addrLen, (char*)&msg->bufferSize, sizeof(size_t));
  if (err != PNET_SUCCESS) return err;
  return psend(this->sock, (struct sockaddr*)&this->addr, this->addrLen, msg->buffer, (int)msg->bufferSize);
}

#ifdef _WIN32
perror_e recieve(SOCKET sock, struct sockaddr *addr, int *addrLen, pmsg_t **msg) {
  *msg = (pmsg_t*)malloc(sizeof(pmsg_t));
  int iRes = 0;

  size_t *len = &(*msg)->bufferSize;
  if (sProtocol == PNET_TCP) {
    if (recv(sock, (char*)len, sizeof(size_t), 0) <= 0) {
      free(*msg);
      *msg = NULL;
      return (perror_e) WSAGetLastError();
    }
  } else {
    if (recvfrom(sock, (char*)len, sizeof(size_t), 0, addr, addrLen) <= 0) {
      free(*msg);
      *msg = NULL;
      return (perror_e) WSAGetLastError();
    }
  }

  (*msg)->buffer = (char*)malloc(sizeof(char) * ((*len) + 1));
  if (sProtocol == PNET_TCP) {
    if (((*msg)->bufferSize = recv(sock, (*msg)->buffer, *len, 0)) <= 0) {
      free(*msg);
      *msg = NULL;
      return (perror_e) WSAGetLastError();
    }
  } else {
    if (((*msg)->bufferSize = recvfrom(sock, (*msg)->buffer, *len, 0, addr, addrLen)) <= 0) {
      free(*msg);
      *msg = NULL;
      return (perror_e) WSAGetLastError();
    }
  }
  (*msg)->buffer[(*msg)->bufferSize] = '\0';

  return PNET_SUCCESS;
}
#else
#endif

perror_e pclient_recieve(pclient_t *this, pmsg_t **msg) {
  #ifdef _WIN32
    return recieve(this->sock, (struct sockaddr*)&this->addr, &this->addrLen, msg);
  #else
  #endif
}

void pclient_free(pclient_t *this) {
  #ifdef _WIN32
    if (this->sock != INVALID_SOCKET) closesocket(this->sock);
  #else
  #endif

  free(this);
}

void DefaultClientFunc(pclient_t *this) {
  printf("Client connected!\n");
  pmsg_t *msg = NULL;
  while (pclient_recieve(this, &msg) == PNET_SUCCESS) {
    printf("Client: %s\n", msg->buffer);
  }
  printf("Client disconnected!\n");
}

pserver_t *pserver_create() {
  pserver_t *this = (pserver_t *)malloc(sizeof(pserver_t));
  #ifdef _WIN32
    this->sock = INVALID_SOCKET;
  #else

  #endif
  this->clients = (pclient_t **)malloc(sizeof(pclient_t*) * (this->clientsCapacity = 512));
  this->clientsSize = 0;
  this->clientFunction = &DefaultClientFunc;

  return this;
}

void pserver_setClientFunction(pserver_t *this, void(*func)(pclient_t *clnt)) {
  this->clientFunction = func;
}

perror_e pserver_bind(pserver_t *this, uint16_t port) {
  #ifdef _WIN32
    this->sock = psock_create();
    if (this->sock == INVALID_SOCKET) return WSAGetLastError();
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(this->sock, (struct sockaddr*)&addr, sizeof(addr)) != 0) return WSAGetLastError();
  #else

  #endif
  return PNET_SUCCESS;
}

void pserver_addClient(pserver_t *this, pclient_t *client) {
  if (this->clientsCapacity <= this->clientsSize)
    this->clients = (pclient_t**)realloc(this->clients, ++this->clientsCapacity);
  this->clients[this->clientsSize++] = client;
}

void pserver_removeClient(pserver_t *this, pclient_t *client) {
  size_t index = -1;
  for (size_t i = 0; i < this->clientsSize; ++i) 
    if (this->clients[i] == client) { free(this->clients[i]); index = i; break; }
  if (index < 0) return;
  for (size_t i = index; i < this->clientsSize-1; ++i)
    this->clients[i] = this->clients[i+1];
  if ((--this->clientsSize) > 0) free(this->clients[this->clientsSize]);
}

typedef struct callbackData {
  pserver_t *server;
  pclient_t *client;
} callbackData_t;

unsigned __stdcall ClientSession(void *data) {
  callbackData_t *cbData = (callbackData_t*)data;
  cbData->server->clientFunction(cbData->client);
  pserver_removeClient(cbData->server, cbData->client);

  return 0;
}

unsigned __stdcall ListenSession(void *data) {
  pserver_t *this = (pserver_t*)data;
  #ifdef _WIN32
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr addr;
    int addrLen;
    while ((sock = accept(this->sock, &addr, &addrLen)) != INVALID_SOCKET) {
      if (sock == INVALID_SOCKET) continue;
      unsigned threadID;

      pclient_t *client = pclient_create();
      client->sock = sock;

      struct sockaddr_in *sin = &addr;
      inet_ntop(AF_INET, &sin->sin_addr, client->ip, sizeof(client->ip));
      client->port = htons(sin->sin_port);
      free(sin);

      callbackData_t cbData = {0};
      cbData.client = client;
      cbData.server = this;
      client->thread = (HANDLE)_beginthreadex(NULL, 0, &ClientSession, (void*)&cbData, 0, &threadID);

      pserver_addClient(this, client);
    }
  #else
  #endif
  return 0;
}

perror_e pserver_listen(pserver_t *this) {
  #ifdef _WIN32
    if (listen(this->sock, SOMAXCONN) != 0) {
      return (perror_e) WSAGetLastError();
    }

    unsigned threadID;
    this->listenThread = (HANDLE)_beginthreadex(NULL, 0, &ListenSession, (void*)this, 0, &threadID);
  #else
  #endif
  return PNET_SUCCESS;
}

perror_e pserver_recieve(pserver_t *this, pmsg_t **msg, pclient_t **client) {
  perror_e err = PNET_SUCCESS;
  #ifdef _WIN32
    struct sockaddr_in addr;
    int addrLen = sizeof(addr);
    if ((err = recieve(this->sock, (struct sockaddr*)&addr, &addrLen, msg)) != PNET_SUCCESS) return err;

    char ip[INET_ADDRSTRLEN];
    uint16_t port;

    struct sockaddr_in *sin = &addr;
    inet_ntop(AF_INET, &sin->sin_addr, ip, sizeof(ip));
    port = htons(sin->sin_port);

    *client = NULL;
    for (size_t i = 0; i < this->clientsSize; ++i) {
      pclient_t *clnt = this->clients[i];
      if (strcmp(clnt->ip, ip) == 0 && clnt->port == port) {
        *client = clnt;
        break;
      }
    }
    if (!(*client)) {
      *client = pclient_create();
      (*client)->addr = addr;
      strcpy((*client)->ip, ip);
      (*client)->port = port;
      pserver_addClient(this, *client);
    }
  #else
  #endif

  return err;
}

void pserver_free(pserver_t *this) {
  #ifdef _WIN32
    for (size_t i = 0; i < this->clientsSize; ++i) pclient_free(this->clients[i]);
    free(this->clients);
    if (this->sock != INVALID_SOCKET) closesocket(this->sock);
    free(this);
  #else
  #endif
}