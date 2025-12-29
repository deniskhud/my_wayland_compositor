#include "../include/clients/client.h"

#include <wlr/types/wlr_scene.h>

#include "../include/input/cursor.h"
#include "../include/server.h"


#include <wlr/types/wlr_output_layout.h>
#include "../../protocols/wlr-layer-shell-unstable-v1-protocol.h"
#include "../include/tree/node.h"
#include "../include/output.h"

/*** toplevel ***/
void client_new_xdg_toplevel(struct wl_listener* listener, void* data) {
	struct server* server = wl_container_of(listener, server, new_xdg_toplevel);

	struct wlr_xdg_toplevel* xdg_toplevel = data;

	struct client_xdg_toplevel* client_toplevel = calloc(1, sizeof(struct client_xdg_toplevel));

	client_toplevel->xdg_toplevel = xdg_toplevel;
	client_toplevel->server = server;

	//добавление в сцену

	//default mode
	client_toplevel->mode = WINDOW_TILING;

	/* Listen to the various events it can emit */
	client_toplevel->map.notify = mxdg_toplevel_map;
	wl_signal_add(&xdg_toplevel->base->surface->events.map, &client_toplevel->map);

	client_toplevel->unmap.notify = mxdg_toplevel_unmap;
	wl_signal_add(&xdg_toplevel->base->surface->events.unmap, &client_toplevel->unmap);

	client_toplevel->commit.notify = mxdg_toplevel_commit;
	wl_signal_add(&xdg_toplevel->base->surface->events.commit, &client_toplevel->commit);

	client_toplevel->destroy.notify = client_xdg_toplevel_destroy;
	wl_signal_add(&xdg_toplevel->events.destroy, &client_toplevel->destroy);

	client_toplevel->request_move.notify = mxdg_toplevel_request_move;
	wl_signal_add(&xdg_toplevel->events.request_move, &client_toplevel->request_move);

	client_toplevel->request_resize.notify = mxdg_toplevel_request_resize;
	wl_signal_add(&xdg_toplevel->events.request_resize, &client_toplevel->request_resize);

	client_toplevel->request_maximize.notify = mxdg_toplevel_request_maximize;
	wl_signal_add(&xdg_toplevel->events.request_maximize, &client_toplevel->request_maximize);

	client_toplevel->request_fullscreen.notify = mxdg_toplevel_request_fullscreen;
	wl_signal_add(&xdg_toplevel->events.request_fullscreen, &client_toplevel->request_fullscreen);

	//
	client_toplevel->request_minimize.notify = mxdg_toplevel_request_minimize;
	wl_signal_add(&xdg_toplevel->events.request_minimize, &client_toplevel->request_minimize);

	//
	client_toplevel->set_title.notify = mxdg_toplevel_set_title;
	wl_signal_add(&xdg_toplevel->events.set_title, &client_toplevel->set_title);

	//
	client_toplevel->show_window_menu.notify = mxdg_toplevel_show_window_menu;
	wl_signal_add(&xdg_toplevel->events.request_show_window_menu, &client_toplevel->show_window_menu);

	//
	client_toplevel->set_app_id.notify = mxdg_toplevel_set_app_id;
	wl_signal_add(&xdg_toplevel->events.set_app_id, &client_toplevel->set_app_id);

	client_toplevel->set_parent.notify = mxdg_toplevel_set_parent;
	wl_signal_add(&xdg_toplevel->events.set_parent, &client_toplevel->set_parent);

	wl_list_insert(&client_toplevel->server->clients, &client_toplevel->link);

}

void mxdg_toplevel_map(struct wl_listener* listener, void* data) {
	//wlr_log(WLR_INFO, "MAPPED %p", listener);
	struct client_xdg_toplevel* client_toplevel = wl_container_of(listener, client_toplevel, map);

	// if we have no open windows yet, add one to tree

	//unless we have a focused window, then add a split node

	//добавление в сцену
	client_toplevel->scene_tree =
		wlr_scene_xdg_surface_create(client_toplevel->server->layers.tiling, client_toplevel->xdg_toplevel->base);
	client_toplevel->scene_tree->node.data = client_toplevel;
	client_toplevel->xdg_toplevel->base->data = client_toplevel->scene_tree;

	arrange_windows(client_toplevel->server);
	//focus to new window
	focus_toplevel(client_toplevel);


}

void mxdg_toplevel_unmap(struct wl_listener* listener, void* data) {
	struct client_xdg_toplevel* client_toplevel = wl_container_of(listener, client_toplevel, map);


}
void mxdg_toplevel_commit(struct wl_listener* listener, void* data) {
	struct client_xdg_toplevel* client_toplevel = wl_container_of(listener, client_toplevel, commit);

	if (client_toplevel->xdg_toplevel->base->initial_commit) {
		wlr_xdg_surface_schedule_configure(client_toplevel->xdg_toplevel->base);
	}
	client_toplevel->box = client_get_geometry(client_toplevel);


	//wlr_log(WLR_INFO, "current geometry is -> w : %i, h : %i, x : %i, y : %i", client_toplevel->box->width, client_toplevel->box->height, client_toplevel->box->x, client_toplevel->box->y);


}
void mxdg_toplevel_request_move(struct wl_listener* listener, void* data) {
	struct client_xdg_toplevel* toplevel = wl_container_of(listener, toplevel, request_move);
	//begin_interactive(toplevel, CURSOR_MOVE, 0);
}
void mxdg_toplevel_request_resize(struct wl_listener* listener, void* data) {
	struct client_xdg_toplevel* toplevel = wl_container_of(listener, toplevel, request_resize);
	//begin_interactive(toplevel, CURSOR_RESIZE, 0);
}
void mxdg_toplevel_request_fullscreen(struct wl_listener* listener, void* data) {
	struct client_xdg_toplevel* toplevel = wl_container_of(listener, toplevel, request_fullscreen);
	wlr_xdg_toplevel_set_fullscreen(toplevel->xdg_toplevel, true);
}

void mxdg_toplevel_request_maximize(struct wl_listener* listener, void* data) {
	struct client_xdg_toplevel* toplevel = wl_container_of(listener, toplevel, request_maximize);
	//wlr_xdg_surface_schedule_configure(toplevel->xdg_toplevel->base);
	//wlr_xdg_toplevel_set_maximized(toplevel->xdg_toplevel, true);
}

void mxdg_toplevel_request_minimize(struct wl_listener* listener, void* data) {

}
void mxdg_toplevel_set_app_id(struct wl_listener* listener, void* data) {

}
void mxdg_toplevel_set_title(struct wl_listener* listener, void* data) {

}
void mxdg_toplevel_set_parent(struct wl_listener* listener, void* data) {

}
void mxdg_toplevel_show_window_menu(struct wl_listener* listener, void* data) {

}

void client_xdg_toplevel_destroy(struct wl_listener* listener, void* data) {

	struct client_xdg_toplevel* toplevel = wl_container_of(listener, toplevel, destroy);


	if (toplevel->scene_tree) {
		wlr_scene_node_destroy(&toplevel->scene_tree->node);
	}
	wl_list_remove(&toplevel->link);

	wl_list_remove(&toplevel->commit.link);
	wl_list_remove(&toplevel->map.link);
	wl_list_remove(&toplevel->unmap.link);

	wl_list_remove(&toplevel->request_fullscreen.link);
	wl_list_remove(&toplevel->request_maximize.link);
	wl_list_remove(&toplevel->request_move.link);
	wl_list_remove(&toplevel->request_resize.link);
	wl_list_remove(&toplevel->destroy.link);
	wl_list_remove(&toplevel->request_minimize.link);
	wl_list_remove(&toplevel->set_app_id.link);
	wl_list_remove(&toplevel->set_parent.link);
	wl_list_remove(&toplevel->set_title.link);
	wl_list_remove(&toplevel->show_window_menu.link);


	arrange_windows(toplevel->server);
	free(toplevel);
	wlr_log(WLR_INFO, "Client xdg_toplevel destroyed");
}


void focus_toplevel(struct client_xdg_toplevel* toplevel) {
	struct server* server = toplevel->server;
	struct wlr_seat* seat = server->seat;
	struct wlr_surface* surface = toplevel->xdg_toplevel->base->surface;
	struct wlr_surface* prev = seat->keyboard_state.focused_surface;
	if (prev == surface) return;

	if (prev) {
		//disable the previous client
		struct wlr_xdg_toplevel *prev_toplevel =
			wlr_xdg_toplevel_try_from_wlr_surface(prev);
		if (prev_toplevel != NULL) {
			wlr_xdg_toplevel_set_activated(prev_toplevel, false);
		}
	}
	struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
	/* Move the toplevel to the front */
	wlr_scene_node_raise_to_top(&toplevel->scene_tree->node);
	wl_list_remove(&toplevel->link);
	wl_list_insert(&server->clients, &toplevel->link);
	/* Activate the new surface */
	wlr_xdg_toplevel_set_activated(toplevel->xdg_toplevel, true);

	if (keyboard != NULL) {
		wlr_seat_keyboard_notify_enter(seat, surface,
			keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
	}

	server->current_focus = toplevel;

	wlr_log(WLR_INFO, "Current focused toplevel is %s", server->current_focus->xdg_toplevel->app_id);
}

struct client_xdg_toplevel *desktop_toplevel_at(
		struct server *server, double lx, double ly,
		struct wlr_surface **surface, double *sx, double *sy) {


	struct wlr_scene_node *node = wlr_scene_node_at(
		&server->scene->tree.node, lx, ly, sx, sy);
	if (node == NULL || node->type != WLR_SCENE_NODE_BUFFER) {
		return NULL;
	}
	struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node);
	struct wlr_scene_surface *scene_surface =
		wlr_scene_surface_try_from_buffer(scene_buffer);
	if (!scene_surface) {
		return NULL;
	}

	*surface = scene_surface->surface;

	struct wlr_scene_tree *tree = node->parent;
	while (tree != NULL && tree->node.data == NULL) {
		tree = tree->node.parent;
	}
	return tree->node.data;
}

void set_focus(struct client_xdg_toplevel* toplevel) {

}


struct wlr_box* client_get_geometry(struct client_xdg_toplevel* toplevel) {
	return &toplevel->xdg_toplevel->base->geometry;
}

void client_set_size(struct client_xdg_toplevel* toplevel, uint32_t width, uint32_t height) {
	wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, (int32_t)width, (int32_t)height);
}

void arrange_windows(struct server* server) {
	int count = 0;
	struct client_xdg_toplevel* client;

	wl_list_for_each(client, &server->clients, link) {
		if (client->mode == WINDOW_TILING) count++;

	}

	if (count == 0) return;
	wlr_log(WLR_INFO, "Client %d (%d clients)", count, count);
	int screen_w = server->output->wlr_output->width;
	int screen_h = server->output->wlr_output->height;
	wlr_log(WLR_INFO, "OUTPUT COORDS %i, %i" , screen_h, screen_w);
	int each_w = screen_w / count;

	int index = 0;
	wl_list_for_each(client, &server->clients, link) {
		if (client->mode != WINDOW_TILING) {
			continue;
		}
		int x = each_w * index;
		int y = 0;

		client_set_size(client, each_w, screen_h);

		wlr_scene_node_set_position(&client->scene_tree->node, x, y);

		index++;
	}
}

/*** popup ***/

void client_new_xdg_popup(struct wl_listener* listener, void* data) {
	struct server* server = wl_container_of(listener, server, new_xdg_popup);
	wlr_log(WLR_INFO, "New xdg popup");
	struct wlr_xdg_popup* popup = data;

	struct client_xdg_popup* client = calloc(1, sizeof(struct client_xdg_popup));

	client->xdg_popup = popup;

	struct wlr_xdg_surface *parent = wlr_xdg_surface_try_from_wlr_surface(popup->parent);
	if (parent == NULL) {
		return;
	}
	struct wlr_scene_tree *parent_tree = parent->data;

	popup->base->data = wlr_scene_xdg_surface_create(parent_tree, popup->base);

	client->commit.notify = client_popup_commit;
	wl_signal_add(&popup->base->surface->events.commit, &client->commit);

	client->destroy.notify = client_xdg_popup_destroy;
	wl_signal_add(&popup->events.destroy, &client->destroy);

	client->reposition.notify = client_popup_reposition;
	wl_signal_add(&popup->events.reposition, &client->reposition);


}

void client_xdg_popup_destroy(struct wl_listener* listener, void* data) {
	struct client_xdg_popup* popup = wl_container_of(listener, popup, destroy);

	wl_list_remove(&popup->destroy.link);
	wl_list_remove(&popup->reposition.link);
	wl_list_remove(&popup->commit.link);
	free(popup);
}

void client_popup_commit(struct wl_listener *listener, void *data) {
	struct client_xdg_popup* client = wl_container_of(listener, client, commit);

	if (client->xdg_popup->base->initial_commit) {
		wlr_xdg_surface_schedule_configure(client->xdg_popup->base);
	}
}

void client_popup_reposition(struct wl_listener* listener, void* data) {
	struct client_xdg_popup* popup = wl_container_of(listener, popup, reposition);

	struct wlr_xdg_popup* wlr_popup = popup->xdg_popup;

	// если используешь сцены wlroots:
	if (popup->scene_tree) {
		// перемещаем сценический узел в новые координаты
		wlr_scene_node_set_position(
			&popup->scene_tree->node,
			wlr_popup->base->geometry.x,
			wlr_popup->base->geometry.y
		);
	}

}


/*** windows mods functions ***/
void client_set_tiling_mode(struct client_xdg_toplevel* toplevel) {
	wlr_scene_node_destroy(&toplevel->scene_tree->node);

	toplevel->mode = WINDOW_TILING;

	toplevel->scene_tree =
		wlr_scene_xdg_surface_create(toplevel->server->layers.tiling, toplevel->xdg_toplevel->base);
	toplevel->scene_tree->node.data = toplevel;
	toplevel->xdg_toplevel->base->data = toplevel->scene_tree;

	arrange_windows(toplevel->server);
}

void client_set_floating_mode(struct client_xdg_toplevel* toplevel) {
	wlr_log(WLR_INFO, "FLOATING MODE for %s", toplevel->xdg_toplevel->app_id);
	wlr_scene_node_destroy(&toplevel->scene_tree->node);

	toplevel->mode = WINDOW_FLOATING;

	toplevel->scene_tree =
		wlr_scene_xdg_surface_create(toplevel->server->layers.floating, toplevel->xdg_toplevel->base);
	toplevel->scene_tree->node.data = toplevel;
	toplevel->xdg_toplevel->base->data = toplevel->scene_tree;

	arrange_windows(toplevel->server);

}

void client_set_holding_mode(struct client_xdg_toplevel* toplevel) {
	wlr_scene_node_destroy(&toplevel->scene_tree->node);

	toplevel->mode = WINDOW_HOLDING;

	toplevel->scene_tree =
		wlr_scene_xdg_surface_create(toplevel->server->layers.overlay, toplevel->xdg_toplevel->base);
	toplevel->scene_tree->node.data = toplevel;
	toplevel->xdg_toplevel->base->data = toplevel->scene_tree;
	arrange_windows(toplevel->server);
}

