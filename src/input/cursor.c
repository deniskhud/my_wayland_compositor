#include "../include/input/cursor.h"
#include "../include/server.h"
#include "../include/clients/client.h"
#include "../include/input/input.h"

void server_new_pointer(struct server* server, struct wlr_input_device* device) {
	wlr_cursor_attach_input_device(server->cursor->wlr_cursor, device);
}

void server_cursor_init(struct server_cursor* cursor) {
	cursor->wlr_cursor = wlr_cursor_create();
	wlr_cursor_attach_output_layout(cursor->wlr_cursor, cursor->server->output_layout);
	cursor->cursor_manager = wlr_xcursor_manager_create(NULL, 24);
	wlr_xcursor_manager_load(cursor->cursor_manager, 24);
	wlr_cursor_set_xcursor(cursor->wlr_cursor, cursor->cursor_manager, "left_ptr");
	cursor->cursor_mode = CURSOR_NORMAL;

	cursor->cursor_motion.notify = server_cursor_motion;
	wl_signal_add(&cursor->wlr_cursor->events.motion, &cursor->cursor_motion);

	cursor->cursor_motion_absolute.notify = server_cursor_motion_absolute;
	wl_signal_add(&cursor->wlr_cursor->events.motion_absolute, &cursor->cursor_motion_absolute);

	cursor->cursor_button.notify = server_cursor_button;
	wl_signal_add(&cursor->wlr_cursor->events.button, &cursor->cursor_button);

	cursor->cursor_axis.notify = server_cursor_axis;
	wl_signal_add(&cursor->wlr_cursor->events.axis, &cursor->cursor_axis);

	cursor->cursor_frame.notify = server_cursor_frame;
	wl_signal_add(&cursor->wlr_cursor->events.frame, &cursor->cursor_frame);

	wlr_log(WLR_INFO, "New cursor created");

}

void server_cursor_destroy(struct server_cursor* cursor) {
	wl_list_remove(&cursor->cursor_frame.link);
	wl_list_remove(&cursor->cursor_axis.link);
	wl_list_remove(&cursor->cursor_button.link);
	wl_list_remove(&cursor->cursor_motion_absolute.link);
	wl_list_remove(&cursor->cursor_motion.link);

	wlr_xcursor_manager_destroy(cursor->cursor_manager);
	wlr_cursor_destroy(cursor->wlr_cursor);
	free(cursor);
	wlr_log(WLR_INFO, "Cursor destroyed");
}



void server_cursor_motion(struct wl_listener* listener, void* data) {
	struct server_cursor* cursor = wl_container_of(listener, cursor, cursor_motion);
	struct wlr_pointer_motion_event* event = data;
	wlr_cursor_move(cursor->wlr_cursor, &event->pointer->base, event->delta_x, event->delta_y);
	process_cursor_motion(cursor, event->time_msec);

}

void server_cursor_motion_absolute(struct wl_listener* listener, void* data) {
	struct server_cursor* cursor = wl_container_of(listener, cursor, cursor_motion_absolute);
	struct wlr_pointer_motion_absolute_event* event = data;
	wlr_cursor_warp_absolute(cursor->wlr_cursor, &event->pointer->base, event->x, event->y);
	process_cursor_motion(cursor, event->time_msec);

}

void server_cursor_button(struct wl_listener* listener, void* data) {
	wlr_log(WLR_INFO, "Cursor button begin");

	struct server_cursor* cursor = wl_container_of(listener, cursor, cursor_button);
	cursor->event = data;
	struct wlr_pointer_button_event* event = cursor->event;
	wlr_seat_pointer_notify_button(cursor->server->seat, event->time_msec, event->button, event->state);

	struct wlr_keyboard *keyboard =
		wlr_seat_get_keyboard(cursor->server->seat);

	uint32_t modifiers = 0;
	if (keyboard) {
		modifiers = wlr_keyboard_get_modifiers(keyboard);
	}

	if (event->state == WL_POINTER_BUTTON_STATE_PRESSED) {
		// Нажали: меняем курсор и фокусируем окно под курсором (если есть)
		wlr_cursor_set_xcursor(cursor->wlr_cursor, cursor->cursor_manager, "hand2");
		double sx, sy;
		struct wlr_surface* surface = NULL;
		struct client_xdg_toplevel* toplevel =
			desktop_toplevel_at(cursor->server,
								cursor->wlr_cursor->x, cursor->wlr_cursor->y,
								&surface, &sx, &sy);
		if (toplevel) {
			focus_toplevel(toplevel);
		}
		if ((modifiers & WLR_MODIFIER_CTRL)) {
			if (toplevel->mode == WINDOW_FLOATING && event->button == 0x110) {
				begin_interactive(toplevel, CURSOR_MOVE, 0);
			}
			//button right
			else if (event->button == 0x111) {
				struct wlr_box box = {
					.x = toplevel->scene_tree->node.x,
					.y = toplevel->scene_tree->node.y,
					.width  = toplevel->xdg_toplevel->current.width,
					.height = toplevel->xdg_toplevel->current.height,
				};
				double cx = cursor->wlr_cursor->x;
				double cy = cursor->wlr_cursor->y;

				double left = fabs(cx - box.x);
				double right  = fabs((box.x + box.width)  - cx);
				double top    = fabs(cy - box.y);
				double bottom = fabs((box.y + box.height) - cy);

				uint32_t edges = 0;

				if (left < right)
					edges |= WLR_EDGE_LEFT;
				else
					edges |= WLR_EDGE_RIGHT;

				if (top < bottom)
					edges |= WLR_EDGE_TOP;
				else
					edges |= WLR_EDGE_BOTTOM;

				begin_interactive(toplevel, CURSOR_RESIZE, edges);

			}

		}
		wlr_log(WLR_INFO, "Button pressed, focused: %p", (void*)toplevel);
	}
	else if (event->state == WL_POINTER_BUTTON_STATE_RELEASED) {
		// Отпустили: вернуть обычный курсор
		wlr_log(WLR_DEBUG, "Event state Button Released");
		reset_cursor_mode(cursor);
		// при необходимости: выход из режимов move/resize
	}

	wlr_log(WLR_INFO, "Cursor button released");

}

void server_cursor_axis(struct wl_listener* listener, void* data) {
	struct server_cursor* cursor = wl_container_of(listener, cursor, cursor_axis);
	struct wlr_pointer_axis_event* event = data;
	wlr_seat_pointer_notify_axis(cursor->server->seat, event->time_msec, event->orientation,
		event->delta, event->delta_discrete, event->source, event->relative_direction);

}

void server_cursor_frame(struct wl_listener* listener, void* data) {
	struct server_cursor* cursor = wl_container_of(listener, cursor, cursor_frame);
	wlr_seat_pointer_notify_frame(cursor->server->seat);
}

void process_cursor_motion(struct server_cursor* cursor, uint32_t time) {
	if (cursor->cursor_mode == CURSOR_MOVE) {
		process_cursor_move(cursor);
		return;
	}
	if (cursor->cursor_mode == CURSOR_RESIZE) {
		process_cursor_resize(cursor);
	}
	double sx, sy;
	struct wlr_seat *seat = cursor->server->seat;
	struct wlr_surface *surface = NULL;
	struct client_xdg_toplevel *toplevel = desktop_toplevel_at(cursor->server,
				cursor->wlr_cursor->x, cursor->wlr_cursor->y, &surface, &sx, &sy);
	if (!toplevel) {

		wlr_cursor_set_xcursor(cursor->wlr_cursor, cursor->cursor_manager, "default");
	}
	if (surface) {

		focus_toplevel(toplevel);
		wlr_seat_pointer_notify_enter(seat, surface, sx, sy);
		wlr_seat_pointer_notify_motion(seat, time, sx, sy);
	} else {

		wlr_seat_pointer_clear_focus(seat);
	}

}

void process_cursor_move(struct server_cursor* cursor) {
	struct client_xdg_toplevel *toplevel = cursor->server->current_focus;
	wlr_scene_node_set_position(&toplevel->scene_tree->node,
		cursor->wlr_cursor->x - cursor->grab_x,
		cursor->wlr_cursor->y - cursor->grab_y);
}

void reset_cursor_mode(struct server_cursor* cursor) {
	cursor->cursor_mode = CURSOR_NORMAL;
	wlr_cursor_set_xcursor(cursor->wlr_cursor, cursor->cursor_manager, "left_ptr");

}

void begin_interactive(struct client_xdg_toplevel *toplevel, enum cursor_mode mode, uint32_t edges) {
	/* This function sets up an interactive move or resize operation, where the
	 * compositor stops propegating pointer events to clients and instead
	 * consumes them itself, to move or resize windows. */
	struct server *server = toplevel->server;
	struct server_cursor* cursor = server->cursor;

	server->current_focus = toplevel;
	cursor->cursor_mode = mode;

	if (mode == CURSOR_MOVE) {
		wlr_cursor_set_xcursor(cursor->wlr_cursor, cursor->cursor_manager, "grabbing");
		cursor->grab_x = cursor->wlr_cursor->x - toplevel->scene_tree->node.x;
		cursor->grab_y = cursor->wlr_cursor->y - toplevel->scene_tree->node.y;
	} else if (mode == CURSOR_RESIZE){
		struct wlr_box geo_box = toplevel->xdg_toplevel->base->geometry;

		double border_x = (toplevel->scene_tree->node.x + geo_box.x) +
			((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);
		double border_y = (toplevel->scene_tree->node.y + geo_box.y) +
			((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);
		cursor->grab_x = cursor->wlr_cursor->x - border_x;
		cursor->grab_y = cursor->wlr_cursor->y - border_y;

		toplevel->grab_box = geo_box;
		toplevel->grab_box.x += toplevel->scene_tree->node.x;
		toplevel->grab_box.y += toplevel->scene_tree->node.y;

		toplevel->resize_edges = edges;
	}
}

void process_cursor_resize(struct server_cursor* cursor) {
	struct server* server = cursor->server;
	struct client_xdg_toplevel *toplevel = server->current_focus;

	double border_x = server->cursor->wlr_cursor->x - server->cursor->grab_x;
	double border_y = server->cursor->wlr_cursor->y - server->cursor->grab_y;
	int new_left = toplevel->grab_box.x;
	int new_right = toplevel->grab_box.x + toplevel->grab_box.width;
	int new_top = toplevel->grab_box.y;
	int new_bottom = toplevel->grab_box.y + toplevel->grab_box.height;

	if (toplevel->resize_edges & WLR_EDGE_TOP) {
		new_top = border_y;
		if (new_top >= new_bottom) {
			new_top = new_bottom - 1;
		}
	} else if (toplevel->resize_edges & WLR_EDGE_BOTTOM) {
		new_bottom = border_y;
		if (new_bottom <= new_top) {
			new_bottom = new_top + 1;
		}
	}
	if (toplevel->resize_edges & WLR_EDGE_LEFT) {
		new_left = border_x;
		if (new_left >= new_right) {
			new_left = new_right - 1;
		}
	} else if (toplevel->resize_edges & WLR_EDGE_RIGHT) {
		new_right = border_x;
		if (new_right <= new_left) {
			new_right = new_left + 1;
		}
	}

	struct wlr_box *geo_box = &toplevel->xdg_toplevel->base->geometry;
	//wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, new_left - geo_box->x, new_top - geo_box->y);
	wlr_scene_node_set_position(&toplevel->scene_tree->node,
	                            new_left - geo_box->x, new_top - geo_box->y);

	int new_width = new_right - new_left;
	int new_height = new_bottom - new_top;
	wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, new_width, new_height);
}

uint32_t cursor_edges_for_toplevel(struct client_xdg_toplevel *toplevel,
                                   struct server_cursor *cursor) {
	const int margin = 10;

	struct wlr_box *box = toplevel->box;

	double cx = cursor->wlr_cursor->x - toplevel->scene_tree->node.x;
	double cy = cursor->wlr_cursor->y - toplevel->scene_tree->node.y;

	uint32_t edges = 0;

	if (cx < margin) edges |= WLR_EDGE_LEFT;
	if (cx > box->width - margin) edges |= WLR_EDGE_RIGHT;
	if (cy < margin) edges |= WLR_EDGE_TOP;
	if (cy > box->height - margin) edges |= WLR_EDGE_BOTTOM;

	return edges;
}
