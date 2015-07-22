
/*stores global receive capability*/


#pragma once

#include <l4/sys/capability>
#include <l4/re/util/object_registry>




#define DEBUG 0
#define RUNSTATE 1

namespace nasp {
extern L4::Cap<void> rcv_cap;
}

