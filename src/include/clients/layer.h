#ifndef LAYER_H
#define LAYER_H

#include <wlr/types/wlr_xdg_shell.h>
#include <stdlib.h>

struct client_layer {
	struct wlr_layer_surface_v1 *layer_surface;

	struct wl_listener destroy;
	struct wl_listener new_popup;
	struct wl_listener unmap;
	struct wl_listener commit;

	struct server* server;
	struct wlr_scene_layer_surface_v1 *scene_layer;
	struct wlr_scene_tree* scene;
	struct wlr_scene_tree* popups;
	struct server_output* output;

	struct wl_list link;
};



#endif //LAYER_H
