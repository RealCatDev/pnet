#include "pnet/net.h"

#include <stdio.h>
#include <stdlib.h>

perror_e getError(int err) {
  #ifdef _WIN32
    switch (err) {
    case WSAEADDRINUSE: return PNET_ADDRINUSE;
    case WSAENOTSOCK:   return PNET_NOSOCKET;
    case WSAECONNRESET: return PNET_DISCONNECT;
    case 0:             return PNET_SUCCESS;
    }
  #else
  #endif
}

bool pnet_init() {
  #ifdef _WIN32
    return WSAStartup(MAKEWORD(2, 2), &sWsaData) == 0;
  #else
  #endif
}

void pnet_close() {
  #ifdef _WIN32
    WSACleanup();
  #else
  #endif
}

pmsg_t *pmsg_create(const char *msg) {
  pmsg_t *this = (pmsg_t *)malloc(sizeof(pmsg_t));
  strcpy(this->buffer, msg);
  this->bufferSize = strlen(msg);

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

perror_e pclient_connect(pclient_t *this, const char *ip, int port) {
  #ifdef _WIN32
    this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (this->sock == INVALID_SOCKET) {
      return getError(WSAGetLastError());
    }

    struct sockaddr_in serv_addr;
    ZeroMemory(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) != 1) {
      return getError(WSAGetLastError());
    }

    if (connect(this->sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
      return getError(WSAGetLastError());
    }
  #else
  #endif

  return PNET_SUCCESS;
}

perror_e pclient_send(pclient_t *this, pmsg_t *msg) {
  #ifdef _WIN32
    {
      size_t len = msg->bufferSize;
      if (send(this->sock, (char*)&len, sizeof(size_t), 0) <= 0) {
        return getError(WSAGetLastError());
      }
    }

    {
      if (send(this->sock, msg->buffer, msg->bufferSize, 0) <= 0) {
        return getError(WSAGetLastError());
      }
    }
  #else
  #endif

  return true;
}

perror_e pclient_recieve(pclient_t *this, pmsg_t **msg) {
  #ifdef _WIN32
    *msg = (pmsg_t*)malloc(sizeof(pmsg_t));
    int iRes = 0;

    size_t *len = &(*msg)->bufferSize;
    {
      if (recv(this->sock, (char*)len, sizeof(size_t), 0) <= 0) {
        free(*msg);
        *msg = NULL;
        return getError(WSAGetLastError());
      }
    }

    (*msg)->buffer = (char*)malloc(sizeof(char) * ((*len) + 1));
    {
      if (((*msg)->bufferSize = recv(this->sock, (*msg)->buffer, *len, 0)) <= 0) {
        free(*msg);
        *msg = NULL;
        return getError(WSAGetLastError());
      }
      (*msg)->buffer[(*msg)->bufferSize] = '\0';
    }
  #else
  #endif

  return PNET_SUCCESS;
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

perror_e pserver_bind(pserver_t *this, int port) {
  #ifdef _WIN32
    this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (this->sock == INVALID_SOCKET) {
      fprintf(stderr, "`socket` failed with error: %ld\n", WSAGetLastError());
      return false;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // change?
    addr.sin_port = htons(port);

    if (bind(this->sock, (struct sockaddr*)&addr, sizeof(addr)) != 0) return getError(WSAGetLastError());
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
    while ((sock = accept(this->sock, NULL, NULL)) != INVALID_SOCKET) {
      if (sock == INVALID_SOCKET) continue;
      unsigned threadID;

      pclient_t *client = pclient_create();
      client->sock = sock;
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
      return getError(WSAGetLastError());
    }

    unsigned threadID;
    this->listenThread = (HANDLE)_beginthreadex(NULL, 0, &ListenSession, (void*)this, 0, &threadID);
  #else
  #endif
  return PNET_SUCCESS;
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