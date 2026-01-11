#ifndef LAYER_H
#define LAYER_H

#include <wlr/types/wlr_xdg_shell.h>
#include <stdlib.h>
#include <wlr/types/wlr_fractional_scale_v1.h>

struct client_layer_surface {
	struct wlr_layer_surface_v1 *layer_surface;
	struct wlr_scene_layer_surface_v1 *scene_layer_surface;
	struct server *server;

	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener surface_commit;
	struct wl_listener output_destroy;
	struct wl_listener node_destroy;
	struct wl_listener new_popup;
	bool mapped;
	bool being_unmapped;
};

struct client_layer_popup {
	struct wlr_xdg_popup *wlr_popup;
	struct wlr_scene_tree *scene_tree;
	struct server *server;

	struct wlr_box output_toplevel_sx_box;

	struct wl_listener commit;
	struct wl_listener destroy;
	struct wl_listener new_popup;
	struct wl_listener reposition;
};
/*void layers_init(struct server *server);
void layers_finish(struct server *server);

void layers_arrange(struct output *output);
void layer_try_set_focus(struct seat *seat, struct wlr_layer_surface_v1 *layer_surface);*/

void handle_new_layer_surface(struct wl_listener *listener, void *data);
void handle_surface_commit(struct wl_listener* listener, void* data);
void handle_map(struct wl_listener* listener, void* data);
void handle_unmap(struct wl_listener* listener, void* data);
void handle_new_popup(struct wl_listener* listener, void* data);
void handle_output_destroy(struct wl_listener* listener, void* data);
void handle_node_destroy(struct wl_listener* listener, void* data);

//void layer_try_set_focus(struct seat *seat, struct wlr_layer_surface_v1 *layer_surface);

#endif //LAYER_H

