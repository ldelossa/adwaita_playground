#include <adwaita.h>

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Window");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

	GtkWidget *pref_group = adw_preferences_group_new();
	// make a testing label
	GtkWidget *label = gtk_label_new("Hello World");

	gtk_window_set_child(GTK_WINDOW(window), pref_group);
	gtk_widget_set_hexpand(pref_group, true);
	gtk_widget_set_vexpand(pref_group, true);
	adw_preferences_group_add(ADW_PREFERENCES_GROUP(pref_group), label);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
