#include "include/output.h"
#include "include/server.h"
#include <wayland-util.h>

void server_new_output(struct wl_listener* listener, void* data) {
	struct server* server = wl_container_of(listener, server, new_output);
	struct wlr_output* wlr_output = data;

	wlr_output_init_render(wlr_output, server->allocator, server->renderer);

	struct wlr_output_state state;
	wlr_output_state_init(&state);
	wlr_output_state_set_enabled(&state, true);

	if (!wlr_output_commit_state(wlr_output, &state)) {
		wlr_log(WLR_ERROR, "Failed to commit output state");
		wlr_log(WLR_ERROR, "Failed to commit output state");
		wlr_log(WLR_ERROR, "Failed to commit output state");
		wlr_log(WLR_ERROR, "Failed to commit output state");
	}

	wlr_output_state_finish(&state);

	struct server_output* output = calloc(1, sizeof(struct server_output));
	//не забыть подвязать монитор к серверу
	server->output = output;

	output->wlr_output = wlr_output;
	output->server = server;

	output->frame.notify = output_frame;
	wl_signal_add(&wlr_output->events.frame, &output->frame);

	output->request_state.notify = output_request_state;
	wl_signal_add(&wlr_output->events.request_state, &output->request_state);

	output->destroy.notify = output_destroy;
	wl_signal_add(&wlr_output->events.destroy, &output->destroy);

	wl_list_insert(&server->outputs, &output->link);

	struct wlr_output_layout_output *l_output = wlr_output_layout_add_auto(server->output_layout,
		wlr_output);
	output->scene_output = wlr_scene_output_create(server->scene, wlr_output);
	wlr_scene_output_layout_add_output(server->scene_layout, l_output, output->scene_output);

	//create layers
	for (size_t i = 0; i < 4; ++i) {
		output->layers_tree[i] = wlr_scene_tree_create(&server->scene->tree);
	}


	/*
	 * Set the z-positions to achieve the following order (from top to
	 * bottom):
	 *	- session lock layer
	 *	- window switcher osd
	 *	- (compositor menu)
	 *	- layer-shell popups
	 *	- overlay layer
	 *	- top layer
	 *	- (views)
	 *	- bottom layer
	 *	- background layer
	 */
	/***
		[3] - overlay
		[2]	- top layer
		[1]	- bottom layer
		[0] - background
	 ***/
	wlr_scene_node_lower_to_bottom(&output->layers_tree[1]->node);
	wlr_scene_node_lower_to_bottom(&output->layers_tree[0]->node);

	wlr_scene_node_raise_to_top(&output->layers_tree[2]->node);
	wlr_scene_node_raise_to_top(&output->layers_tree[3]->node);


	wlr_log(WLR_INFO, "Created new output %s", output->wlr_output->name);
}

void output_frame(struct wl_listener* listener, void* data) {
	struct server_output* output = wl_container_of(listener, output, frame);
	struct wlr_scene* scene = output->server->scene;

	struct wlr_scene_output* scene_output = wlr_scene_get_scene_output(scene, output->wlr_output);

	wlr_scene_output_commit(scene_output,NULL);

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	wlr_scene_output_send_frame_done(scene_output,&now);

}
void output_request_state(struct wl_listener* listener, void* data) {
	struct server_output* output = wl_container_of(listener, output, request_state);
	struct wlr_output_event_request_state *event = data;
	wlr_output_commit_state(output->wlr_output, event->state);

}
void output_destroy(struct wl_listener* listener, void* data) {
	struct server_output* output = wl_container_of(listener, output, destroy);

	for (size_t i = 0; i < 4; ++i) {
		wlr_scene_node_destroy(&output->layers_tree[i]->node);
	}

	wl_list_remove(&output->frame.link);
	wl_list_remove(&output->request_state.link);
	wl_list_remove(&output->destroy.link);
	wl_list_remove(&output->link);
	free(output);
}

