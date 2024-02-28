#include <pnet/net.h>

#include <stdio.h>

int main(void) {
  pnet_init();

  pserver_t *server = pserver_create();
  pserver_bind(server, 641);
  pserver_listen(server);

  while (1);

  pserver_free(server);

  pnet_close();

  return 0;
}