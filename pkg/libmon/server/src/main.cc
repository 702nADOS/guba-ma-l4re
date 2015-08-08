/*
  Matrix Multiplication program to generate some kind of CPU processing
  to monitor data from tasks in the L4re uKOS.
*/

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/object_registry>
#include <l4/cxx/ipc_server>
//#include "shared.h"

static L4Re::Util::Registry_server<> server;

class Mon_server: public L4::Server_object
{
  public:
      int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios);
};

int Mon_server::dispatch(l4_umword_t, L4::Ipc::Iostream &ios){
  l4_msgtag_t t;
  //ios >> t;
  ios << 77;
  return L4_EOK;
}


int
main(void) {
  static Mon_server mon;
  // Register calculation server
  if (!server.registry()->register_obj(&mon, "mon_channel").is_valid())
    {
      printf("Could not register my service, is there a 'mon_channel' in the caps table?\n");
      return 1;
    }
  printf("Welcome to the monitoring server!\n");
  // Wait for client requests
  server.loop();
  return 0;

  /*
  int i=0;
  for (;;)
    {
      i++;
      puts("Hello World!");
      sleep(i);
    }
  */
}
