#include <pnet/net.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#define sleep Sleep
#else
#include <unistd.h>
#endif

int main(void) {
  pnet_init();
  pclient_t *client = pclient_create();
  pclient_connect(client, "127.0.0.1", 641);

  pmsg_t *msg = pmsg_create("ping");
  while (1) {
    pclient_send(client, msg);
    sleep(1000);
  }
  pmsg_free(msg);

  pnet_close();

  return 0;
}