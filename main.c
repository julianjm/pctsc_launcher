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
	GList *tmpkey = keys;
	while(tmpkey) {
		printf("setting_%s=\"%s\"\n", (char*)tmpkey->data, (char*)g_hash_table_lookup(settings, tmpkey->data));
		tmpkey=g_list_next(tmpkey);
	}
	g_list_free(keys);

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
		printf("Usage: %s <gtkmainmenu|shellexport> <configfile>\n", argv[0]);
		exit(1);
	}

	char *file = argv[2];
	if (!file_exists(file)) {
		printf("File doesn't exist: %s\n", file);
		exit(1);
	}

	GHashTable *settings;
	GSList *applications;

	if (parse_config_xml(file, &settings, &applications)) {
		printf("Parsing failed\n");
		exit(1);
	}


	if (strcmp(argv[1], "gtkmainmenu")==0) {
		gtkmainmenu(&argc, &argv, settings, applications);
	} else if (strcmp(argv[1], "shellexport")==0) {
		shellexport(settings, applications);
	}

	applist_free(applications);
	settings_free(settings);

	applications=NULL;
	settings=NULL;
	

	return 0;
}

