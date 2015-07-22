/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "boot_fs.h"
#include "loader_elf.h"
#include "debug.h"

#include <cstring>
#include <stdio.h>
#include <l4/libloader/elf>


bool
Elf_loader::launch(App_task *t, cxx::String const &prog,
                   cxx::String const &args)
{
	Dbg ldr(Dbg::Loader, "ldr");
	Moe_x_app_model am(t, prog, args);
	Ldr::Elf_loader<Moe_x_app_model, Dbg>::launch(&am, "rom/l4re", ldr);
	return true;
}

L4::Cap<L4::Thread>
Elf_loader::launchac(App_task *t, cxx::String const &args)
{
	cxx::String progi("rom/acstand");
	Dbg ldr(Dbg::Loader, "ldr");
	Moe_x_app_model am(t, progi, args);

	/*_stack.push(L4Re::Env::Cap_entry((*i).name().name(), s));
    s += L4_CAP_OFFSET;
    t->map(L4Re::This_task, (*i).obj()->cap().fpage(L4_CAP_FPAGE_RWS), L4::Cap<void>(s).snd_base());
    s += L4_CAP_OFFSET;*/
	Ldr::Elf_loader<Moe_x_app_model, Dbg>::launch(&am, "rom/acstand", ldr);
	L4::Cap<L4::Thread> cap = t->thread_cap();
	ac_set(cap.cap());
	//ac_set_task(t->task_cap().cap());
	return cap;
}

bool
Elf_loader::check_file_type(Moe::Dataspace const *file) const
{
	char const *data = file->address(0).adr<char const*>();
	return memcmp(data, "\177ELF", 4) == 0;
}
