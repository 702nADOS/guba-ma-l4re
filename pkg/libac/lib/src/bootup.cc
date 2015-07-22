
#include <l4/re/env.h>
#include <l4/sys/factory.h>
#include <l4/libac/bootup.h>



l4_cap_idx_t ac_cap = 0UL;
l4_cap_idx_t ac_task_cap = 0UL;

l4_cap_idx_t ac_gate = 0UL;

bool valid = false;

bool turned_off = false;


void ac_set_turned_off(){
	turned_off = true;
}

bool ac_get_turned_off(){
	return turned_off;
}


void ac_set(l4_cap_idx_t _ac)
{
	ac_cap = _ac; 
}


l4_cap_idx_t ac_get()
{
	return ac_cap;
}

bool is_valid(l4_cap_idx_t _c)
{
	
	return valid&&(!turned_off);//!(_c & L4_INVALID_CAP_BIT);
}

void set_valid()
{
	
	valid = true;//!(_c & L4_INVALID_CAP_BIT);
}

l4_fpage_t fpage(l4_cap_idx_t _c)
{ return l4_obj_fpage(_c, 0, L4_FPAGE_RWX); }


l4_fpage_t fpage(l4_cap_idx_t _c, unsigned rights)
{ return l4_obj_fpage(_c, 0, rights); }
