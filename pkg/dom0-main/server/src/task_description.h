#pragma once

#include <string>

//Since pthread_create only accepts functions with a single void* argument,
//all arguments need to be packed into a structure.

struct taskDescription
{
  int id;
  int executiontime;
  int criticaltime;
  int priority;
  int period;
  int offset;
  std::string binary_name;
};
