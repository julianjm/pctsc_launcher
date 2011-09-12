#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
//#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>

#include "xmlparser.h"
#include "gtkmainmenu.h"

int file_exists(char *filename) {
	struct stat buf;
	return stat(filename, &buf) == 0;
}

void shellexport(GHashTable *settings, GSList *applications) {
	GList *keys = g_hash_table_get_keys(settings);
	while(keys) {
		printf("setting_%s=\"%s\"\n", (char*)keys->data, (char*)g_hash_table_lookup(settings, keys->data));
		keys=g_list_next(keys);
	}

	GSList *apps = applications;
	int i=1;
	while(apps) {
		ApplicationInfo *ai = (ApplicationInfo*)apps->data;
		printf("application%d_title=\"%s\"\n", i, ai->title);
		printf("application%d_cmd=\"%s\"\n", i, ai->cmd);
		apps=g_slist_next(apps);
		i++;
	}
}

int main(int argc, char *argv[]) {

	if (argc!=3) {
		printf("Usage: %s <gtkmenu|export> <configfile>\n", argv[0]);
		exit(1);
	}

	char *file = argv[2];
	if (!file_exists(file)) {
		printf("File doesn't exist: %s\n", file);
		exit(1);
	}

	GHashTable *settings;
	GSList *applications;

	parse_config_xml(file, &settings, &applications);


	if (strcmp(argv[1], "gtkmenu")==0) {
		gtkmainmenu(argc, argv, settings, applications);
	} else if (strcmp(argv[1], "shellexport")==0) {
		shellexport(settings, applications);
	}

	return 0;
}
