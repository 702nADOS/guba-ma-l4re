#include "strategy.h"
#include "string.h"
#include "list.h"

Strategy::Strategy(const char* s, unsigned fromfile) : _map(0)
{
	my_classes = NULL;
	rights = NULL;
	_case_name = (char*) malloc((strlen(s) + 1) * sizeof(char));
	_file_name = (char*) malloc((strlen(s) + 1 + 4 + 4) * sizeof(char));// + "rom/" + ".json"
	strcpy(_case_name, s);
	strcpy(_file_name, "rom/");
	strcat(_file_name, s);
	strcat(_file_name, ".json");
	printf("NEW STRATEGY: %s\n", _case_name);
	_cpus =  countCPUs();
	if (fromfile)
		readStrategyTable();
	else {
		my_classes = NULL;
		rights = NULL;
	}
}

Strategy::Strategy(const char* s) : _map(0)
{
	my_classes = NULL;
	rights = NULL;
	_case_name = (char*) malloc((strlen(s) + 1) * sizeof(char));
	_file_name = (char*) malloc((strlen(s) + 1 + 4 + 4) * sizeof(char));// + "rom/" + ".json"
	strcpy(_case_name, s);
	strcpy(_file_name, "rom/");
	strcat(_file_name, s);
	strcat(_file_name, ".json");
	printf("NEW STRATEGY: %s\n", _case_name);
	_cpus =  countCPUs();
	readStrategyTable();
}



Strategy::~Strategy()
{
	delete _case_name;
	delete my_classes;
	_cpus =  countCPUs();
}

char* Strategy::get_name()
{
	return _case_name;
}

cList* Strategy::get_list()
{
	return my_classes;
}

rightsList* Strategy::get_rights()
{
	return rights;
}

/** check whether task1 is allowed to start task2 **/
int Strategy::may_start(const char* task1, const char* task2)
{
	rightsList* r_list = rights;

	while (r_list != NULL)
	{
		if (strcmp(r_list->get()->name, task1) == 0)
		{
			if (r_list->get()->all_rights)
			{
				return 1;
			} else {
				textList* s_list = r_list->get()->start;
				while (s_list != NULL)
				{
					if (strcmp(r_list->get()->name, task2) == 0)
					{
						return 1;
					}
					s_list = s_list->next();
				}
				return 0;
			}
		}
		r_list = r_list->next();
	}
	return 0;
}


void Strategy::printRights()
{
	rightsList* r_list = rights;

	printf("RIGHTS:\n");
	while (r_list != NULL)
	{
		textList* s_list = r_list->get()->start;
		printf("	%s", r_list->get()->name);
		if (r_list->get()->all_rights)
		{
			printf(" ALLRIGHTS\n");
		} else {
			printf("\n");
		}
		while (s_list != NULL)
		{
			printf("		=>%s\n", s_list->get());
			s_list = s_list->next();
		}
		r_list = r_list->next();

	}
}



void Strategy::printClass(unsigned i, unsigned j)
{


	task_class* c = my_classes->get(i);
	printf("NAME:%s\n", c->name);
	printf("CBIND:%lu\n", c->cpuBinding);
	printf("SBIND:%lu\n", c->schedulerBind);
	printf("SCHED:%d\n", c->scheduler);
	printf("CPU:%lu\n", c->cpu);
	printf("SIZE:%lu\n", c->size);

	task* ta = c->my_tasks->get(j);
	printf("\tPROGNAME:%s\n", ta->name);
	printf("\tSCHEDULER:%d\n", ta->type);
	printf("\tPRIOORDEADLINE:%lu\n", ta->prio_or_deadline);



}

unsigned Strategy::countCPUs()
{
	l4_sched_cpu_set_t cs = l4_sched_cpu_set(0, 0, 1);
	l4_umword_t cpu_nrs;

	if (l4_error(l4_scheduler_info(l4re_env()->scheduler, &cpu_nrs, &cs)) < 0)
		return 1;

	_cpu_map = cs.map;

	return cpu_nrs;
}


task_class* Strategy::contains_class(char* name)
{
	cList* c_list = my_classes;

	while (c_list != NULL)
	{
		if (strcmp(name, c_list->get()->name) == 0)
		{
			return c_list->get();
		}
		c_list = c_list->next();
	}
	return NULL;
}

void Strategy::readStrategyTable()
{
	char buf[MAX_FILE_SIZE];

	FILE *fp;

	size_t r;


	printf("READING STRATEGY\n");

	const char *fname = _file_name;

	fp = fopen(fname, "r");
	if (!fp)
	{
		printf("XML ERROR\n"); //FILE NOT PART OF FILESYSTEM
		perror(fname);
		return ;
	}

	while ((r = fread(buf, 1, sizeof(buf), fp)))
	{
		buf[r - 1] = 0x00;
	}

	fclose(fp);
	//printf("%s\n", buf);

	jsmn_parser p;
	jsmntok_t t[MAX_JSON_TOKEN_SIZE];//expeect no more than those tokens

	int i = 1;
	int res;

	jsmn_init(&p);
	res = jsmn_parse(&p, buf, strlen(buf), t, sizeof(t) / sizeof(t[0]));
	if (res < 0) {
		printf("Failed to parse JSON: %d\n", res);
		return;
	}

	if (res < 1 || t[0].type != JSMN_OBJECT) {
		printf("Object expected\n");
		return;
	}

	//char const* tup[4] = {"JSMN_PRIMITIVE", "JSMN_OBJECT", "JSMN_ARRAY", "JSMN_STRING"};

	/** check wehther map field is set **/
	if (jsoneq(buf, &t[i], "classes") != 0)
	{
		if (jsoneq(buf, &t[i], "map") != 0)
		{
			return;
		}
		else
		{
			jsonreadint(buf, t, &i, "map", &_map);
			if (jsoneq(buf, &t[i], "classes") != 0)
			{
				return;
			}
		}
	}

	i++;
	if (t[i].type != JSMN_ARRAY)
		return;
	jsmntok_t *cs_tkn = &t[i];
	i++;
	int j, h;
	int cs_nr = 0;
	int t_nr = 0;
	/** Go through all classes and get their fields**/
	for (j = 0; j < cs_tkn->size; j++) {

		/** add new class to strategies list **/
		task_class* c = new task_class();
		c->my_tasks  = NULL;
		if (my_classes == NULL)
			my_classes = new cList(c);
		else
		{
			my_classes->add(c);
		}

		if (t[i].type != JSMN_OBJECT)
			return;
		i++;
		jsonreadstring(buf, t, &i, "name", &(c->name));
		if (jsoneq(buf, &t[i], "map") == 0)
		{
			jsonreadint(buf, t, &i, "map", &(c->map));;
		} else {
			c->map = 1;
		}
		jsonreadint(buf, t, &i, "cpuBinding", &(c->cpuBinding));
		jsonreadint(buf, t, &i, "schedulerBind", &(c->schedulerBind));
		jsonreadscheduler(buf, t, &i, "scheduler", &(c->scheduler));
		jsonreadint(buf, t, &i, "cpu", &(c->cpu));
		jsonreadint(buf, t, &i, "priority", &(c->prio));
		if (jsoneq(buf, &t[i], "tasks") != 0)
		{
			printf("\"%s\" != \"%s\"\n", jsongetstring(buf, t + i), "tasks");
			return;
		}
		i++;
		if (t[i].type != JSMN_ARRAY)
		{
			printf("NO ARRAY\n");
			return;
		}
		jsmntok_t *t_tkn = &t[i];
		i++;
		c->size = t_tkn->size;

		/** go through all tasks of class and set their fields**/
		for (h = 0; h < t_tkn->size; h++) {

			if (t[i].type != JSMN_OBJECT)
			{
				printf("JSON ERROR: NO TASKS\n");
				return;
			}

			/** create new task object **/
			task* ta = new task();
			if (c->my_tasks == NULL)
				c->my_tasks = new tList(ta);
			else
				c->my_tasks->add(ta);
			i++;
			jsonreadstring(buf, t, &i, "name", &(ta->name));

			if (jsoneq(buf, &t[i], "alias") == 0)
			{
				jsonreadstring(buf, t, &i, "alias", &(ta->alias));;
			} else {
				ta->alias = (char*) malloc(sizeof(char));
				ta->alias[0] = 0x00;
			}
			jsonreadscheduler(buf, t, &i, "scheduler", &(ta->type));
			jsonreadint(buf, t, &i, "prio_end", &(ta->prio_or_deadline));
			t_nr++;
		}
		cs_nr++;
	}

	/**check whether rights list is set**/
	if (jsoneq(buf, &t[i], "tasks") == 0)
	{
		i++;
		if (t[i].type != JSMN_ARRAY)
			return;
		jsmntok_t *cs_tkn = &t[i];
		i++;


		/** for each task-rights object: create object and get its fields **/
		for (j = 0; j < cs_tkn->size; j++) {
			task_rights* tr =  new task_rights();
			if (rights == NULL)
				rights = new rightsList(tr);
			else
			{
				rights->add(tr);
			}
			if (t[i].type != JSMN_OBJECT)
				return;
			i++;
			tr->start = NULL;

			jsonreadstring(buf, t, &i, "name", &(tr->name));
			jsonreadint(buf, t, &i, "all_rights", &(tr->all_rights));

			/** get the list of tasks which may be started **/
			if (jsoneq(buf, &t[i], "start") == 0)
			{
				i++;
				if (t[i].type != JSMN_ARRAY)
					return;
				jsmntok_t *start_tkn = &t[i];
				i++;
				for (j = 0; j < start_tkn->size; j++) {
					if (t[i].type != JSMN_OBJECT)
						return;
					i++;
					char *name = NULL;
					jsonreadstring(buf, t, &i, "task", &(name));
					if (tr->start == NULL)
						tr->start  = new textList(name);
					else
					{
						tr->start->add(name);
					}
				}
			}
		}
	}

#if DEBUG
	unsigned k, l;
	printf("CLASSES:%d\n", my_classes->size());

	for (k = 0; k < my_classes->size(); ++k)
	{
		for (l = 0; l < my_classes->get(k)->my_tasks->size(); ++l)
		{
			printClass(k, l);
		}
	}
#endif

}
int Strategy::map()
{
	return _map;
}

l4_sched_param_t Strategy::get_scheduler(const char *name, l4_sched_param_t s)
{

	unsigned CPUs = _cpus;
	l4_sched_param_t sp;
	/** to through class list **/
	cList* c_list = my_classes;
#if DEBUG
	printf("\x1b[31mFound %d CPUs\x1b[0m\n", CPUs);
#endif
	while (c_list != NULL) //find class
	{
		task_class* c = c_list->get();
		tList* t_list = c->my_tasks;
		/** go through task list if class**/
		while (t_list != NULL)
		{

			task* ta = t_list->get();
			if (strcmp(name, ta->name) == 0 && s.prio <= ta->prio_or_deadline) //find programm in class
			{
				if (!c->map)
					return s;

#if DEBUG
				printf("\x1b[31m%s...\x1b[0m\n", ta->name);
#endif
				if (c->schedulerBind)//choose the scheduler, maybe bound to class
				{
					sp = l4_sched_param_by_type(c->scheduler, c->prio, 0);
#if DEBUG
					printf("\x1b[31mBOND SCHEDULER FOR CLASS %s! SCHEDULER %u\x1b[0m\n", c->name, c->scheduler);
#endif
				}
				else
				{
					sp = l4_sched_param_by_type(ta->type, c->prio, 0);
				}

				if (c->cpuBinding)//choose the cpu, maybe bound to class
				{
					sp.affinity = l4_sched_cpu_set(c->cpu % CPUs, 0, 1);
#if DEBUG
					printf("\x1b[31mBOND CPU FOR CLASS %s! CPU %lu\x1b[0m\n", c->name, c->cpu % CPUs);
#endif
				}
				else
				{
					sp.affinity = l4_sched_cpu_set(0, 0, 1); //CPU is 0 if not bond, implement a better option in future work
				}
#if DEBUG
				printClass(my_classes->pos(c), c->my_tasks->pos(ta));
#endif
#if RUNSTATE
				printf("RUN %s\n", ta->alias);
#endif
				return sp;
			}
			t_list = t_list->next();
		}
		c_list = c_list->next();
	}
#if DEBUG
	printf("libac unknown programm\n");
#endif
	//mh.. I dont know it, musst be unimportant. Better put it in FP-scheduler on low priority
	return l4_sched_param_by_type(Fixed_prio, 1, 0);
}