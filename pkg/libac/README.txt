Implementation of the Admission Control for the L4re/Fiasco.OC system.

Folder:

include/ 
	admcontrol_protocol.h : Library for using the admission control protocol.
	bootup.h : macros and helper methods for booting up admission control.
lib/
	bootup.cc : compiled helper methods
server/
	src/	Main implementatin of die admission control standalone module "acstand".
testing/
	test/			Show the dynamic task execution order on the scvreen with a test server
	test-execution_to_csv/ 	Make sample threads an meassure their time. Log output on screen in csv format.
		

The following preconditions musst be fullfilled:
	-module.list needs to have acstand and standard.json as modules
	-admission control needs to be loaded by someone, use modified implementatin of moe for this
	-task needs to register at admission control, use modified version of l4re_kernel for this


Admission Control uses alien code from the following sources:
	-scheduler_proxy.cc scheduler_proxy.h from l4/pkg/loader 
	-jsmn.h jsmn.c from JSMN-project for parsing json files (see https://bitbucket.org/zserge/jsmn/wiki/Home and JSMN_LICENSE)


fixed issues:
	-libproxy/lib/src/factory_proxy.cc see handle_task 
	-libkproxy/include/scheduler_svr see SCHEDULER_RUN_THREAD_OP

known issues in admission control:

	-tasks might not be able to get capability provided by ned (turn factory_proxy in bootup.h off and recompile moe and l4re_kernel)
	-on rare occasions to many threads dont work

known issues in alien code:

	-EDF-Threads crash (page fault) when they don't implement the edft_exit_thread() method. I suppose the instruction pointer doesn't stop executing. edft_exit_thread puts the threads in while(1){sleep(1)}
	

