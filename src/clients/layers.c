#include <src/include/clients/layers.h>
#include <src/include/server.h>

#include "src/include/output.h"
#include "src/include/input/cursor.h"


void handle_new_layer_surface(struct wl_listener *listener, void *data) {
	struct server* server = wl_container_of(listener, server, new_layer_surface);

	struct wlr_layer_surface_v1 *layer_surface = data;
	wlr_log(WLR_INFO, "New layer surface surface, ENTER TO FUNCTION");
	if (!layer_surface->output) {
		struct wlr_output* output = wlr_output_layout_output_at(server->output_layout,
			server->cursor->wlr_cursor->x, server->cursor->wlr_cursor->y);
		if (!output) {
			wlr_log(WLR_INFO,
				"No output available to assign layer surface");
			wlr_layer_surface_v1_destroy(layer_surface);
			return;
		}
		layer_surface->output = output;
	}
	struct client_layer_surface* surface = calloc(1, sizeof(struct client_layer_surface));
	surface->server = server;
	surface->layer_surface = layer_surface;

	struct server_output *output = server->output;

	wlr_fractional_scale_v1_notify_scale(layer_surface->surface, output->wlr_output->scale);

	struct wlr_scene_tree *selected_layer =
		output->layers_tree[layer_surface->current.layer];

	surface->scene_layer_surface = wlr_scene_layer_surface_v1_create(
		selected_layer, layer_surface);
	if (!surface->scene_layer_surface) {
		wlr_layer_surface_v1_destroy(layer_surface);
		wlr_log(WLR_ERROR, "could not create layer surface");
		return;
	}

	/* In support of IME popup */
	layer_surface->surface->data = surface->scene_layer_surface->tree;

	/*node_descriptor_create(&surface->scene_layer_surface->tree->node,
		LAB_NODE_LAYER_SURFACE, /*view#1# NULL, surface);*/

	surface->server = server;
	surface->scene_layer_surface->layer_surface = layer_surface;

	surface->surface_commit.notify = handle_surface_commit;
	wl_signal_add(&layer_surface->surface->events.commit,
		&surface->surface_commit);

	surface->map.notify = handle_map;
	wl_signal_add(&layer_surface->surface->events.map, &surface->map);

	surface->unmap.notify = handle_unmap;
	wl_signal_add(&layer_surface->surface->events.unmap, &surface->unmap);

	surface->new_popup.notify = handle_new_popup;
	wl_signal_add(&layer_surface->events.new_popup, &surface->new_popup);

	surface->output_destroy.notify = handle_output_destroy;
	wl_signal_add(&layer_surface->output->events.destroy,
		&surface->output_destroy);

	surface->node_destroy.notify = handle_node_destroy;
	wl_signal_add(&surface->scene_layer_surface->tree->node.events.destroy,
		&surface->node_destroy);

}

void
handle_surface_commit(struct wl_listener *listener, void *data)
{
	struct client_layer_surface *layer =
		wl_container_of(listener, layer, surface_commit);
	wlr_log(WLR_INFO, "ENTER TO SURFACE COMMIT");
	struct wlr_layer_surface_v1 *layer_surface =
		layer->scene_layer_surface->layer_surface;
	struct wlr_output *wlr_output =
		layer->scene_layer_surface->layer_surface->output;

	if (!wlr_output) {
		return;
	}

	uint32_t committed = layer_surface->current.committed;
	struct server_output *output = layer->server->output;

	uint32_t width = output->wlr_output->width;
	uint32_t height = output->wlr_output->height;
	switch (layer_surface->current.layer) {
		case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
		case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
			// полный экран
			break;
		case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
		case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
			// ограничиваем высоту по exclusive_zone, если есть
			if (layer_surface->current.exclusive_zone > 0) {
				height = layer_surface->current.exclusive_zone;
			} else {
				height = 24; // пример минимальной панели1
			}
			break;
	}

	wlr_layer_surface_v1_configure(layer_surface, width, height);

	/* Process layer change */
	if (committed & WLR_LAYER_SURFACE_V1_STATE_LAYER) {
		wlr_scene_node_reparent(&layer->scene_layer_surface->tree->node,
			output->layers_tree[layer_surface->current.layer]);
	}
	/* Process keyboard-interactivity change */
	if (committed & WLR_LAYER_SURFACE_V1_STATE_KEYBOARD_INTERACTIVITY) {
		/*
		 * On-demand interactivity should only be honoured through
		 * normal focus semantics (for example by surface receiving
		 * cursor-button-press).
		 */
		/*if (is_on_demand(layer_surface)) {
			struct seat *seat = &layer->server->seat;
			if (seat->focused_layer == layer_surface) {
				/*
				 * Must be change from EXCLUSIVE to ON_DEMAND,
				 * so we should give us focus.
				 #1#
				struct server *server = layer->server;
				try_to_focus_next_layer_or_toplevel(server);
			}
			goto out;
		}*/
		goto out;
		/*/* Handle EXCLUSIVE and NONE requests #1#
		struct seat *seat = &layer->server->seat;
		layer_try_set_focus(seat, layer_surface);*/
	}
	out:
		if (committed || layer->mapped != layer_surface->surface->mapped) {
			layer->mapped = layer_surface->surface->mapped;
			//output_update_usable_area(output);
			/*
			 * Update cursor focus here to ensure we
			 * enter a new/moved/resized layer surface.
			 */
			//cursor_update_focus(layer->server);
		}
}


void handle_map(struct wl_listener* listener, void* data) {
	struct client_layer_surface *layer = wl_container_of(listener, layer, map);
	wlr_log(WLR_INFO, "ENTER TO MAP COMMIT");
	struct wlr_output *wlr_output =
		layer->scene_layer_surface->layer_surface->output;
	if (wlr_output) {
		//output_update_usable_area(wlr_output->data);
	}

	/*
	 * Since moving to the wlroots scene-graph API, there is no need to
	 * call wlr_surface_send_enter() from here since that will be done
	 * automatically based on the position of the surface and outputs in
	 * the scene. See wlr_scene_surface_create() documentation.
	 */

	//struct seat *seat = &layer->server->seat;
	//layer_try_set_focus(seat, layer->scene_layer_surface->layer_surface);
}
void handle_unmap(struct wl_listener* listener, void* data) {
	struct client_layer_surface *layer = wl_container_of(listener, layer, unmap);
	struct wlr_layer_surface_v1 *layer_surface =
		layer->scene_layer_surface->layer_surface;

	/*
	 * If we send a configure event in unmap handler, the layer-shell
	 * client sends ack_configure back and wlroots posts a
	 * "wrong configure serial" error, which terminates the client (see
	 * https://github.com/labwc/labwc/pull/1154#issuecomment-2906885183).
	 *
	 * To prevent this, we set being_unmapped here and check it in
	 * arrange_one_layer() called by output_update_usable_area().
	 */
	layer->being_unmapped = true;

	if (layer_surface->output) {
		//output_update_usable_area(layer_surface->output->data);
	}
	/*struct seat *seat = &layer->server->seat;
	if (seat->focused_layer == layer_surface) {
		//try_to_focus_next_layer_or_toplevel(layer->server);
	}*/
	layer->being_unmapped = false;
}
void handle_new_popup(struct wl_listener* listener, void* data) {

}
void handle_output_destroy(struct wl_listener* listener, void* data) {

}
void handle_node_destroy(struct wl_listener* listener, void* data) {
	struct client_layer_surface* layer = wl_container_of(listener, layer, node_destroy);

	wl_list_remove(&layer->map.link);
	wl_list_remove(&layer->unmap.link);
	wl_list_remove(&layer->surface_commit.link);
	wl_list_remove(&layer->new_popup.link);
	wl_list_remove(&layer->output_destroy.link);
	wl_list_remove(&layer->node_destroy.link);
	free(layer);
}

void
layer_try_set_focus(struct seat *seat, struct wlr_layer_surface_v1 *layer_surface)
{
	/*switch (layer_surface->current.keyboard_interactive) {
		case ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE:
			wlr_log(WLR_DEBUG, "interactive-exclusive '%p'", layer_surface);
			if (has_precedence(seat, layer_surface->current.layer)) {
				seat_set_focus_layer(seat, layer_surface);
			}
			break;
		case ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND:
			wlr_log(WLR_DEBUG, "interactive-on-demand '%p'", layer_surface);
			if (!focused_layer_has_exclusive_interactivity(seat)) {
				seat_set_focus_layer(seat, layer_surface);
			}
			break;
		case ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE:
			wlr_log(WLR_DEBUG, "interactive-none '%p'", layer_surface);
			if (seat->focused_layer == layer_surface) {
				try_to_focus_next_layer_or_toplevel(seat->server);
			}
			break;
	}*/
}