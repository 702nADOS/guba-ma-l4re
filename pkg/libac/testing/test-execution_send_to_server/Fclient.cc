/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/sys/err.h>
#include <l4/sys/types.h>
#include <l4/re/env>
#include <l4/sys/scheduler>
#include <l4/re/util/cap_alloc>
#include <l4/cxx/ipc_stream>

#include <l4/sys/ipc.h>
#include <l4/sys/thread.h>
#include <l4/sys/factory.h>
#include <l4/sys/utcb.h>
#include <l4/re/env.h>
#include <l4/re/c/util/cap_alloc.h>

extern "C" {
#include <l4/libedft/edft.h>
}
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <l4/sys/l4int.h>
#include <l4/util/rdtsc.h>


#include <stdio.h>

#include <l4/util/util.h>

#include "shared.h"

#include "global.h"


#define edftn (0)
#define bedZ (0)


long unsigned times[FThreads][bedZ];

void thread_func(l4_umword_t);


struct thread
{
  unsigned startzeit;
  unsigned scheduler;//0 FP, 1 EDF
  unsigned bedienzeit;
  l4_sched_param_t sp;
  Edf_thread t;
};




struct thread t[FThreads];

L4::Cap<void> server;



void thread_func(l4_umword_t no)
{
  l4_umword_t i;

  L4::Ipc::Iostream* s = new  L4::Ipc::Iostream(l4_utcb());
  int r;
  for (i = 0; i < FbedZ; i++)
  {
    s = new  L4::Ipc::Iostream(l4_utcb());
    (*s) << 1UL;
    (*s) << no;
    
    r = l4_error(s->call(server.cap(), 0));
    if (r)
    {
      printf("Error talking to server\n");
      return;
    }
    delete s;
  }


  edft_exit_thread();
}

int
main()
{
  l4_sleep(100);
  server = L4Re::Env::env()->get_cap<void>("Fcap");
  if (!server.is_valid())
  {
    printf("Could not get server capability!\n");
    return 1;
  }

  unsigned i;

  l4_cap_idx_t task_cap = 0UL;

  unsigned fpn = FThreads;

  unsigned baseDl = 10;
  for (i = 0; i < fpn + edftn; i++)
  {
    t[i].t.no = i;
    t[i].bedienzeit = 3;
    printf("Create Thread %i\n", i);

    sprintf(t[i].t.name, "thread%i", i);
    t[i].t.wcet = 5;
    if (i < fpn)
      t[i].t.dl = -1;
    else
      t[i].t.dl = baseDl + 5 * (i - fpn);
    t[i].t.func = (void*)thread_func;
    t[i].t.arg = i;
    t[i].t.suc = NULL;
    t[i].t.thread_cap = 0;
    t[i].t.task_cap = task_cap;
    //printf("Finished Create Thread %i\n", i);
    edft_create_l4_thread(&(t[i].t), 0);
    if (i < fpn) {
      //printf("TASK 1, THREAD %d, PRIO:%d\n", i,  i * 75 + 75);
      t[i].sp = l4_sched_param_by_type(Fixed_prio, i * 75 + 40, 0);
    }
    else
      t[i].sp = l4_sched_param_by_type(Deadline, t[i].t.dl, 0);

  }

  printf("CREATE THREADS\n");

  for (i = 0; i < fpn + edftn; i++)
  {
    if (l4_error(l4_scheduler_run_thread(l4re_env()->scheduler, t[i].t.thread_cap, &t[i].sp)))
      return -1;
  }
  printf("THREADS CREATED\n");


  l4_sleep(1000000);



  return 0;
}
