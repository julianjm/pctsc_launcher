#include <gtk/gtk.h>
#include "xmlparser.h"

static GtkWidget *window;

extern char _binary_ui_gtkbuilder_xml_start;
extern char _binary_ui_gtkbuilder_xml_end;

gint on_mainwindow_delete_event (GtkWidget *widget, GdkEvent *event, gpointer data) {
	gtk_main_quit();
	return FALSE;
}

void child_exited(GPid pid, gint status, gpointer user_data) {
	ApplicationInfo *ai = (ApplicationInfo*) user_data;
	g_spawn_close_pid(pid);

	if (ai->shutdownrestart)
		gtk_main_quit();
	else
		gtk_widget_show(window);
}


void button_clicked(GtkWidget *widget, gpointer data) {
	ApplicationInfo *ai=(ApplicationInfo*) data;

	gint argc;
	gchar **argv;
	GError *error=NULL;

	if (!g_shell_parse_argv(ai->cmd, &argc, &argv, &error)) {
		printf("Error parsing command line: %s\n",ai->cmd);
		printf("Error: %s\n", error->message);
		gtk_main_quit();
		return;
	}

	GPid pid;

	if (!g_spawn_async(
			NULL,
			argv,
			NULL,
			G_SPAWN_DO_NOT_REAP_CHILD,
			NULL,
			NULL,
			&pid,
			&error)) {
		printf("Error spawning process: %s\n", error->message);
		g_strfreev(argv);
		return;
	}

	g_strfreev(argv);

	g_child_watch_add(pid, child_exited, ai);

	gtk_widget_hide(window);
}

void gtkmainmenu(int *argc, char **argv[], GHashTable *settings, GSList *applications) {
	gtk_init(argc, argv);

	GtkBuilder *builder;

	//Create the GTK Builder with the embeded xml
	builder = gtk_builder_new();
	gtk_builder_add_from_string(builder, 
		&_binary_ui_gtkbuilder_xml_start,
		(int)(&_binary_ui_gtkbuilder_xml_end-&_binary_ui_gtkbuilder_xml_start)+1,
		NULL );


	window = GTK_WIDGET(gtk_builder_get_object (builder, "mainwindow"));

	//Set menu logo
	gchar *logo_filename;
	if( (logo_filename=g_hash_table_lookup(settings,"logo_img")) ) {
		GtkImage *image_logo= GTK_IMAGE(gtk_builder_get_object(builder,"image_logo"));
		gtk_image_set_from_file( image_logo, logo_filename);
	}

	//Set Menu title
	gchar *title;
	if ( (title = g_hash_table_lookup(settings,"title")) ) {
		GtkLabel *label_title= GTK_LABEL(gtk_builder_get_object(builder,"label_title"));
		gtk_label_set_text(label_title, title);
	}

	//Create the application buttons
	GtkWidget *vbox_buttons=GTK_WIDGET(gtk_builder_get_object(builder,"vbox_buttons"));
	GSList *apps=applications;
	while(apps) {
		GtkWidget *button;
		ApplicationInfo *ai=(ApplicationInfo*)apps->data;
		button = gtk_button_new_with_label(ai->title);
		gtk_signal_connect(GTK_OBJECT(button), "clicked", G_CALLBACK(button_clicked), ai);
		gtk_box_pack_start(GTK_BOX(vbox_buttons), button, TRUE, TRUE, TRUE);
		gtk_widget_show(button);

		//If they defined an icon, assign it to the button
		if (ai->icon) {
			gsize icondata_size;
			guchar *icondata = g_base64_decode(ai->icon, &icondata_size);

			gchar *tmpname;
			gint fh = g_file_open_tmp(NULL, &tmpname, NULL);
			write(fh, icondata, icondata_size);
			close(fh);

			GtkWidget *gtkimg = gtk_image_new_from_file(tmpname);
			gtk_button_set_image(GTK_BUTTON(button), gtkimg);
		}

		apps=g_slist_next(apps);
	}

	gchar *cmd;
	if ((cmd = g_hash_table_lookup(settings,"cmd_restart")) ) {
		ApplicationInfo *ai=g_malloc0(sizeof(ApplicationInfo));
		ai->cmd=cmd;
		ai->shutdownrestart=TRUE;
		GtkWidget *button;
		button = GTK_WIDGET(gtk_builder_get_object(builder, "button_restart"));
		gtk_signal_connect(GTK_OBJECT(button), "clicked", G_CALLBACK(button_clicked),ai);
	}
	if ((cmd = g_hash_table_lookup(settings,"cmd_poweroff")) ) {
		ApplicationInfo *ai=g_malloc0(sizeof(ApplicationInfo));
		ai->cmd=cmd;
		ai->shutdownrestart=TRUE;
		GtkWidget *button;
		button = GTK_WIDGET(gtk_builder_get_object(builder, "button_poweroff"));
		gtk_signal_connect(GTK_OBJECT(button), "clicked", G_CALLBACK(button_clicked),ai);
	}

	gtk_builder_connect_signals (builder, NULL);

	g_object_unref(G_OBJECT(builder));
	gtk_widget_show(window);
	gtk_main();

	return;
}
