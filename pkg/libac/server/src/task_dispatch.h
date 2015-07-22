/*dispatch for a single task
*
* Variables:
* 	_task_name: 	literal identification of task (for example rom/hello)
*	_cap:			ipc-gate of task
*	main_strategy:	indirekt pointer to strategy; actual pointer might be changed by main_dispatcher later
*/


#pragma once

#include <l4/sys/task.h>
#include <l4/sys/factory.h>
#include <l4/sys/types.h>

#include <string.h>

#include <l4/cxx/ipc_server>
#include <l4/cxx/iostream>
#include "sched_proxy.h"
#include "factory_proxy.h"
//#include "ac.h"
#include "strategy.h"



class Task_dispatch : public Sched_proxy, public Fac_proxy
{

private:
	char* _task_name;
	L4::Cap<void> _cap;
	unsigned long _limit;
	Strategy** main_strategy;


public:

	//Sched_proxy schedp;
	Cap_allocator capp;

	Task_dispatch(char* n, Strategy** s) : Fac_proxy(&capp), main_strategy(s)
	{
		_task_name = n;//(char*) malloc((strlen(n) + 1) * sizeof(char));
		//strcpy(_task_name,n);
	}

	char* task_name()
	{
		return _task_name;
	}

	L4::Cap<void> get_cap()
	{
		return _cap;
	}

	void set_cap(L4::Cap<void> cap)
	{
		_cap = cap;
	}


	int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios)
	{

		l4_msgtag_t tag;
		ios >> tag;

		if (tag.label() == L4_PROTO_IRQ)
		{
#if DEBUG
			printf("L4_PROTO_IRQ\n");
#endif
			return Sched_proxy::dispatch(obj, ios);
		}
		if (tag.label() == L4_PROTO_SCHEDULER)
		{
#if DEBUG
			printf("L4_PROTO_SCHEDULER\n");
#endif
			return Sched_proxy::dispatch(obj, ios);
		}
		if (tag.label() == L4_PROTO_FACTORY)
		{
#if DEBUG
			printf("L4_PROTO_FACTORY\n");
#endif
			return Fac_proxy::dispatch(obj, ios);
		}

		return 0;
	}


	/*int create_factory(L4::Cap<L4::Factory> const &target_cap,
	                   unsigned long limit)
	{
		L4::Cap<L4::Factory> cap = L4::cap_cast<L4::Factory>(_cap);
		target_cap.move(cap);
		_limit = limit;
		return 0;
		return 0;

	}*/


	int run_thread(L4::Cap<L4::Thread> thread, l4_sched_param_t const &sp)
	{
		l4_sched_param_t s = (*main_strategy)->get_scheduler(_task_name, sp);
		if (!(*main_strategy)->map()) {
			s = sp;
		}
#if RUNSTATE
		printf("RUN THREAD FOR %s, PRIO %d, OLDPRIO %d\n", _task_name, s.prio, sp.prio);
#endif
		/*if (strcmp(_task_name, "rom/testAC") == 0)
		{
			if (s.prio == 255)
			{
				s.prio = 300;
			}
			if (s.prio == 2)
			{
				s.prio = 299;
			}
		}*/

#if DEBUG
		printf("loader[%p] run_thread: o=%u scheduler affinity = %lx sp.m=%lx sp.o=%u sp.g=%u\n",
		       this, sp.affinity.map, sp.affinity.offset, sp.affinity.map, (unsigned)sp.affinity.offset, (unsigned)sp.affinity.granularity);
		printf("loader[%p]                                      sp.m=%lx sp.o=%u sp.g=%u\n",
		       this, s.affinity.map, (unsigned)s.affinity.offset, (unsigned)s.affinity.granularity);
#endif

		//s.scheduler = l4_debugger_set_object_name(nasp::rcv_cap.cap(), _task_name.start())
		l4_msgtag_t t = l4_scheduler_run_thread(l4re_env()->scheduler, thread.cap(), &s); //L4Re::Env::env()->scheduler()->run_thread(thread, s);
		int error = l4_error(t);
		/*if (error)
			printf("create task errorcode %i: %s\n", -error, l4sys_errtostr(error));*/


		return error;
	}



};