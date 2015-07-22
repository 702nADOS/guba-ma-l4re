
#include <l4/crtn/crt0.h>
#include <l4/util/util.h>
#include <l4/re/env.h>

#include <l4/sys/kip>
#include <l4/sys/utcb.h>
#include <l4/sys/debugger.h>
#include <l4/sys/scheduler>
#include <l4/sys/thread>

#include <l4/cxx/exceptions>
#include <l4/cxx/ipc_stream>

#include <l4/cxx/string>

#include <l4/cxx/iostream>
#include <l4/cxx/l4iostream>

#include <l4/util/mb_info.h>
#include <l4/cxx/ipc_server>

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>

#include <l4/sys/ipc.h>
#include <l4/sys/thread.h>
#include <l4/sys/factory.h>
#include <l4/sys/utcb.h>
#include <l4/re/env.h>
#include <l4/re/c/util/cap_alloc.h>
#include <l4/sys/task.h>
#include <l4/sys/thread.h>
#include <l4/sys/ipc_gate.h>
#include <l4/libedft/edft.h>

#include "sched_proxy.h"
#include "factory_proxy.h"
#include <l4/util/util.h>

#include "global.h"
#include "strategy.h"

#include "obj_reg.h"
#include "list.h"
#include "task_dispatch.h"
#include <l4/libac/admcontrol_protocol.h>


L4::Cap<void> nasp::rcv_cap;

/*	Main_dispatcher is the main object to dispatch ipc request
*	
*	Supports 3 protocols:
*		scheduler protocoll
*		factory protocol
*		admission control protocol
*
*/

class Main_dispatcher
{
private:

	Object_pool* 	_pool;
	char* 			_roottask;
	l4_umword_t 	_roottask_label;
	Strategy*		_active_strategy;
	sList* 			strategies;

	void setup_roottask(const char* roottask) {
		_roottask = (char*) malloc(sizeof(char) * (strlen(roottask) + 1));
		memcpy(_roottask, roottask, strlen(roottask) + 1);
		Task_dispatch *o = new Task_dispatch(_roottask, &_active_strategy);
		L4::Server_object *_o = reinterpret_cast<L4::Server_object*>(o);
		_roottask_label = l4_umword_t(_o);
		o->obj_cap(L4::cap_cast<L4::Kobject>(L4Re::Env::env()->main_thread()));
	}

	char* rcv_string(unsigned offset, L4::Ipc::Iostream &ios)
	{
		l4_msgtag_t tag;
		ios >> tag;

		char* c = (char*) malloc(sizeof(l4_umword_t) * (tag.words() - offset));

		for (unsigned i = offset; i < tag.words(); i++)
		{
			l4_umword_t buf;
			ios >> buf;
			for (unsigned j = 0; j < sizeof(l4_umword_t); ++j)
			{
				c[(i - offset)*sizeof(l4_umword_t) + j] = ((char*) & (buf))[j];//copies the l4_umword_t s, in the char array (beginning at the offset)
			}
		}
		return c;
	}



public:

	Main_dispatcher(const char* roottask, Strategy* s) : _active_strategy(s)
	{
		strategies = new sList(s);
		Cap_alloc* _alloc = new Cap_alloc();
		_pool = new Object_pool(_alloc);
		setup_roottask(roottask);
	}

	int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios)
	{
		if (obj == 0) {
#if DEBUG
			L4::cout << _roottask << " COMMUNICATION\n";
#endif
			obj = _roottask_label;
		}
		L4::Server_object* _newo;
		_newo = (_pool->find(obj));
		Task_dispatch* newo = NULL;

		if (_newo != NULL )
		{

			newo = reinterpret_cast<Task_dispatch*>(_newo);
#if DEBUG

			L4::cout << "TASK DISPATCH: "<< newo->task_name() << " \n";
#endif

		}
		else if (obj != 0)
		{
			L4::cout << "\e[31mSecurity violation! No such object: " << obj << "\n\e[0m";
		} else {
#if DEBUG
			L4::cout <<"MOE NOT INITIALISED\n";
#endif
		}


		l4_msgtag_t tag;
		ios >> tag;

		if (tag.label() == L4_PROTO_IRQ)
		{
#if DEBUG
			L4::cout << "L4_PROTO_IRQ\n";
#endif
			return newo->dispatch(obj, ios);
		}
		if (tag.label() == L4_PROTO_SCHEDULER)
		{
#if DEBUG
			L4::cout <<"L4_PROTO_SCHEDULER\n";
#endif
			return newo->dispatch(obj, ios);
		}
		if (tag.label() == L4_PROTO_FACTORY)
		{
#if DEBUG
			L4::cout <<"L4_PROTO_FACTORY\n";
#endif
			return newo->dispatch(obj, ios);
		}
		if (tag.label() == L4_PROTO_ADMCONTROL)
		{
#if DEBUG
			L4::cout <<"L4_PROTO_ADMCONTROL";
#endif
			return handleAdmProto(newo, ios);
		}
		else
			return -L4_EBADPROTO;
	}

	/*
		Responsible for correct dispatching of the Admission Control Protocol
	*/

	int handleAdmProto(Task_dispatch* current, L4::Ipc::Iostream &ios)
	{

		//tag.words();
		//tag.items();
		l4_msgtag_t tag;
		ios >> tag;
		l4_umword_t proto;
		ios >> proto;
#if DEBUG
		L4::cout << "PROTO: " << proto << "\n";
#endif


		if (proto == ADMCONTROL_NEW_CAP_OP) //register new task
		{

			char* c = (char*) malloc(sizeof(l4_umword_t) * (tag.words() - 1));

			for (unsigned i = 1; i < tag.words(); i++)
			{

				l4_umword_t buf;
				ios >> buf;
				for (unsigned j = 0; j < sizeof(l4_umword_t); ++j)
				{
					c[(i - 1)*sizeof(l4_umword_t) + j] = ((char*) & (buf))[j];
				}
			}

			#if RUNSTATE
				L4::cout << "NEW ADMISSION CONTROL TASK: " << c << "\n";
			#endif
			if (!_active_strategy->may_start(current->task_name(), c)) {
				ios << current->get_cap();
				L4::cout << "\e[31mSECURITY VIOLATION ERROR: " << current->task_name() << " MAY NOT START " << c << " (using " << current->task_name() << "'s rights instead)\e[0m\n";
				return L4_EPERM;
			}
			Task_dispatch *o = new Task_dispatch(c, &_active_strategy);
			L4::Server_object *_o = reinterpret_cast<L4::Server_object*>(o); // necessary for dyanmic typing
			
			L4::Cap<void> cap = _pool->cap_alloc()->alloc(_o);
			o->set_cap(cap);
			ios << cap;

		} else if (proto == ADMCONTROL_ACTIVE_STRATEGY_OP) //send name and id strategy
		{
			l4_umword_t no = strategies->pos(_active_strategy);
			ios << no;
			char* name = _active_strategy->get_name();
			//ios << name;
			for (unsigned i = 0; i < (strlen(name) + 1); ++i)
			{

				ios << name[i];
			}

		} else if (proto == ADMCONTROL_SWITCH_STRATEGY_OP) //receive id of strategy to siwtch to
		{
			l4_umword_t no;
			ios >> no;
			if (strategies->valid(no))
			{
				return L4_ENODEV;
			}
			_active_strategy = strategies->get(no);

		} else if (proto == ADMCONTROL_GET_STRATEGYLIST_OP) //send list of current strategies
		{

			unsigned pos = 0;
			sList* s = strategies;
			ios << strategies->size();
			while (s != NULL)
			{
				ios << pos;
				char* name = s->get()->get_name();

				for (unsigned i = 0; i < (strlen(name) + 1); ++i)
				{
					ios << name[i];
				}
				s = s->next();
				pos++;
			}

		}  else if (proto == 9UL) //send class information
		{
			l4_umword_t no;
			ios >> no;
			if (!strategies->valid(no))
			{
				return L4_ENODEV;
			}
			Strategy* s = strategies->get(no);
			cList* cl = s->get_list();


			ios >> no;
			if (!cl->valid(no))
			{
				return L4_ENODEV;
			}

			task_class* c = cl->get(no);
			ios << c->cpuBinding;
			ios << c->schedulerBind;
			ios << c->prio;
			ios << (l4_umword_t)c->scheduler;
			ios << c->cpu;
			ios << c->map;

			char* name = c->name;

			for (unsigned i = 0; i < (strlen(name) + 1); ++i)
			{
				ios << name[i];
			}

		} else if (proto == ADMCONTROL_GET_CLASSLIST_OP) //send list of classes in strategy
		{
			unsigned pos = 0;
			l4_umword_t no;
			ios >> no;
			if (!strategies->valid(no))
			{
				return L4_ENODEV;
			}

			Strategy* s = strategies->get(no);

			cList* c = s->get_list();

			ios << c->size();
			while (c != NULL)
			{
				ios << pos;
				char* name = c->get()->name;
				for (unsigned i = 0; i < (strlen(name) + 1); ++i)
				{
					ios << name[i];
				}
				c = c->next();
				pos++;
			}

		} else if (proto == ADMCONTROL_GET_TASKLIST_OP) //send list of tasks in class
		{
			unsigned pos = 0;
			l4_umword_t no;
			ios >> no;
			if (!strategies->valid(no))
			{
				return L4_ENODEV;
			}
			Strategy* s = strategies->get(no);
			cList* cl = s->get_list();


			ios >> no;
			if (!cl->valid(no))
			{
				return L4_ENODEV;
			}

			tList* t = cl->get(no)->my_tasks;

			ios << t->size();
			while (t != NULL)
			{
				ios << pos;
				char* name = t->get()->name;
				for (unsigned i = 0; i < (strlen(name) + 1); ++i)
				{
					ios << name[i];
				}
				t = t->next();
				pos++;
			}
		} else if (proto == ADMCONTROL_ADD_STRATEGY_OP) //get strategy to add
		{
			Strategy* s = new Strategy(rcv_string(1, ios), 0);
			strategies->add(s);
			ios << strategies->pos(s);
			//printf("ID: %u\n", strategies->pos(s));
		} else if (proto == ADMCONTROL_ADD_CLASS_OP) //get class to add to strategy
		{
			l4_umword_t no;
			ios >> no;
			if (!strategies->valid(no))
			{
				return L4_ENODEV;
			}
			Strategy* s = strategies->get(no);
			task_class* new_class = new task_class();
			ios >> new_class->cpuBinding;
			ios >> new_class->schedulerBind;
			ios >> new_class->prio;
			l4_umword_t sched;
			ios >> sched;
			(new_class->scheduler) = (l4_sched_param_type_t)sched;
			ios >> new_class->cpu;
			ios >> new_class->map;
			new_class->name = rcv_string(8, ios);
			new_class->size = 0;
			task_class* c = s->contains_class(new_class->name);
			if (c != NULL)
			{
				#if RUNSTATE
				L4::cout << s->get_name() << " ALREADY CONTAINS CLASS NAIMED " << new_class->name << "! Class updated\n";
				#endif
				c->cpuBinding = new_class->cpuBinding;
				c->schedulerBind = new_class->schedulerBind;
				c->prio = new_class->prio;
				c->scheduler = new_class->scheduler;
				c->cpu = new_class->cpu;
				c->map = new_class->map;
				ios << s->my_classes->pos(c);
			}
			else if (s->my_classes == NULL)
			{
				s->my_classes = new cList(new_class);
				ios << 0UL; //position
			} else {
				s->my_classes->add(new_class);
				ios << s->my_classes->pos(new_class);
			} 

		} else if (proto == ADMCONTROL_ADD_TASK_OP) //add task to add to class
		{
			l4_umword_t strat_id, class_id;
			ios >> strat_id;
			if (!strategies->valid(strat_id))
			{
				return L4_ENODEV;
			}

			Strategy* s = strategies->get(strat_id);
			cList* cl = s->get_list();

			ios >> class_id;

			if (!cl->valid(class_id))
			{
				return L4_ENODEV;
			}

			task_class* c = cl->get(class_id);
			//tList* tl = c->my_tasks;


			task* new_task = new task();
			l4_umword_t sched;
			ios >> sched;
			(new_task->type) = (l4_sched_param_type_t)sched;
			ios >> new_task->prio_or_deadline;
			new_task->name = rcv_string(5, ios);


			if (c->my_tasks == NULL)
			{
				c->my_tasks = new tList(new_task);
				c->size = 1;
				ios << 0;
			} else {
				c->my_tasks->add(new_task);
				c->size += 1;
				ios << c->my_tasks->pos(new_task);
			}

		}

		return 0;
	}


	~Main_dispatcher()
	{
		delete _pool;
		delete _active_strategy;
		free(_roottask);
	}


















};

/*class for receiving thread capability*/

class Loop_hooks : 
	public L4::Ipc_svr::Ignore_errors,
	public L4::Ipc_svr::Default_timeout,
	public L4::Ipc_svr::Compound_reply
{
public:
	void setup_wait(L4::Ipc::Istream &istr, L4::Ipc_svr::Reply_mode)
	{
		istr.reset();
		istr << L4::Ipc::Small_buf(nasp::rcv_cap.cap(), 0);
		l4_utcb_br_u(istr.utcb())->bdr = 0;
	}
};


static L4::Server<Loop_hooks> server(l4_utcb());
static Main_dispatcher* disp;
static Strategy* main_strategy;

int main(int argc, char** argv)
{
	l4_debugger_set_object_name(l4re_env()->main_thread, "admcon thread");
	l4_debugger_set_object_name(L4_BASE_TASK_CAP, "admcon");

#if RUNSTATE
	L4::cout << "\e[31m Admission Control loading ...\e[0m\n";
#endif

	/*argument of admission control is name of roottask*/
	char const* roottask;
	if (argc < 2)
		roottask = "rom/moe";
	else
		roottask = argv[1];

	/*loading up main strategy ("standard"), main_dispatcher and capability to receive thread*/
	main_strategy 	= new Strategy("standard");
	disp 			= new Main_dispatcher(roottask, main_strategy);
	nasp::rcv_cap 	= L4Re::Util::cap_alloc.alloc<void>();

#if RUNSTATE
	main_strategy->printRights();

	L4::cout << "\e[31mAdmission Control online :: started from " << roottask << "\e[0m\n";
#endif

	/*start server loop*/
	server.loop_noexc(disp);
	return 0;
}