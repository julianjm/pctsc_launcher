
typedef struct
{
	char *title;
	char *cmd;
} ApplicationInfo;

int parse_config_xml(char *filename, GHashTable **settings, GSList **applications);
