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
	g_spawn_close_pid(pid);
	gtk_main_quit();	
}

void button_clicked(GtkWidget *widget, gpointer data) {
	ApplicationInfo *ai=(ApplicationInfo*) data;

	gint argc;
	gchar **argv;
	GError *error;

	if (!g_shell_parse_argv(ai->cmd, &argc, &argv, &error)) {
		printf("Error parsing command line: %s\n",ai->cmd);
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
		printf("Error spawning process\n");
		gtk_main_quit();
		g_strfreev(argv);
		return;
	}

	g_strfreev(argv);

	g_child_watch_add(pid, child_exited, NULL);

	gtk_widget_hide(window);
}

void gtkmainmenu(int *argc, char **argv[], GHashTable *settings, GSList *applications) {
	gtk_init(argc, argv);

	GtkBuilder *builder;
	builder = gtk_builder_new();
	gtk_builder_add_from_string(builder, 
		&_binary_ui_gtkbuilder_xml_start,
		(int)(&_binary_ui_gtkbuilder_xml_end-&_binary_ui_gtkbuilder_xml_start)+1,
		NULL );

	window = GTK_WIDGET(gtk_builder_get_object (builder, "mainwindow"));
	gtk_builder_connect_signals (builder, NULL);

	gchar *logo_filename;
	if( (logo_filename=g_hash_table_lookup(settings,"logo_img")) ) {
		GtkImage *image_logo= GTK_IMAGE(gtk_builder_get_object(builder,"image_logo"));
		gtk_image_set_from_file( image_logo, logo_filename);
	}

	gchar *title;
	if ( (title = g_hash_table_lookup(settings,"title")) ) {
		GtkLabel *label_title= GTK_LABEL(gtk_builder_get_object(builder,"label_title"));
		gtk_label_set_text(label_title, title);
	}

	GtkWidget *vbox_buttons=GTK_WIDGET(gtk_builder_get_object(builder,"vbox_buttons"));
	GSList *apps=applications;
	while(apps) {
		GtkWidget *button;
		ApplicationInfo *ai=(ApplicationInfo*)apps->data;
		button = gtk_button_new_with_label(ai->title);
		gtk_signal_connect(GTK_OBJECT(button), "clicked", G_CALLBACK(button_clicked), ai);
		gtk_box_pack_start(GTK_BOX(vbox_buttons), button, TRUE, TRUE, TRUE);
		gtk_widget_show(button);
		apps=g_slist_next(apps);
	}

	g_object_unref(G_OBJECT(builder));
	gtk_widget_show(window);
	gtk_main();

	return;
/*
	GtkWidget *box1;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

//	gtk_signal_connect(GTK_OBJECT(window), "delete_event",
//				GTK_SIGNAL_FUNC(delete_event), NULL);

	box1 = gtk_vbox_new(FALSE,10);

	GtkWidget *button;

	GSList *apps=applications;
	while(apps) {
		ApplicationInfo *ai=(ApplicationInfo*)apps->data;
printf("%s\n", ai->title);
		button = gtk_button_new_with_label(ai->title);
		gtk_signal_connect(GTK_OBJECT(button), "clicked", G_CALLBACK(button_clicked), ai);
		gtk_box_pack_start(GTK_BOX(box1), button, TRUE, TRUE, TRUE);
		gtk_widget_show(button);
		apps=g_slist_next(apps);
	}

	gtk_container_add(GTK_CONTAINER(window), box1);

	gtk_widget_show(box1);
	gtk_widget_show(window);
	gtk_main();
*/
}
