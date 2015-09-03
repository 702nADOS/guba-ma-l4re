#include <l4/sys/l4int.h>
//#include <l4/sys/ipc.h>
//#include <stdio.h>


struct monitor{
  l4_uint64_t idle_time;
};
/*
L4::Ipc::Istream& operator>>(L4::Ipc::Istream& is, monitor& m);
L4::Ipc::Ostream& operator<<(L4::Ipc::Ostream& os, monitor& m);

L4::Ipc::Istream& operator>>(L4::Ipc::Istream& is, monitor& m)
{
  monitor mon;
  is >> mon.idle_time;
  m = mon; // could do more validation here
  return is;
}

L4::Ipc::Ostream& operator<<(L4::Ipc::Ostream& os, monitor& m)
{
  return os << m.idle_time;
}
*/
