#include <l4/sys/capability>
#include <l4/re/env>

#pragma once

#define ADMCONTROL_FACTORY_PROXY 	0
#define ADMCONTROL_SCHEDULER_PROXY 	1

/** File to be used by ned and l4re_kernel.
*	Turning ADM-CONTROL off with methods in one module doesnt have any effect on other modules (it's a local library).
* 	Use macros above or module.list to turn admission control off.
*/

void ac_set_turned_off();

bool ac_get_turned_off();

void ac_set(l4_cap_idx_t _ac);

l4_cap_idx_t ac_get();

bool is_valid(l4_cap_idx_t _c);

void set_valid();


l4_fpage_t fpage(l4_cap_idx_t _c);

l4_fpage_t fpage(l4_cap_idx_t _c, unsigned rights);
