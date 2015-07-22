/**
scheduler test
 */
#include <l4/libedft/edft.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <l4/sys/l4int.h>
#include <l4/util/rdtsc.h>

#define fpn (3)
#define n (fpn)
#define bedZ (15000)


long unsigned times[n][bedZ];

void thread_func(l4_umword_t);

unsigned prios[n] = {30,80,90};

struct thread
{
    l4_sched_param_t sp;
    Edf_thread t;
};


struct thread t[n];

void thread_func(l4_umword_t no)
{
    unsigned i;
    l4_sleep(300);
    for (i = 0; i < bedZ; ++i)
    {
        times[no][i] = l4_rdtsc();
    }


    edft_exit_thread();
}




int main(void)
{
    unsigned i, j;


    
    for (i = 0; i < fpn; i++)
    {
        t[i].t.no = i;

        sprintf(t[i].t.name, "thread%i", i);
        t[i].t.wcet = 5;
        t[i].t.func = thread_func;
        t[i].t.arg = i;
        t[i].t.suc = NULL;
        t[i].t.thread_cap = 0;
        edft_create_l4_thread(&(t[i].t), 0);

    }

    printf("WAIT 5 SECONDS FOR THREADS TO RUN, AND ENOUGH TIME FOR FILE TO POPULATE!\n");
    for (i = 0; i < fpn; i++)
    {
        t[i].sp = l4_sched_param_by_type(Fixed_prio, prios[i], 0);
        if (l4_error(l4_scheduler_run_thread(l4re_env()->scheduler, t[i].t.thread_cap, &t[i].sp)))
            return -1;
    }

    l4_sleep(10000);

    for (i = 0; i < fpn ; i++)
    {
        for (j = 0; j < bedZ; j++)
        {
            printf("%d;%lu\n", i, times[i][j]);
        }
    }
    return 0;
}
