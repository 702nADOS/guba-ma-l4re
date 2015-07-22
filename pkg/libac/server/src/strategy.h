#pragma once

#include <l4/sys/scheduler.h>
#include <l4/re/env.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <l4/cxx/string>
#include "jsmn.h" //using jsmn for json strategy table
///#include "ac.h"
#include "global.h"
#include "list.h"

#define MAX_JSON_TOKEN_SIZE (1024)
#define MAX_FILE_SIZE (4096)


/*	Implementing strategy strategy package for admission control consisting of multiple classes/structs:
*		task_rights:	right of task to start another task (all_rights for every task)
*		task:			task member of task_class
*		task_class:		list of task with special execution 
*		Strategy:		main class for handling task_classes
*		
*/


typedef list<char*>	textList;

typedef struct _task_rights
{
	char *name;
	l4_umword_t all_rights;
	textList* start;
} task_rights;

typedef list<task_rights*>	rightsList;

typedef struct _task
{
	char *name;
	l4_sched_param_type_t type;
	char *alias;
	l4_umword_t prio_or_deadline;
} task;

typedef list<task*>			tList;

typedef struct _task_class
{
	char *name;
	l4_umword_t cpuBinding;
	l4_umword_t schedulerBind;
	l4_umword_t prio;
	l4_sched_param_type_t scheduler;
	l4_umword_t cpu; //cpu Nr. when exeeded, taken modulo
	l4_umword_t size;
	l4_umword_t map;
	tList* my_tasks;

} task_class;




typedef list<task_class*> 	cList;





class Strategy {
private:
	char* _case_name;
	char* _file_name;
	unsigned  _cpus;
	l4_umword_t _cpu_map;
	l4_umword_t _map;

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

	static int jsonreadint(const char *json, jsmntok_t *tok, int* pos, const char *s, l4_umword_t* t) {
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

	static int jsonreadscheduler(const char *json, jsmntok_t *tok, int* pos, const char *s, l4_sched_param_type_t* t) {
		if (jsoneq(json, tok + (*pos), s) != 0)
		{
			printf("\"%s\" != \"%s\"\n", jsongetstring(json, tok + (*pos)), s);
			printf("JSON ERROR!\n");
			return -1;
		}
		(*pos)++;
		char* string_buf = jsongetstring(json, tok + (*pos));
		*t =  sToSched(string_buf);
		free(string_buf);
		(*pos)++;
		return 0;
	}

	static l4_sched_param_type_t sToSched(const char *c){
		if (strcmp(c, "Deadline") == 0)
		{
			return Deadline;
		}
		return Fixed_prio;
	}

public:
	cList* my_classes;
	rightsList* rights;

	/*if strategy not loaded from filesystem set fromfile to 0*/
	Strategy(const char* s, unsigned fromfile);

	Strategy(const char* s);

	~Strategy();

	/*may task1 start task2*/
	int may_start(const char* task1, const char* task2); 

	/*returns the rights list*/
	rightsList* get_rights();

	/*prints the table of rights on the screen*/
	void printRights();

	/*get name of strategy*/
	char* get_name();

	/*get class list*/
	cList* get_list();

	/*print all classes and task on the screen*/
	void printClass(unsigned i, unsigned j);

	/*count cpus, need for setup*/
	unsigned countCPUs(); //How many CPU-cores are on the system


	int map();

	/*returns handle to class identified with literals*/
	task_class* contains_class(char*);

	/*setting up the strategy table from file*/
	void readStrategyTable(); //reading JSON-formatted strategy table (easier to parse)

	/*main method for return the scheduler parameters concerning the strategy to a task concerning its wished parameter*/
	l4_sched_param_t get_scheduler(const char *name, l4_sched_param_t s);//find scheduler and core concerning sched_param
};

typedef list<Strategy*> 	sList;
