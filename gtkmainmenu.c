#include <gtk/gtk.h>
#include "xmlparser.h"

gint delete_event (GtkWidget *widget, GdkEvent *event, gpointer data) {
	gtk_main_quit();
	return FALSE;
}

void gtkmainmenu(int argc, char *argv[], GHashTable *settings, GSList *applications) {
	gtk_init(&argc, &argv);

	GtkWidget *window;
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
		gtk_box_pack_start(GTK_BOX(box1), button, TRUE, TRUE, TRUE);
		gtk_widget_show(button);
		apps=g_slist_next(apps);
	}

	gtk_container_add(GTK_CONTAINER(window), box1);

	gtk_widget_show(box1);
	gtk_widget_show(window);
	gtk_main();
}
