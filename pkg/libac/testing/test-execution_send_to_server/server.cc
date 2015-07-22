/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <stdio.h>
#include <l4/re/env>
#include <l4/sys/scheduler>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/object_registry>
#include <l4/cxx/ipc_server>

#include <l4/util/util.h>

#include "shared.h"
#include "global.h"
#include "list.h"
#include <stdlib.h>

const char*  TASKS[2] = {"MO", "FA"};


typedef struct _thread {
  l4_umword_t thread;
  l4_umword_t task;
  l4_umword_t count;

  _thread(l4_umword_t th, l4_umword_t ta, l4_umword_t c) : thread(th), task(ta), count (c)
  {

  }
} thread;


thread* threads[MThreads + FThreads];

static unsigned use_first = 1;

static unsigned print = 0;
static unsigned max = 1;
static unsigned plots = 0;

class Test_Server : public L4::Server_object
{
public:



  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios);
};




int
Test_Server::dispatch(l4_umword_t, L4::Ipc::Iostream &ios)
{

  if (use_first)
  {
    l4_sleep(5000);
    use_first = 0;
    for (int i = 0; i < 50; ++i)
    {
      printf("\n");
    }
  }

  l4_msgtag_t tag;
  ios >> tag;

  l4_umword_t task_no;
  l4_umword_t thread_no;
  ios >> task_no;
  ios >> thread_no;

  thread* t = threads[task_no * MThreads + thread_no];
  t->count++;
  if (t->count > max)
    max = t->count;
  print++;
  if (print > 150)
  {
    
    printf("\033[H0\n");
    printf("PLOT: %d\n", plots);
    plots++;
    for (int i = 0; i < MThreads+FThreads; ++i)
    {
      printf("                                                          \n");
    }
    printf("\033[H0\n");
    printf("\n");
    for (int i = 0; i < MThreads; ++i)
    {
      printf("\033[31m%s-%lu:", TASKS[threads[i]->thread], threads[i]->task);
      for (unsigned j = 0; j < (threads[i]->count * 50) / max; ++j)
      {
        printf("|");
      }
      printf("\033[0m\n");
      threads[i]->count = 0;
    }
    for (int i = 0; i < FThreads; ++i)
    {

      printf("\033[32m%s-%lu:", TASKS[threads[i + MThreads]->thread], threads[i + MThreads]->task);
      for (unsigned j = 0; j < (threads[i + MThreads]->count * 50) / max; ++j)
      {
        printf("|");
      }
      printf("\033[0m\n");
      threads[i + MThreads]->count = 0;
    }
    print = 0;
  }

  return 0;

}





static L4Re::Util::Registry_server<> server;

int
main()
{

  for (int i = 0; i < MThreads; ++i)
  {
    threads[i] = new thread(0, i, 0);
  }
  for (int i = 0; i < FThreads; ++i)
  {
    threads[i + MThreads] = new thread(1, i, 0);
  }


  static Test_Server srv;

  if (!server.registry()->register_obj(&srv, "ACTestServer").is_valid())
  {
    printf("Could not register my service, is there a 'ACTestServer' in the caps table?\n");
    return 1;
  }

  // Wait for client requests
  server.loop();

  return 0;
}
