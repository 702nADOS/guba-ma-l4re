#include "l4re_shared_ds_server.h"

#include <l4/dom0-main/communication_magic_numbers.h>
#include <l4/dom0-main/ipc_protocol.h>
#include "jsmn.h"
#include "lua_ipc_client.h"

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>

/* json helper functions */
static int jsoneq(const char *json, jsmntok_t *tok, const char *s);
static char* jsongetstring(const char *json, jsmntok_t *tok);
static int jsonreadstring(const char *json, jsmntok_t *tok, int* pos, const char *s, char** t);
static int jsonreadint(const char *json, jsmntok_t *tok, int* pos, const char *s, int* t);

L4reSharedDsServer::L4reSharedDsServer(const char* capName,
    L4::Cap<L4Re::Dataspace>& ds) :
  ipcServer(ds, elfDataSpace)
{
  this->capName = capName;
}

bool L4reSharedDsServer::init()
{
  if (!registryServer.registry()->register_obj(&ipcServer, capName).is_valid())
  {
    printf("Could not register my service, is there a 'l4re_ipc' in the caps table?\n");
    return false;
  }
  printf("Welcome to the shared ds server for l4re!\n");
  return true;
}

void L4reSharedDsServer::loop()
{
  registryServer.loop();
}

//Tell if l4re has already finished copying the contents of the shared ds.
//If true is returned, it can be safely reused.
bool L4reSharedDsServer::dsInUse()
{
  return ipcServer.dsInUse();
}

void L4reSharedDsServer::startTasks(LuaIpcClient& luaIpc){
  //for (std::vector<taskDescription*>::iterator it = tasks.begin(); it != tasks.end(); ++it) {
  taskDescription* td;
  while(!tasks.empty()){
    td = tasks.back();
    std::string lua_start="L4.default_loader:start({caps={l4re_ipc = L4.Env.l4re_ipc}},\"";
    lua_start = lua_start + std::string(td->binaryName);
    lua_start = lua_start + std::string(":n\");");
    luaIpc.executeString(lua_start.c_str());
    tasks.pop_back();
 }
}

void L4reSharedDsServer::parseTaskDescriptions(char *json){
  printf ("parsing the json file");
  jsmn_parser p;
  jsmntok_t t[128];

  int res, pos;

  jsmn_init(&p);
  res = jsmn_parse(&p, json, strlen(json), t, sizeof(t)/sizeof(t[0]));

  if(res < 0){
    std::cout << "Failed to parse";
    return;
  }

  if(res<1 || t[0].type !=JSMN_ARRAY){
    std::cout << "Array expected";
    return;
  }

  pos = 1;

  for(int j=0; j < t[0].size; j++){
    if(t[pos].type != JSMN_OBJECT){
      pos++; j--; continue;
    }

    pos++;
    taskDescription* td = new taskDescription();
    jsonreadint(json, t, &pos, "id", &td->id);
    jsonreadint(json, t, &pos, "executiontime", &td->executionTime);
    jsonreadint(json, t, &pos, "criticaltime", &td->criticalTime);
    jsonreadint(json, t, &pos, "priority", &td->priority);
    jsonreadint(json, t, &pos, "period", &td->period);
    jsonreadint(json, t, &pos, "offset", &td->offset);
    jsonreadint(json, t, &pos, "matrixSize", &td->matrixSize);
    jsonreadstring(json, t, &pos, "pkg", &td->binaryName);
    tasks.push_back(td);
  }

  for (std::vector<taskDescription*>::iterator it = tasks.begin(); it != tasks.end(); ++it) {
    std::cout << (*it)->id;
    std::cout << (*it)->matrixSize;
    std::cout << (*it)->binaryName;
 }
}

L4::Cap<L4Re::Dataspace>& L4reSharedDsServer::getDataSpaceFor(std::string binary_name){

  elfDataSpace.insert(std::make_pair(binary_name,
        L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>()));
  //elfDataSpace[binary_name] = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
  if(! (elfDataSpace[binary_name]).is_valid()){
    printf("Dataspace allocation failed\n");
  } else {
    printf("Dataspace allocated for %s\n", binary_name.c_str());
  }

  return elfDataSpace.find(binary_name)->second;
}


L4reSharedDsServer::L4reIpcServer::L4reIpcServer(L4::Cap<L4Re::Dataspace>& ds, std::map <std::string,L4::Cap<L4Re::Dataspace> >& eds) :
  dsIsInUse(false), ds(ds), elfDataSpace(eds)
{
}

int L4reSharedDsServer::L4reIpcServer::dispatch(l4_umword_t,
    L4::Ipc::Iostream& ios)
{
  l4_msgtag_t t;
  ios >> t;
  char binary_name[8];
  // We're only talking the l4reIpc protocol
  if (t.label() != Protocol::l4reIpc)
    return -L4_EBADPROTO;

  L4::Opcode opcode;
  ios >> opcode;
  unsigned long int size = sizeof(binary_name);
  switch (opcode)
  {
    case Opcode::getSharedDataspace:
      //ios >> binary_name;
      ios >> L4::Ipc::buf_cp_in<char>(binary_name, size);
      if (strcmp(binary_name,"network") == 0)
      {
        printf("Starting Network \n");
        //Mark the ds used until l4re has copied its content
        dsIsInUse=true;
        ios << ds;
      } else {
        printf("Startinng %s \n",binary_name);
        //Write the dataspace capability into the stream. The kernel will map it to the other task.
        //XXX: if binary_name is not valid, EXIT or send dummy
        //ios << elfDataSpace.find(binary_name)->second;
        if(elfDataSpace.find(binary_name) != elfDataSpace.end())
          ios << elfDataSpace.find(binary_name)->second;
        else
          return -L4_ENOSYS;
      }
      return L4_EOK;
    case Opcode::dataspaceCopyFinished:
      //The l4re thread has finished copying the contents of the shared dataspaces;
      //we can now reuse it for something else.
      dsIsInUse=false;
      return L4_EOK;
    default:
      return -L4_ENOSYS;
  }
}


bool L4reSharedDsServer::L4reIpcServer::dsInUse()
{
  return dsIsInUse;
}



/* helper functions to read the json file
 * taken from libac */

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

static char* jsongetstring(const char *json, jsmntok_t *tok) {
  char* c = (char*) malloc(sizeof(char) * (tok->end - tok->start + 1));
  memcpy(c, json + tok->start, tok->end - tok->start);
  c[tok->end - tok->start] = 0x00;
  return c;
}

static int jsonreadstring(const char *json, jsmntok_t *tok, int* pos, const char *s, char** t) {
  if (jsoneq(json, tok + (*pos), s) != 0)
  {
    printf("\"%s\" != \"%s\"\n", jsongetstring(json, tok + (*pos)), s);
    printf("JSON ERROR!\n");
    return -1;
  }
  (*pos)++;
  *t =  jsongetstring(json, tok + (*pos));
  (*pos)++;
  return 0;
}

static int jsonreadint(const char *json, jsmntok_t *tok, int* pos, const char *s, int* t) {
  if (jsoneq(json, tok + (*pos), s) != 0)
  {
    printf("\"%s\" != \"%s\"\n", jsongetstring(json, tok + (*pos)), s);
    printf("JSON ERROR!\n");
    return -1;
  }
  (*pos)++;
  char* string_buf = jsongetstring(json, tok + (*pos));
  *t =  strtoul(string_buf, NULL, 0);
  free(string_buf);
  (*pos)++;
  return 0;
}
