#pragma once

#include <l4/sys/kernel_object.h>
#include <l4/sys/ipc.h>
#include <l4/sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define ADMCONTROL_MAX_INFO 9
#define ADMCONTROL_CAP l4re_env()->scheduler
#define L4_PROTO_ADMCONTROL -42L 
//Protocol for messages to the admission control
enum admcontrol_ops
{
	ADMCONTROL_NEW_CAP_OP      			= 0UL, /**gettin a new cap**/
	ADMCONTROL_ACTIVE_STRATEGY_OP   	= 1UL, /**gettin a new cap**/
	ADMCONTROL_SWITCH_STRATEGY_OP   	= 2UL, /**gettin a new cap**/
	ADMCONTROL_GET_STRATEGYLIST_OP		= 3UL, /**gettin a new cap**/
	ADMCONTROL_GET_CLASSLIST_OP			= 4UL, /**gettin a new cap**/
	ADMCONTROL_GET_TASKLIST_OP			= 5UL, /**gettin a new cap**/
	ADMCONTROL_ADD_STRATEGY_OP      	= 6UL, /**gettin a new cap**/
	ADMCONTROL_ADD_CLASS_OP      		= 7UL, /**gettin a new cap**/
	ADMCONTROL_ADD_TASK_OP      		= 8UL, /**gettin a new cap**/
};


static char const *const __admcontrol_information[ADMCONTROL_MAX_INFO] = {
	[ADMCONTROL_NEW_CAP_OP]				= "ADMISSION CONTROL: new capability request",

	[ADMCONTROL_ACTIVE_STRATEGY_OP]		= "ADMISSION CONTROL: get active strategy",

	[ADMCONTROL_SWITCH_STRATEGY_OP]		= "ADMISSION CONTROL: swtich strategy",

	[ADMCONTROL_GET_STRATEGYLIST_OP]	= "ADMISSION CONTROL: get all strategies",

	[ADMCONTROL_GET_CLASSLIST_OP]		= "ADMISSION CONTROL: get classes of strategy",

	[ADMCONTROL_GET_TASKLIST_OP]		= "ADMISSION CONTROL: get task of class",

	[ADMCONTROL_ADD_STRATEGY_OP]		= "ADMISSION CONTROL: add strategy",

	[ADMCONTROL_ADD_CLASS_OP]			= "ADMISSION CONTROL: add class to strategy",

	[ADMCONTROL_ADD_TASK_OP]			= "ADMISSION CONTROL: add task to class",
};

/**
 * \brief Admission Control class set.
 * \ingroup admcontrol_api
 * \author Sebastian Bachmaier
 */
typedef struct _admcontrol_class
{
	char *name;					//name of the admission control class
	l4_umword_t parent_strategy;	 	//id of the parent strategy
	l4_umword_t cpu_binding; 			//same cpu for all threads in class
	l4_umword_t scheduler_binding; 		//same scheduler for all threads in class
	l4_umword_t priority; 				//the target priority for threads in this class
	l4_sched_param_type_t scheduler; 	//which scheduler; deadline or fixed_prio (see l4sys)
	l4_umword_t cpu; 					//which cpu 
	l4_umword_t map; 					//do any mapping at all (0 for no mapping)
} admcontrol_class;


/**
 * \brief Admission Control task set for a class
 * \ingroup admcontrol_api
 * \author Sebastian Bachmaier
 */
typedef struct _admcontrol_task
{
	char* name;							//name of the task in the class
	l4_umword_t parent_strategy;		//id of the parent strategy
	l4_umword_t parent_class;			//id of the parent class
	l4_sched_param_type_t scheduler;	//which scheduler (like admcontrol_class), just used when scheduler_bind is off
	l4_umword_t end_prio;				//until which priority is the task in this class
} admcontrol_task;


L4_CV char const *admcontrol_inf(l4_umword_t inf);

L4_CV char const *admcontrol_inf(l4_umword_t inf)
{
	if (inf < ADMCONTROL_MAX_INFO)
		return __admcontrol_information[inf];
	else
		return "unknown information";
}


/**
 * \brief Used for registering task in Admission Control
 * \ingroup admcontrol_api
 * \author Sebastian Bachmaier
 */
L4_INLINE l4_msgtag_t
admcontrol_get_cap_u(l4_cap_idx_t admcontrol, char const *task_name, l4_cap_idx_t cap, l4_utcb_t *utcb) L4_NOTHROW
{
	l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
	l4_umword_t len = strlen(task_name) + 1;
	v->mr[0] = ADMCONTROL_NEW_CAP_OP;
	memcpy(v->mr + 1, task_name, len);
	l4_utcb_br_u(utcb)->bdr &= ~L4_BDR_OFFSET_MASK;
	l4_utcb_br_u(utcb)->br[0] = cap | L4_RCV_ITEM_SINGLE_CAP;

	return l4_ipc_call(admcontrol, utcb, l4_msgtag(L4_PROTO_ADMCONTROL, len / sizeof(l4_umword_t) + 2, 0, 0), L4_IPC_NEVER);
}


/**
 * \brief Getting active strategy of Admission Control
 * \ingroup admcontrol_api
 * \author Sebastian Bachmaier
 */
L4_INLINE l4_msgtag_t
admcontrol_active_strategy_u(l4_cap_idx_t admcontrol, l4_umword_t* no, char** name, l4_utcb_t *utcb) L4_NOTHROW
{
	l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
	v->mr[0] = ADMCONTROL_ACTIVE_STRATEGY_OP;

	l4_msgtag_t tag = l4_ipc_call(admcontrol, utcb, l4_msgtag(L4_PROTO_ADMCONTROL, 1, 0, 0), L4_IPC_NEVER);
	l4_umword_t message_size = l4_msgtag_words(tag);
	l4_umword_t buffer[message_size];//Not nice, but doesnt work differently because malloc kills message registers
	memcpy(buffer, v->mr, message_size * sizeof(l4_umword_t));
	*no = buffer[0];
	*name = (char*) malloc(sizeof(char) * (strlen((char*)(v->mr + 1)) + 1));
	strcpy(*name, (char*)(buffer + 1));
	return tag;
}

/**
 * \brief Switch strategy to strategy with id no
 * \ingroup admcontrol_api
 * \author Sebastian Bachmaier
 */
L4_INLINE l4_msgtag_t
admcontrol_switch_strategy_u(l4_cap_idx_t admcontrol, l4_umword_t no, l4_utcb_t *utcb) L4_NOTHROW
{
	l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
	v->mr[0] = ADMCONTROL_SWITCH_STRATEGY_OP;
	v->mr[1] = no;

	l4_msgtag_t tag = l4_ipc_call(admcontrol, utcb, l4_msgtag(L4_PROTO_ADMCONTROL, 2, 0, 0), L4_IPC_NEVER);
	return tag;
}

/**
 * \brief Helper method for sending strings
 * \ingroup admcontrol_api
 * \author Sebastian Bachmaier
 */
L4_INLINE void
admcontrol_hndl_list(l4_msgtag_t tag, l4_msg_regs_t *v, l4_umword_t* size, l4_umword_t** pos, char*** names) L4_NOTHROW
{
	l4_umword_t message_size = l4_msgtag_words(tag);
	l4_umword_t buffer[message_size];//Not nice, but doesnt work differently because malloc kills message registers
	memcpy(buffer, v->mr, message_size * sizeof(l4_umword_t));

	*size = buffer[0];

	unsigned i;
	*pos 	= (l4_umword_t*) 	malloc(sizeof(l4_umword_t) 	*	*size);
	*names 	= (char**) 			malloc(sizeof(char*)		*	*size);
	char* ptr = (char*) (buffer + 1);


	for (i = 0; i < *size; ++i)
	{
		(*pos)[i] = *((l4_umword_t*) ptr);
		ptr += sizeof(l4_umword_t);
		unsigned len = strlen((char*)ptr);
		(*names)[i] = (char*) malloc(sizeof(char) * (len + 1));
		strcpy((*names)[i], (char*)ptr);
		ptr += (sizeof(char) * (len + 1) + sizeof(l4_umword_t)*((len + 1) % sizeof(l4_umword_t)!=0) - (len + 1) % sizeof(l4_umword_t));
	}
}



/**
 * \brief Get alls strategys stored in admission control
 * \ingroup admcontrol_api
 * \author Sebastian Bachmaier
 */
L4_INLINE l4_msgtag_t
admcontrol_get_strategylist_u(l4_cap_idx_t admcontrol, l4_umword_t* size, l4_umword_t** pos, char*** names, l4_utcb_t *utcb) L4_NOTHROW
{
	l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
	v->mr[0] = ADMCONTROL_GET_STRATEGYLIST_OP;

	l4_msgtag_t tag = l4_ipc_call(admcontrol, utcb, l4_msgtag(L4_PROTO_ADMCONTROL, 1, 0, 0), L4_IPC_NEVER);

	admcontrol_hndl_list(tag, v,  size, pos, names);
	return tag;
}


/**
 * \brief Get all classes of strategy
 * \ingroup admcontrol_api
 * \author Sebastian Bachmaier
 */
L4_INLINE l4_msgtag_t
admcontrol_get_classlist_u(l4_cap_idx_t admcontrol, l4_umword_t sNo, l4_umword_t* size, l4_umword_t** pos, char*** names, l4_utcb_t *utcb) L4_NOTHROW
{
	l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
	v->mr[0] = ADMCONTROL_GET_CLASSLIST_OP;
	v->mr[1] = sNo;
	l4_msgtag_t tag = l4_ipc_call(admcontrol, utcb, l4_msgtag(L4_PROTO_ADMCONTROL, 2, 0, 0), L4_IPC_NEVER);

	admcontrol_hndl_list(tag, v,  size, pos, names);
	return tag;
}


/**
 * \brief Get all tasks of class
 * \ingroup admcontrol_api
 * \author Sebastian Bachmaier
 */
L4_INLINE l4_msgtag_t
admcontrol_get_tasklist_u(l4_cap_idx_t admcontrol, l4_umword_t sNo, l4_umword_t cNo, l4_umword_t* size, l4_umword_t** pos, char*** names, l4_utcb_t *utcb) L4_NOTHROW
{
	l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
	v->mr[0] = ADMCONTROL_GET_TASKLIST_OP;
	v->mr[1] = sNo;
	v->mr[2] = cNo;

	l4_msgtag_t tag = l4_ipc_call(admcontrol, utcb, l4_msgtag(L4_PROTO_ADMCONTROL, 3, 0, 0), L4_IPC_NEVER);

	admcontrol_hndl_list(tag, v,  size, pos, names);
	return tag;
}


/**
 * \brief Add strategy to Admission Control
 * \ingroup admcontrol_api
 * \author Sebastian Bachmaier
 */
L4_INLINE l4_msgtag_t
admcontrol_add_strategy_u(l4_cap_idx_t admcontrol, char const *stratey_name, l4_umword_t* id, l4_utcb_t *utcb) L4_NOTHROW
{
	l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
	l4_umword_t len = strlen(stratey_name) + 1;
	v->mr[0] = ADMCONTROL_ADD_STRATEGY_OP;
	memcpy(v->mr + 1, stratey_name, len);
	l4_msgtag_t t = l4_ipc_call(admcontrol, utcb, l4_msgtag(L4_PROTO_ADMCONTROL, (len-1) / sizeof(l4_umword_t) + 2, 0, 0), L4_IPC_NEVER);
	*id = v->mr[0];
	return t;
}


/**
 * \brief Add class to strategy
 * \ingroup admcontrol_api
 * \author Sebastian Bachmaier
 */
L4_INLINE l4_msgtag_t
admcontrol_add_class_u(l4_cap_idx_t admcontrol, admcontrol_class *settings, l4_umword_t* id, l4_utcb_t *utcb) L4_NOTHROW
{
	l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
	l4_umword_t len = strlen(settings->name) + 1;
	v->mr[0] = ADMCONTROL_ADD_CLASS_OP;
	v->mr[1] = settings->parent_strategy;
	v->mr[2] = settings->cpu_binding;
	v->mr[3] = settings->scheduler_binding;
	v->mr[4] = settings->priority;
	v->mr[5] = (l4_umword_t)(settings->scheduler);
	v->mr[6] = settings->cpu;
	v->mr[7] = settings->map;
	memcpy(v->mr + 8, settings->name, len);
	l4_msgtag_t t = l4_ipc_call(admcontrol, utcb, l4_msgtag(L4_PROTO_ADMCONTROL, (len-1) / sizeof(l4_umword_t) + 2 + 7, 0, 0), L4_IPC_NEVER);
	*id = v->mr[0];
	return t;
}


/**
 * \brief Add tas to class
 * \ingroup admcontrol_api
 * \author Sebastian Bachmaier
 */
L4_INLINE l4_msgtag_t
admcontrol_add_task_u(l4_cap_idx_t admcontrol,  admcontrol_task *settings, l4_umword_t* id, l4_utcb_t *utcb) L4_NOTHROW
{
	l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
	l4_umword_t len = strlen(settings->name) + 1;
	v->mr[0] = ADMCONTROL_ADD_TASK_OP;
	v->mr[1] = settings->parent_strategy;
	v->mr[2] = settings->parent_class;
	v->mr[3] = (l4_umword_t)(settings->scheduler);
	v->mr[4] = settings->end_prio;
	memcpy(v->mr + 5, settings->name, len);
	l4_msgtag_t t = l4_ipc_call(admcontrol, utcb, l4_msgtag(L4_PROTO_ADMCONTROL, (len-1) / sizeof(l4_umword_t) + 2 + 4, 0, 0), L4_IPC_NEVER);
	*id = v->mr[0];
	return t;
}



L4_INLINE l4_msgtag_t
admcontrol_get_cap(l4_cap_idx_t admcontrol, char const *task_name, l4_cap_idx_t cap) L4_NOTHROW
{
	return admcontrol_get_cap_u(admcontrol, task_name, cap, l4_utcb());
}

L4_INLINE l4_msgtag_t
admcontrol_active_strategy(l4_cap_idx_t admcontrol, l4_umword_t* no, char** name) L4_NOTHROW
{
	return admcontrol_active_strategy_u(admcontrol, no, name, l4_utcb());
}

L4_INLINE l4_msgtag_t
admcontrol_switch_strategy(l4_cap_idx_t admcontrol, l4_umword_t no) L4_NOTHROW
{
	return admcontrol_switch_strategy_u(admcontrol, no, l4_utcb());
}

L4_INLINE l4_msgtag_t
admcontrol_get_strategylist(l4_cap_idx_t admcontrol, l4_umword_t* size, l4_umword_t** pos, char*** name) L4_NOTHROW
{
	return admcontrol_get_strategylist_u(admcontrol, size, pos, name, l4_utcb());
}

L4_INLINE l4_msgtag_t
admcontrol_get_classlist(l4_cap_idx_t admcontrol, l4_umword_t sNo, l4_umword_t* size, l4_umword_t** pos, char*** name) L4_NOTHROW
{
	return admcontrol_get_classlist_u(admcontrol, sNo, size, pos, name, l4_utcb());
}

L4_INLINE l4_msgtag_t
admcontrol_get_tasklist(l4_cap_idx_t admcontrol, l4_umword_t sNo, l4_umword_t cNo, l4_umword_t* size, l4_umword_t** pos, char*** name) L4_NOTHROW
{
	return admcontrol_get_tasklist_u(admcontrol, sNo, cNo, size, pos, name, l4_utcb());
}

L4_INLINE l4_msgtag_t
admcontrol_add_strategy(l4_cap_idx_t admcontrol, char *stratey_name, l4_umword_t* id) L4_NOTHROW
{
	return admcontrol_add_strategy_u(admcontrol, stratey_name, id, l4_utcb());
}

L4_INLINE l4_msgtag_t
admcontrol_add_class(l4_cap_idx_t admcontrol, admcontrol_class *settings, l4_umword_t* id) L4_NOTHROW
{
	return admcontrol_add_class_u(admcontrol, settings, id, l4_utcb());
}


L4_INLINE l4_msgtag_t
admcontrol_add_task(l4_cap_idx_t admcontrol, admcontrol_task *settings, l4_umword_t* id) L4_NOTHROW
{
	return admcontrol_add_task_u(admcontrol, settings, id, l4_utcb());
}

void admcontrol_print_config();

void admcontrol_print_config()
{
	char** s_names;
	l4_umword_t* s_pos;
	l4_umword_t s_size;

	unsigned i,j,h;

	admcontrol_get_strategylist(l4re_env()->scheduler, &s_size, &s_pos, &s_names);
	for (i = 0; i < s_size; ++i)
	{

		printf("STRATEGY: %s\n", s_names[i]);
		char** c_names;
		l4_umword_t* c_pos;
		l4_umword_t c_size;
		admcontrol_get_classlist(l4re_env()->scheduler, i, &c_size, &c_pos, &c_names);


		for (j = 0; j < c_size; ++j)
		{
			printf("	CLASS: %s\n", c_names[j]);
			char** t_names;
			l4_umword_t* t_pos;
			l4_umword_t t_size;
			admcontrol_get_tasklist(l4re_env()->scheduler, i, j, &t_size, &t_pos, &t_names);
			for (h = 0; h < t_size; ++h)
			{
				printf("		TASK: %s\n", t_names[h]);
			}
		}

	}
}

