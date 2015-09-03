#include "mon_ipc_client.h"

#include <l4/re/util/cap_alloc>
#include <l4/re/env>
#include <l4/cxx/ipc_stream>
#include <l4/dom0-main/communication_magic_numbers.h>
#include <l4/dom0-main/ipc_protocol.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>


MonIpcClient::MonIpcClient(const char* capName) :
		monIpcServer(L4_INVALID_CAP)
{
	this->name = capName;
}

bool MonIpcClient::init()
{
	monIpcServer = L4Re::Env::env()->get_cap<void>(name);
	if (!monIpcServer.is_valid())
	{
		printf("Could not get IPC gate capability for Monitoring!\n");
		return false;
	}
  printf("Got IPC capability for Monitoring! \n");
	return true;
}

monitor* MonIpcClient::getMonitoring(){
  int32_t op;
  monitor* data = new monitor;
	L4::Ipc::Iostream monstream(l4_utcb());

	op = l4_error(monstream.call(monIpcServer.cap()));
	if (op)
		return false; // failure

	monstream >> data->idle_time;
	return data;
}
