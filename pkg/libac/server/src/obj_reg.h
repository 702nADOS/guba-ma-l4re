/* similiar to implementation of l4re loader-module*/

#pragma once

#include <l4/cxx/exceptions>
#include <l4/sys/task.h>
#include <l4/sys/factory.h>
#include <l4/sys/types.h>

#include <l4/re/util/bitmap_cap_alloc>

#include <l4/cxx/ipc_server>
#include <l4/cxx/iostream>

#include "task_dispatch.h"

#include <cstring>
#include <cassert>
#include <cstdio>

#define DEBUG_CAP_ALLOC 0



/*helper class for object_pool*/
class Cap_alloc
{

public:

  L4::Cap<L4::Kobject> alloc()
  {
    L4::Cap<L4::Kobject> cap(l4re_util_cap_alloc());
    return cap;
  }

  template< typename T >
  L4::Cap<T> alloc() { return L4::cap_cast<T>(alloc()); }

  L4::Cap<L4::Kobject> alloc(L4::Server_object *o)
  {
    L4::Cap<L4::Kobject> cap = alloc();
    if (!cap.is_valid())
      throw(L4::Out_of_memory());

    l4_umword_t id = l4_umword_t(o);
    l4_factory_create_gate(l4re_env()->factory, cap.cap(), l4re_env()->main_thread, id);
    o->obj_cap(cap);
    return cap;
  }
/*
  bool free(L4::Cap<void> const &cap)
  {
    return true;
  }

  bool free(L4::Server_object *o)
  {
    return true;
  }

  bool is_allocated(L4::Cap<void> c)
  {
    return true;
  }*/

  static L4::Server_object* find(l4_umword_t label)
  {

    L4::Server_object* o = NULL;
    o = L4::Basic_registry::find(label);
    return o;
  }
  /*

  Cap_alloc() {

  }

  long hint() const { return 0;}*/

};


/* class for identifing L4::Server_objects with labels*/
class Object_pool : public L4::Basic_registry
{
public:
  Object_pool(Cap_alloc *ca) : _cap_alloc(ca) {}
  Cap_alloc *cap_alloc() const { return _cap_alloc; }
private:
  Cap_alloc *_cap_alloc;
};



Object_pool* pool(void);