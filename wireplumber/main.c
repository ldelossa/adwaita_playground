#include <adwaita.h>
#include <spa-0.2/spa/param/props.h>
#include <spa-0.2/spa/pod/iter.h>
#include <stdlib.h>
#include <string.h>
#include <wireplumber-0.4/wp/wp.h>

#include "wp/core.h"
#include "wp/global-proxy.h"
#include "wp/iterator.h"
#include "wp/link.h"
#include "wp/metadata.h"
#include "wp/node.h"
#include "wp/object-manager.h"
#include "wp/object.h"
#include "wp/plugin.h"
#include "wp/properties.h"
#include "wp/proxy-interfaces.h"
#include "wp/spa-pod.h"
#include "wp/spa-type.h"

WpCore *core = NULL;

static void print_pw_node_spa_props(GObject *obj, GAsyncResult *res,
                                    gpointer data) {
    WpPipewireObject *node = WP_PIPEWIRE_OBJECT(obj);
    GValue value = G_VALUE_INIT;
    GError *error = NULL;

    WpIterator *i = wp_pipewire_object_enum_params_finish(node, res, &error);
    if (error) {
        g_debug("error: %s", error->message);
        return;
    }

    while (wp_iterator_next(i, &value)) {
        const char *name = NULL;

        float vol = 0.1;

        WpSpaPod *pod = NULL;
        if (G_VALUE_TYPE(&value) != WP_TYPE_SPA_POD) {
            g_debug("value is not a spa pod");
            continue;
        }
        pod = g_value_get_boxed(&value);

        wp_spa_pod_get_object(pod, &name, "volume", "?f", &vol, NULL);

        g_debug("name: %s, vol: %f", name, vol);

        g_value_unset(&value);
    }
}

static void print_pw_node(WpNode *node) {
    WpProperties *props =
        wp_global_proxy_get_global_properties(WP_GLOBAL_PROXY(node));

    GValue value = G_VALUE_INIT;
    WpIterator *props_i = wp_properties_new_iterator(props);

    WpObjectFeatures features =
        wp_object_get_supported_features(WP_OBJECT(node));
    g_debug("features: %d", features);

    while (wp_iterator_next(props_i, &value)) {
        WpPropertiesItem *item = g_value_get_boxed(&value);
        const gchar *key = wp_properties_item_get_key(item);
        const gchar *val = wp_properties_item_get_value(item);

		g_debug("key: %s, val: %s", key, val);

        g_value_unset(&value);
    }

    GVariant *params =
        wp_pipewire_object_get_param_info(WP_PIPEWIRE_OBJECT(node));

    if (!params) {
        g_debug("no params");
        return;
    }

    WpIterator *it = wp_pipewire_object_enum_params_sync(
        WP_PIPEWIRE_OBJECT(node), "Props", NULL);
    for (; it && wp_iterator_next(it, &value); g_value_unset(&value)) {
        WpSpaPod *param = g_value_get_boxed(&value);

        gboolean mute;
        float volume;
        float base;
        float step;
        uint8_t channels;
        float channel_volumes[64];
        g_autoptr(WpSpaPod) channelVolumes = NULL;
        g_autoptr(WpSpaPod) channelMap = NULL;
        g_autoptr(WpSpaPod) monitorVolumes = NULL;

        if (!wp_spa_pod_get_object(param, NULL, "mute", "b", &mute, NULL)) {
            g_debug("no mute");
            continue;
        } else {
            g_debug("mute: %d", mute);
        }

        wp_spa_pod_get_object(param, NULL, "channelMap", "?P", &channelMap,
                              "volumeBase", "?f", &base, "volumeStep", "?f",
                              &step, "volume", "?f", &volume, "monitorVolumes",
                              "?P", &monitorVolumes, NULL);
        g_debug("base: %f, step: %f, volume: %f", base, step, volume);

		//       channels = spa_pod_copy_array(wp_spa_pod_get_spa_pod(channelVolumes),
		//                                     SPA_TYPE_Float, channel_volumes, 64);
		//
		// for (int i = 0; i < channels; i++) {
		// 	g_debug("channel %d: %f", i, channel_volumes[i]);
		// }
    }

    // GVariantIter *params_i = g_variant_iter_new(params);
    // const gchar *key;
    // const gchar *val;
    // while (g_variant_iter_next(params_i, "{ss}", &key, &val)) {
    //     g_debug("key: %s, val: %s", key, val);
    //     if (g_strcmp0(key, "Props") == 0) {
    //         g_debug("getting props");
    //         wp_pipewire_object_enum_params(WP_PIPEWIRE_OBJECT(node), key,
    //         NULL,
    //                                        NULL, print_pw_node_spa_props,
    //                                        NULL);
    //     }
    // }
}

static void print_pw_link(WpLink *link) {
    WpProperties *props =
        wp_global_proxy_get_global_properties(WP_GLOBAL_PROXY(link));

    GValue value = G_VALUE_INIT;
    WpIterator *i = wp_properties_new_iterator(props);

    while (wp_iterator_next(i, &value)) {
        WpPropertiesItem *item = g_value_get_boxed(&value);
        const gchar *key = wp_properties_item_get_key(item);
        const gchar *val = wp_properties_item_get_value(item);

        g_debug("key: %s, val: %s", key, val);

        g_value_unset(&value);
    }
}

static void print_metadata(WpMetadata *md) {
    GValue value = G_VALUE_INIT;
    WpIterator *i = wp_metadata_new_iterator(md, NULL);

    while (wp_iterator_next(i, &value)) {
		guint32 subject;
		const gchar *key;
		const gchar *type;
		const gchar *val;

		wp_metadata_iterator_item_extract(&value, &subject, &key, &type, &val);

		g_debug("subject: %d, key: %s, type: %s, val: %s", subject, key, type, val);

        g_value_unset(&value);
    }
}

static void on_om_install(WpObjectManager *self, gpointer *data) {
    g_debug("object manager installed");
    WpIterator *i = wp_object_manager_new_iterator(self);

    g_auto(GValue) value = G_VALUE_INIT;
    while (wp_iterator_next(i, &value)) {
        GObject *obj = NULL;

        obj = g_value_get_object(&value);
        if (WP_IS_METADATA(obj)) {
            g_debug("object is metadata");
			print_metadata(WP_METADATA(obj));
        }
        // if (WP_IS_LINK(obj)) {
        //     g_debug("object is a link");
        //     print_pw_link(WP_LINK(obj));
        // }
        // if (WP_IS_PLUGIN(obj)) {
        //     g_debug("object is a plugin");
        // }

        g_value_unset(&value);
    }
	
	// WpState *state = wp_state_new("restore-stream");
	// WpProperties *props = wp_state_load(state);
	//
	// if (props) {
	// 	g_debug("loaded state");
	// } else {
	// 	g_debug("no state");
	// }
	//
	// WpIterator *i = wp_properties_new_iterator(props);
	//    GValue value = G_VALUE_INIT;
	//
	//    while (wp_iterator_next(i, &value)) {
	//        WpPropertiesItem *item = g_value_get_boxed(&value);
	//        const gchar *key = wp_properties_item_get_key(item);
	//        const gchar *val = wp_properties_item_get_value(item);
	//        g_debug("key: %s, val: %s", key, val);
	//        g_value_unset(&value);
	//    }
	
}

static void on_wp_core_connected(WpCore *core, gpointer *data) {
    g_debug("wp core connected");

    WpObjectManager *wp_om = wp_object_manager_new();
    WpObjectInterest *all_nodes = wp_object_interest_new_type(WP_TYPE_NODE);
    wp_object_manager_request_object_features(
        wp_om, WP_TYPE_NODE,
        WP_PIPEWIRE_OBJECT_FEATURE_PARAM_PROPS |
            WP_PIPEWIRE_OBJECT_FEATURE_INFO |
            WP_PIPEWIRE_OBJECT_FEATURE_PARAM_FORMAT |
            WP_PIPEWIRE_OBJECT_FEATURE_PARAM_PROFILE |
            WP_PIPEWIRE_OBJECT_FEATURE_PARAM_PORT_CONFIG |
            WP_PIPEWIRE_OBJECT_FEATURE_PARAM_ROUTE);
    WpObjectInterest *all_links = wp_object_interest_new_type(WP_TYPE_LINK);
    wp_object_manager_request_object_features(wp_om, WP_TYPE_LINK, NULL);

    WpObjectInterest *all_metadata = wp_object_interest_new_type(WP_TYPE_METADATA);
    wp_object_manager_request_object_features(wp_om, WP_TYPE_METADATA, NULL);

    wp_object_manager_add_interest_full(wp_om, all_nodes);
    wp_object_manager_add_interest_full(wp_om, all_links);
    wp_object_manager_add_interest_full(wp_om, all_metadata);

    g_signal_connect(wp_om, "installed", G_CALLBACK(on_om_install), NULL);

    wp_core_install_object_manager(core, wp_om);
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Window");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
    gtk_window_present(GTK_WINDOW(window));

    // initialize wireplumber
    g_debug("initializing wireplumber");
    wp_init(WP_INIT_PIPEWIRE);

    const gchar *module_dir = wp_get_module_dir();
    g_debug("module_dir: %s", module_dir);

    g_debug("initializing wireplumber core");
    GMainContext *ctx = g_main_context_default();

    core = wp_core_new(ctx, NULL);

    // connect to 'connected' signal
    g_signal_connect(core, "connected", G_CALLBACK(on_wp_core_connected), NULL);

    wp_core_connect(core);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    g_setenv("G_MESSAGES_DEBUG", "all", TRUE);

    app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
