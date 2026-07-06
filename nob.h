/*************************************************************************
	> File Name: nob.h
	> Author: mlxh
	> Mail: mlxh_gto@163.com 
	> Created Time: Fri May 22 09:50:37 2026
 ************************************************************************/

#ifndef NOB_H
#define NOB_H

#ifndef MAX_ARRAY_NUM
#define MAX_ARRAY_NUM 1024
#endif

#ifndef MIN_ARRAY_NUM
#define MIN_ARRAY_NUM 256
#endif

#ifndef NOB_LOG
#define NOB_LOG
#include <stdio.h>
#define NOB_INFO(fmt, ...)  fprintf(stdout,"[INFO] func[%s] line[%d]: " fmt "\n",__func__,__LINE__,##__VA_ARGS__)
#define NOB_ERROR(fmt, ...) fprintf(stderr,"[ERROR] func[%s] line[%d]: " fmt "\n",__func__,__LINE__,##__VA_ARGS__)
#endif

#if defined(NOB_REALLOC) && !defined(NOB_FREE) || !defined(NOB_REALLOC) && defined(NOB_FREE) 
#error "You must define both NOB_REALLOC and NOB_FREE, or neither."
#endif

#if !defined(NOB_REALLOC) && !defined(NOB_FREE)
#include <stdlib.h>
#define NOB_REALLOC(o,p,s) realloc(p,s)
#define NOB_FREE(p) free(p)
#endif


typedef struct {
	const char ** data;
	size_t count;
	size_t capacity;
} Varray;

#define varray_append(va, s)																			\
	do {																								\
		if((va)->count >= (va)->capacity && (va)->capacity < MAX_ARRAY_NUM){								\
			size_t capacity_tmp = (va)->capacity == 0 ? MIN_ARRAY_NUM : 2 * (va)->capacity;				\
			const char ** data_tmp = NOB_REALLOC(NULL, (va)->data, capacity_tmp * sizeof(char *));		\
			if(data_tmp){																				\
				(va)->data = data_tmp;																	\
				(va)->capacity = capacity_tmp;															\
			} else {																					\
				NOB_ERROR("Fail to realloc.");															\
				break;																					\
			}																							\
		}																								\
		if((va)->count < (va)->capacity){																	\
			(va)->data[(va)->count] = (s);																\
			(va)->count++;																				\
		} else {																						\
				NOB_ERROR("Array is full or unallocation.");											\
		}																								\
	} while(0)

#define varray_append_many(va, ss, size2append)															\
	do{																									\
		for(int i = 0; i < (size2append); ++i) {														\
			size_t count_tmp = (va)->count;																\
			varray_append((va),(ss)[i]);																\
			if(count_tmp == (va)->count){																\
				NOB_ERROR("Fail to append elements.");													\
				break;																					\
			}																							\
		}																								\
	} while(0)


#define nob_cmd_append(cmd, ...) \
	varray_append_many(cmd,\
			((const char *[]){__VA_ARGS__}),\
			(sizeof((const char *[]){__VA_ARGS__})/sizeof(const char *)))

int nob_cmd_run(Varray *cmd);
void nob_cmd_dump(Varray cmd);

#define NOB_BUILD_PROJECT(cmd) \
	do{\
		NOB_INFO("Building......");\
		if(nob_cmd_run(cmd)<0){\
			NOB_ERROR("Buid fail.");\
			exit(EXIT_FAILURE);\
		}\
	}while(0)

#define nob_cmd_free(cmd)	\
	do{\
		if((cmd)->data) NOB_FREE((cmd)->data);\
		(cmd)->data = NULL;\
		(cmd)->count = 0;\
		(cmd)->capacity = 0;\
	}while(0)

#endif // NOB_H

#if defined(NOB_IMPLEMENTATION) && !defined(NOB_IMPLEMENTATION_DOWN)
#define NOB_IMPLEMENTATION_DOWN
#include <sys/wait.h>
#include <unistd.h>

int nob_cmd_run(Varray *cmd)
{
	nob_cmd_append(cmd,NULL);

	pid_t cpid = fork();
	if(cpid == -1){
		NOB_ERROR("Fail to fork.");
		exit(EXIT_FAILURE);
	}

	if(cpid == 0){
		if(execvp(cmd->data[0],(char * const *)cmd->data)<0){
			NOB_ERROR("Fail to execvp.");
			exit(EXIT_FAILURE);
		}
	}else{
		int status;
		waitpid(cpid, &status, 0);
		return WIFEXITED(status) && WEXITSTATUS(status) == 0;
	}
	return 0;
}

void nob_cmd_dump(Varray cmd)
{
	for(int i = 0;i<cmd.count;++i){
		NOB_INFO("cmd[%d]:%s",i,cmd.data[i]);
	}
}

#endif // NOB_IMPLEMENTATION
