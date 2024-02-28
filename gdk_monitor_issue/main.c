#include <adwaita.h>

static void panel_on_monitor_change(GListModel *monitors, guint position,
                                    guint removed, guint added,
                                    gpointer gtk_app) {
    uint8_t n = g_list_model_get_n_items(monitors);
    const char *desc = NULL;
    const char *model = NULL;
    const char *connector = NULL;
    const char *manufacturer = NULL;

    for (uint8_t i = 0; i < n; i++) {
        GdkMonitor *mon = g_list_model_get_item(monitors, i);
        desc = gdk_monitor_get_description(mon);
        model = gdk_monitor_get_model(mon);
        connector = gdk_monitor_get_connector(mon);
        manufacturer = gdk_monitor_get_manufacturer(mon);
        g_critical(
            "panel.c:panel_on_monitor_change(): bar already exists for "
            "monitor [%s] [%s] [%s] [%s]",
            desc, model, connector, manufacturer);
    }
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Window");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
    gtk_window_present(GTK_WINDOW(window));

    GdkSeat *seat = gdk_display_get_default_seat(gdk_display_get_default());
    GdkDisplay *display = gdk_seat_get_display(seat);
    GListModel *monitors = gdk_display_get_monitors(display);

    panel_on_monitor_change(monitors, 0, 0, 0, app);

    g_signal_connect(monitors, "items-changed",
                     G_CALLBACK(panel_on_monitor_change), app);
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
