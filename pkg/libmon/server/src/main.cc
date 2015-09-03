/*
 * Send monitoring data via IPC
 */

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/object_registry>
#include <l4/cxx/ipc_server>
#include <l4/re/env>
#include <l4/sys/scheduler>
#include <l4/sys/scheduler.h>

#include <l4/libmon/monitor.h>
//#include "shared.h"

static L4Re::Util::Registry_server<> server;

class Mon_server: public L4::Server_object
{
  public:
      int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios);
};

int Mon_server::dispatch(l4_umword_t, L4::Ipc::Iostream &ios){
  //l4_msgtag_t t;
  //l4_msgtag_t tag;
  l4_umword_t cpu_max=0;
  l4_sched_cpu_set_t cs = l4_sched_cpu_set(0, 0);

  monitor* m = new monitor();

  //tag = l4_scheduler_info(l4re_env()->scheduler, &cpu_max, &cs);
  if (l4_error(L4Re::Env::env()->scheduler()->info(&cpu_max, &cs)) < 0)
    printf("MOnitoring ERRRR !!\n");

  printf("monitoring %ld %ld %ld %c\n",cpu_max, cs.offset, cs.map, cs.granularity);

  l4_sched_cpu_set_t css = l4_sched_cpu_set(0, 0);
  if (l4_error(L4Re::Env::env()->scheduler()->idle_time(css)) < 0)
    printf("MOnitoring ERRRR !!\n");

  m->idle_time = l4_utcb_mr()->mr[0];

  //ios >> t;
  ios << m->idle_time;
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
