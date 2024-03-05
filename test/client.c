#include <pnet/net.h>
#include <stdio.h>

#include <windows.h>

int main(int argc, char **argv) {
  if (!pnet_init(PNET_UDP)) {
    fprintf(stderr, "Failed to initialize!\n");
    return 1;
  }

  pclient_t *client = pclient_create();
  perror_e err;
  if ((err = pclient_connect(client, "127.0.0.1", 641)) != PNET_SUCCESS) {
    fprintf(stderr, "Failed to connect! Error: %d\n", err);
    return 1;
  }

  pmsg_t *msg = pmsg_create("ping");
  while (1) {
    printf("Sending: `%s`\n", msg->buffer);
    if ((err = pclient_send(client, msg)) != PNET_SUCCESS) {
      fprintf(stderr, "Failed to send! Error: %d\n", (int)err);
      goto cleanup;
    }
    Sleep(1000);
  }

cleanup:
  pmsg_free(msg);

  pnet_close();

  return 0;
}