#include "dom0_server.h"

#include "tcp_server_socket.h"
#include "lua_ipc_client.h"
#include "thread_args.h"

#include <l4/re/env>
#include <l4/re/util/cap_alloc>

#include <l4/util/util.h>


#include <l4/dom0-main/communication_magic_numbers.h>
#include <l4/dom0-main/ipc_protocol.h>
#include <pthread-l4.h>



//This function is the core of dom0.
//It receives commands from a connected
//dom0 client (on another machine), and processes
//and answers them accordingly.
void* dom0Server(void* args)
{
	//The socket for network communication with the client.
	TcpServerSocket myServer(((serverThreadArgs*) args)->address,
			((serverThreadArgs*) args)->port);

	//The dataspace (shared with l4re instances of custom tasks) which
	//will later contain binaries received over the network.
	L4::Cap<L4Re::Dataspace>& ds=*((serverThreadArgs*) args)->dataspace;

	//Connection to the local LUA Ipc Server
	LuaIpcClient& luaIpc = *((serverThreadArgs*) args)->luaIpcClient;

	L4reSharedDsServer& dsServer = *((serverThreadArgs*) args)->dsServer;

	MonIpcClient& monIpc = *((serverThreadArgs*) args)->monIpcClient;

  pthread_t monThread;

	//Binary commands from the communication partner will be stored here
	int32_t message = 0;

	//LUA commands from the communication partner will be stored here
	char buffer[1024];

	bool dsIsValid=false;
	char *space = 0;

	while (true)
	{
		myServer.connect();

    {
      //start the onitoring thread
      monThreadArgs monArgs;
      monArgs.monIpcClient = &monIpc;
      monArgs.myServer = &myServer;

      pthread_create(&monThread, NULL, monLoop, &monArgs);
    }

		while (true)
		{
			//The first 4 Byte of a new message always
			//tell us what the message contains,
			//either a LUA or a binary (control) command
			NETCHECK_LOOP(myServer.receiveInt32_t(message));
      printf("Message Received %d %d \n",message,LUA);
			if (message == LUA)
			{
        printf("LUA Message received");
				//The next four bytes contain the length
				//of the LUA string (including terminating zero)
				NETCHECK_LOOP(myServer.receiveInt32_t(message));
				//Now, receive the LUA string
				NETCHECK_LOOP(myServer.receiveData(buffer, message));
				printf("LUA received: %s\n", buffer);
				//And send it to our local Ned instance.
				if(luaIpc.executeString(buffer))
					message=LUA_OK;
				else
					message=LUA_ERROR;
				//Answer, if Ned received the message successfully
				//or not (e.g. because the string was too long)
				NETCHECK_LOOP(myServer.sendInt32_t(message));
			}
			else if (message == CONTROL)
			{
        printf("COntrol Message received");
				NETCHECK_LOOP(myServer.receiveInt32_t(message));
				if (message == SEND_BINARY)
				{
					//The communication partner wants to send us a binary.


					//Check, if the shared ds is still being used by l4re.
					//If yes, sleep until it isn't.
					//Could be done more elegantly (and complex), but works,
					//and is almost never needed.
					if(dsServer.dsInUse())
						l4_sleep(50);


					//Detach and free dataspace so that we can reuse it for the new binary
					if(dsIsValid)
					{
						printf("Freeing shared dataspace\n");
						message = L4Re::Env::env()->rm()->detach(space, &ds);
						if (message)
						{
							printf("Error freeing shared memory!\n");
							return NULL;
						}
						message = L4Re::Env::env()->mem_alloc()->free(ds);
						if (message)
						{
							printf("Error freeing shared memory!\n");
							return NULL;
						}
						dsIsValid=false;
						printf("...free'd\n");
					}


					//Get the size of the binary.
					NETCHECK_LOOP(myServer.receiveInt32_t(message));
					printf("Binary size will be %i Bytes.\n", message);


					//Allocate memory for our DS
					message = L4Re::Env::env()->mem_alloc()->alloc(message, ds, 0);
					if (message < 0)
					{
						printf("mem_alloc->alloc() failed.\n");
						L4Re::Util::cap_alloc.free(ds);
						return NULL;
					}
				    //Attach DS to local address space
					message = L4Re::Env::env()->rm()->attach(&space, ds->size(),
							L4Re::Rm::Search_addr, ds);
					if (message < 0)
					{
						printf("Error attaching data space: %s\n",
								l4sys_errtostr(message));
						L4Re::Util::cap_alloc.free(ds);
						return NULL;
					}

					dsIsValid=true;

					//Success
					printf("Attached DS, size: %li\n", ds->size());

					//Our partner can now start sending.
					NETCHECK_LOOP(myServer.sendInt32_t(GO_SEND));

					//Get it...
					NETCHECK_LOOP(myServer.receiveData(space, (ssize_t) ds->size()));
					printf("Binary received.\n");

				}
			}
		}

    // cancel the monitoring thread
    printf("===== CANCELLING MONITORING THREAD =====\n");
    pthread_cancel(monThread);
	}
	//clean up
	myServer.disconnect();
	return NULL;
}

//This function is the core of dom0.
//It receives commands from a connected
//dom0 client (on another machine), and processes
//and answers them accordingly.
void* monLoop(void* args)
{
  MonIpcClient& monIpc = *((monThreadArgs*) args)->monIpcClient;
  TcpServerSocket& myServer = *((monThreadArgs*) args)->myServer;
  int monitoring_data;

  while(true){
    // if the network disconnects, then stop the thread
    pthread_testcancel();

    monitoring_data = monIpc.getMonitoring();
    printf("%d\n",monitoring_data);

    // send monitoring data over the channel
    NETCHECK_LOOP(myServer.sendInt32_t(monitoring_data));

    l4_sleep(500);
  }

  return NULL;
}
