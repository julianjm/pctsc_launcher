#include <gtk/gtk.h>
#include "xmlparser.h"

static GtkWidget *window;

gint delete_event (GtkWidget *widget, GdkEvent *event, gpointer data) {
	gtk_main_quit();
	return FALSE;
}

void child_exited(GPid pid, gint status, gpointer user_data) {
	g_spawn_close_pid(pid);
	gtk_main_quit();	
}

void button_clicked(GtkWidget *widget, gpointer data) {
	ApplicationInfo *ai=(ApplicationInfo*) data;
	g_print("Click '%s'\n", ai->title);


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
		return;
	}

	g_child_watch_add(pid, child_exited, NULL);

	gtk_widget_hide(window);
}

void gtkmainmenu(int argc, char *argv[], GHashTable *settings, GSList *applications) {
	gtk_init(&argc, &argv);

	GtkWidget *box1;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_signal_connect(GTK_OBJECT(window), "delete_event",
				GTK_SIGNAL_FUNC(delete_event), NULL);

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
}
