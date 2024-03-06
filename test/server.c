#include <pnet/pnet.h>

#include <stdio.h>

pprot_e sProtocol = PNET_TCP;

int main(void) {
  if (!pnet_init(sProtocol)) {
    fprintf(stderr, "Failed to initialize!\n");
    return 1;
  }

  pserver_t *server = pserver_create();
  perror_e err;
  if ((err = pserver_bind(server, 641)) != PNET_SUCCESS) {
    fprintf(stderr, "Failed to bind! Error: %d\n", err);
    return 1;
  }

  if (sProtocol == PNET_TCP) {
    pserver_listen(server);
    while (1);
  } else {
    pclient_t *client;
    pmsg_t *msg;
    perror_e err;
    bool running = true;
    while (((err = pserver_recieve(server, &msg, &client)), true)) {
      if (err != PNET_SUCCESS && err != PNET_DISCONNECT) { running = false; break; }
      printf("[%s:%d]: %s\n", client->ip, client->port, msg->buffer);
    }
    printf("Failed with error: %d\n", err);
  }

  pserver_free(server);

  pnet_close();

  return 0;
}