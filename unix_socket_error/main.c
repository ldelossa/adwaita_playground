#include <adwaita.h>
#include <errno.h>
#include <glib-unix.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK "/var/run/user/1000/mysocket.sock"

// callback function for unix socket
gboolean on_recv(gint fd, GIOCondition condition, GtkApplication *app) {
	g_debug("Received message on socket %d", fd);

    // receive message
	uint8_t buf[16];
again:
    if (read(34, buf, sizeof(buf)) != 0) {
        if (errno == EINTR) goto again;
        if (errno) {
            printf("Error: %s\n", strerror(errno));
            exit(1);
        }
    }
	g_debug("Received message: %x", buf[0]);
    return TRUE;
}

void activate(GtkApplication *app, gpointer user_data) {
    // connect to unix socket and get fd
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCK);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        printf("Connect Error: %s\n", strerror(errno));
        exit(1);
    }
	g_debug("Connected to socket %d", fd);

    g_unix_fd_add(fd, G_IO_IN | G_IO_ERR | G_IO_HUP, (GUnixFDSourceFunc)on_recv,
                  app);

	// spawn a dummy window to keep application alive
	GtkWidget *window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "Dummy");
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
	gtk_window_present(GTK_WINDOW(window));

}

// gtk4 main function with application activate
int main(int argc, char *argv[]) {
    GtkApplication *app;
    int status;

    g_setenv("G_MESSAGES_DEBUG", "all", TRUE);

    app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
