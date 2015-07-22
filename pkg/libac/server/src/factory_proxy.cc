/*
* Factory proxy buffers access to kernel factory.
* No modification is done 
*/

#include <l4/cxx/hlist>
#include <l4/cxx/ipc_server>
#include <l4/libkproxy/factory_svr>
#include <stdio.h>
#include <l4/re/c/util/cap_alloc.h>
#include <l4/re/env.h>
#include "factory_proxy.h"

#include "global.h"



L4::Cap<void> Cap_allocator::cap_alloc() {
	//printf("cap alloc\n");
	L4::Cap<void> cap(l4re_util_cap_alloc());
	return cap;
}

void Cap_allocator::cap_free(L4::Cap<void> cap) {
	l4re_util_cap_free(cap.cap());
	//printf("cap free\n");
}

Fac_proxy::Fac_proxy(Cap_allocator* capp) : L4kproxy::Factory_svr(this, capp)
{

}

int Fac_proxy::create_task(L4::Cap<L4::Task> const &target_cap,
                           l4_fpage_t const &utcb_area)
{

	l4_msgtag_t t = l4_factory_create_task(l4re_env()->factory, target_cap.cap(), utcb_area);
	int error = l4_error(t);
	if (error)
		printf("create task errorcode %i: %s\n", -error, l4sys_errtostr(error));
	return error;
}

int Fac_proxy::create_thread(L4::Cap<L4::Thread> const &target_cap)
{
	l4_msgtag_t t = l4_factory_create_thread(l4re_env()->factory, target_cap.cap());
	int error = l4_error(t);
	if (error)
		printf("create thread errorcode %i: %s\n", -error, l4sys_errtostr(error));
	return error;
}

int Fac_proxy::create_factory(L4::Cap<L4::Factory> const &target_cap,
                              unsigned long limit)
{
	l4_msgtag_t t = l4_factory_create_factory(l4re_env()->factory, target_cap.cap(), limit);
	int error = l4_error(t);
	if (error)
		printf("create factory errorcode %i: %s\n", -error, l4sys_errtostr(error));
	return error;
}

int Fac_proxy::create_gate(L4::Cap<L4::Kobject> const &target_cap,
                           L4::Cap<L4::Thread> const &thread_cap,
                           l4_umword_t label)
{
	l4_msgtag_t t = l4_factory_create_gate(l4re_env()->factory, target_cap.cap(), thread_cap.cap(), label);
	int error = l4_error(t);
	if (error)
		printf("create gate errorcode %i: %s\n", -error, l4sys_errtostr(error));
	return error;
}

int Fac_proxy::create_irq(L4::Cap<L4::Irq>const &target_cap)
{
	l4_msgtag_t t = l4_factory_create_irq(l4re_env()->factory, target_cap.cap());
	int error = l4_error(t);
	if (error)
		printf("create irq errorcode %i: %s\n", -error, l4sys_errtostr(error));
	return error;
}

int Fac_proxy::create_vm(L4::Cap<L4::Vm>const &target_cap)
{
	l4_msgtag_t t = l4_factory_create_vm(l4re_env()->factory, target_cap.cap());
	int error = l4_error(t);
	if (error)
		printf("create vm errorcode %i: %s\n", -error, l4sys_errtostr(error));
	return error;
}

L4::Cap<L4::Thread> Fac_proxy::received_thread(L4::Ipc::Snd_fpage const &fp)
{
	if (!fp.cap_received())
		return L4::Cap<L4::Thread>::Invalid;

	return L4::cap_cast<L4::Thread>(nasp::rcv_cap);
}


