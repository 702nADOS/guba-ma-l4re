#pragma once

#include <l4/sys/capability>
#include <l4/sys/l4int.h>
#include <stddef.h>

class MonIpcClient
{
public:
	MonIpcClient(const char* capName);
	bool init();
  int getMonitoring();

private:
	L4::Cap<void> monIpcServer;
	const char* name;
};
