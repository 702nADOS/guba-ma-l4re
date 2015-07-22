

#pragma once

#include <l4/cxx/hlist>
#include <l4/cxx/ipc_server>
#include <l4/libkproxy/factory_svr>


#include "global.h"

class Cap_allocator :
	public L4kproxy::Cap_allocator_interface
{
public:
  L4::Cap<void> cap_alloc();
  void cap_free(L4::Cap<void>);
  ~Cap_allocator() {

  }
};

class Fac_proxy :
	public virtual L4::Server_object,
	public L4kproxy::Factory_svr,
	public L4kproxy::Factory_interface,
	public cxx::H_list_item
{
public:
	Fac_proxy(Cap_allocator* capp);
	~Fac_proxy(){
		
	}

	int create_task(L4::Cap<L4::Task> const &target_cap,
	                l4_fpage_t const &utcb_area);

	int create_thread(L4::Cap<L4::Thread> const &target_cap);

	int create_factory(L4::Cap<L4::Factory> const &target_cap,
	                   unsigned long limit);

	int create_gate(L4::Cap<L4::Kobject> const &target_cap,
	                L4::Cap<L4::Thread> const &thread_cap,
	                l4_umword_t label);

	int create_irq(L4::Cap<L4::Irq>const &target_cap);

	int create_vm(L4::Cap<L4::Vm>const &target_cap);

	int dispatch(l4_umword_t o, L4::Ipc::Iostream &ios)
	{ 
		return factory_dispatch(o, ios); }

	L4::Cap<L4::Thread> received_thread(L4::Ipc::Snd_fpage const &fp);

private:

};

