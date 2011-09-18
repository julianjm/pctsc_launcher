#include <stdio.h>
#include <glib.h>
#include <string.h>

#include "xmlparser.h"


typedef enum
{
	STATE_START=0,
	STATE_ROOT,
	STATE_SETTINGS,
	STATE_SETTING_VALUE,
	STATE_APPLICATIONS,
	STATE_APPLICATION,
	STATE_APPLICATION_TITLE,
	STATE_APPLICATION_CMD,
	STATE_APPLICATION_ICON
} ParseState;

typedef struct
{
	GSList *states;
	GHashTable *settings;
	GSList *applications;

	ApplicationInfo *tmp_app;
	gchar *tmp_text;
} ParseInfo;


////////////////////////////////////////////

void applist_free(GSList *apps) {
	GSList *ptmp = apps;
	while(ptmp) {
		ApplicationInfo *data;
		data = ptmp->data;
		if (data->title)
			g_free(data->title);
		if (data->cmd)
			g_free(data->cmd);
		if (data->icon)
			g_free(data->icon);
		g_free(data);
		ptmp=g_slist_next(ptmp);
	}
	g_slist_free(apps);
}

void settings_free(GHashTable *settings) {
	if (!settings)
		return;
	g_hash_table_destroy(settings);	
}


////////////////////////////////////////////

static void push_state(ParseInfo *pi, ParseState state) {
/*	switch((int)state) {
		case STATE_START: str="STATE_START"; break;
		case STATE_ROOT: str="STATE_ROOT"; break;
		case STATE_SETTINGS: str="STATE_SETTINGS"; break;
		case STATE_SETTING_VALUE: str="STATE_SETTING_VALUE"; break;
		case STATE_APPLICATIONS: str="STATE_APPLICATIONS"; break;
		case STATE_APPLICATION: str="STATE_APPLICATION"; break;
		case STATE_APPLICATION_TITLE: str="STATE_APPLICATION_TITLE"; break;
		case STATE_APPLICATION_CMD: str="STATE_APPLICATION_CMD"; break;
		default: str="UNKNOWN!";
	}
*/
	pi->states = g_slist_prepend(pi->states, GINT_TO_POINTER(state));
}

static void pop_state(ParseInfo *pi) {
	pi->states = g_slist_remove(pi->states, pi->states->data);
}

static ParseState peek_state (ParseInfo *pi) {
	return GPOINTER_TO_INT (pi->states->data);
}

static void parse_info_init(ParseInfo *pi) {
	pi->states = g_slist_prepend(NULL, GINT_TO_POINTER(STATE_START));

	pi->settings = g_hash_table_new_full(g_str_hash, g_str_equal,g_free,g_free);
	pi->applications = NULL;
}

static void parse_info_free(ParseInfo *pi) {
	while(pi->states) {
		g_slist_free(pi->states);
		pi->states=NULL;
//		pi->states = g_slist_remove(pi->states,pi->states->data);
	}
}

///////////////////////////////////////////




void start_element (GMarkupParseContext *context,
	const gchar	*element_name,
	const gchar	**attribute_names,
	const gchar	**attribute_values,
	gpointer	user_data,
	GError		**error) {

	ParseInfo *pi = (ParseInfo*) user_data;	

	switch(peek_state(pi)) {
		case STATE_START:
			if (strcmp(element_name,"pctsc")!=0) {
				/*set_error(error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
					"Outermost element must be <pctsc>, not <%s>", element_name);*/
				g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE, "Error1");
			}
			push_state(pi,STATE_ROOT);
			break;

		case STATE_ROOT:
			if (strcmp(element_name,"settings")==0)
				push_state(pi, STATE_SETTINGS);
			else if (strcmp(element_name,"applications")==0)
				push_state(pi, STATE_APPLICATIONS);
			else 
				/*set_error(error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
					"Only <applications> or <settings> element allowed, not <%s>", element_name);*/
				g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE, "Error2");
			break;
		case STATE_SETTINGS:
			//element_name is the key
			pi->tmp_text=g_strdup(element_name);
			push_state(pi, STATE_SETTING_VALUE);
			break;
		case STATE_APPLICATIONS:
			if (strcmp(element_name,"application")==0) {
				pi->tmp_app=g_malloc0(sizeof(ApplicationInfo));
				push_state(pi, STATE_APPLICATION);
			} else {
				g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE, "<%s> Not allowed here, only <application>", element_name);
			}
			break;
		case STATE_APPLICATION:
			if (strcmp(element_name,"title")==0) {
				push_state(pi, STATE_APPLICATION_TITLE);
			} else if (strcmp(element_name, "cmd")==0) {
				push_state(pi, STATE_APPLICATION_CMD);
			} else if (strcmp(element_name, "icon")==0) {
				push_state(pi, STATE_APPLICATION_ICON);
			} else {
				g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE, "<%s> Not allowed here, only <title> or <cmd>", element_name);
			}
			break;
		default:break;
	}
}

void text(GMarkupParseContext *context,
	const gchar	*text,
	const gsize	text_len,
	gpointer	user_data,
	GError	**error) {

	ParseInfo *pi = (ParseInfo*) user_data;	
	
	gchar *value;

	switch(peek_state(pi)) {
		case STATE_SETTING_VALUE:
			value = g_strndup(text,text_len);
			g_hash_table_insert(pi->settings, pi->tmp_text, value);
			pi->tmp_text=NULL;
			break;
		case STATE_APPLICATION_TITLE:
		case STATE_APPLICATION_CMD:
		case STATE_APPLICATION_ICON:
			pi->tmp_text = g_strndup(text, text_len);
			break;
		default:break;
	}
			
}

void end_element (GMarkupParseContext *context,
	const gchar	*element_name,
	gpointer	user_data,
	GError		**error) {
	
	ParseInfo *pi = (ParseInfo*) user_data;	


	switch(peek_state(pi)) {
		case STATE_SETTINGS:
			pop_state(pi);
			break;
		case STATE_SETTING_VALUE:
			pop_state(pi);
			break;
		case STATE_APPLICATION_TITLE:
			pi->tmp_app->title = pi->tmp_text;
			pi->tmp_text=NULL;
			pop_state(pi);
			break;
		case STATE_APPLICATION_CMD:
			pi->tmp_app->cmd = pi->tmp_text;
			pi->tmp_text=NULL;
			pop_state(pi);
			break;
		case STATE_APPLICATION_ICON:
			pi->tmp_app->icon = pi->tmp_text;
			pi->tmp_text=NULL;
			pop_state(pi);
			break;
		case STATE_APPLICATION:
			pi->applications = g_slist_append(pi->applications, pi->tmp_app);
			pi->tmp_app=NULL;
			pop_state(pi);
			break;
		default:break;

	}
}


static GMarkupParser parser = {
	start_element,
	end_element,
	text,
	NULL,
	NULL
};




int parse_config_xml(char *filename, GHashTable **settings, GSList **applications) {
	char *text;
	gsize length;

	ParseInfo pi;
	parse_info_init(&pi);

	GMarkupParseContext *context = g_markup_parse_context_new (
		&parser,
		0,
		&pi,
		NULL);

	if (g_file_get_contents(filename, &text, &length, NULL) == FALSE) {
		parse_info_free(&pi);
		g_markup_parse_context_free(context);
		return 1;
	}




	if (g_markup_parse_context_parse (context, text, length, NULL) == FALSE) {
		parse_info_free(&pi);
		g_markup_parse_context_free(context);
		g_free(text);
		return 1;
	}



	*applications = pi.applications;
	*settings = pi.settings;

	pi.applications=NULL;
	pi.settings=NULL;

	parse_info_free(&pi);
	g_free(text);
	g_markup_parse_context_free(context);

	return 0;
}


