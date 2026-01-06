#ifndef OUTPUT_H
#define OUTPUT_H
#include <stdlib.h>
#include <wayland-server-core.h>

#include <wlr/types/wlr_output.h>

extern struct server* server;
struct server_output {
	struct server* server;
	struct wlr_output* wlr_output;
	struct wl_list link;
	struct wl_listener frame;
	struct wl_listener request_state;
	struct wl_listener destroy;

	struct wlr_scene_tree* layers_tree[4];
	struct wlr_scene_output* scene_output;

};



void server_new_output(struct wl_listener* listener, void* data);

void output_frame(struct wl_listener* listener, void* data);
void output_request_state(struct wl_listener* listener, void* data);
void output_destroy(struct wl_listener* listener, void* data);


#endif //OUTPUT_H
