/*************************************************************************
	> File Name: nob.c
	> Author: mlxh
	> Mail: mlxh_gto@163.com 
	> Created Time: Wed 24 Jun 2026 10:33:45 PM CST
 ************************************************************************/


#define NOB_IMPLEMENTATION
#include "nob.h"

#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#define INCLUDE_DIR			"./include/"
#define SRC_DIR				"./src/"

#define TD_DIR				"./3d/"
#define TD_INCLUDE_DIR		"./3d/include/"
#define TD_LIB_DIR			"./3d/lib/"

#define PLUGS_DIR			"./plugs/"
#define PLUGS_INCLUDE_DIR	"./plugs/include/"
#define PLUGS_LIB_DIR		"./plugs/lib/"

void cc(Varray * cmd)
{
	nob_cmd_append(cmd,"gcc");
	nob_cmd_append(cmd,"-Wextra","-Wall");
	nob_cmd_append(cmd,"-D_GNU_SOURCE");
	nob_cmd_append(cmd,"-ggdb");
	nob_cmd_append(cmd,"-Wno-unused-function");
}

void cc_include(Varray *cmd)
{
	nob_cmd_append(cmd,"-I"INCLUDE_DIR);
	nob_cmd_append(cmd,"-I"TD_INCLUDE_DIR);
	nob_cmd_append(cmd,"-I"PLUGS_INCLUDE_DIR);
	nob_cmd_append(cmd,"-I.");

}

void auto_append_lib_paths(Varray *cmd, const char *base_dir)
{
	DIR *dir = opendir(base_dir);
	if (!dir) {
		NOB_ERROR("无法打开目录: %s", base_dir);
		return;
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		if (entry->d_type == DT_DIR) {
			char *l_path = malloc(256);
			char *r_path = malloc(256);

			if (l_path && r_path) {
				snprintf(l_path, 256, "-L%s%s/", base_dir, entry->d_name);
				snprintf(r_path, 256, "-Wl,-rpath,%s%s/", base_dir, entry->d_name);

				nob_cmd_append(cmd, l_path);
				nob_cmd_append(cmd, r_path);
			}
		}
	}
	closedir(dir);
}

void cc_lib(Varray *cmd)
{
	auto_append_lib_paths(cmd,TD_LIB_DIR);
	auto_append_lib_paths(cmd,PLUGS_LIB_DIR);

	nob_cmd_append(cmd,"-luv");
	nob_cmd_append(cmd,"-lm","-ldl","-lpthread");
	nob_cmd_append(cmd,"-lraylib");
}

void cc_main(Varray *cmd)
{
	cc(cmd);
	cc_include(cmd);

	nob_cmd_append(cmd,"main.c");
	nob_cmd_append(cmd,"-o", "go");

	//nob_cmd_append(cmd, SRC_DIR"TODO.c");

	cc_lib(cmd);
	NOB_BUILD_PROJECT(cmd);
}

int main(void)
{
	Varray cmd = {0}; 
	cc_main(&cmd);
	nob_cmd_dump(cmd);

	nob_cmd_free(&cmd);
	return 0;
}
