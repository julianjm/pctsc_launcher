
typedef struct
{
	char *title;
	char *cmd;
} ApplicationInfo;

int parse_config_xml(char *filename, GHashTable **settings, GSList **applications);

void applist_free(GSList *apps);
void settings_free(GHashTable *settings);


