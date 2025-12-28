#ifndef CURSOR_H
#define CURSOR_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_keyboard.h>

struct client_xdg_toplevel;

enum cursor_mode {
	CURSOR_PRESSED,
	CURSOR_MOVE,
	CURSOR_RESIZE,
	CURSOR_NORMAL
};

extern struct server* server;

struct server_cursor {
	struct server* server;

	struct wlr_cursor *wlr_cursor;
	struct wlr_xcursor_manager* cursor_manager;

	struct wlr_pointer_button_event* event;

	enum cursor_mode cursor_mode;
	double grab_x, grab_y;
	//listeners
	struct wl_listener cursor_motion;
	struct wl_listener cursor_motion_absolute;
	struct wl_listener cursor_button;
	struct wl_listener cursor_axis;
	struct wl_listener cursor_frame;


};

void server_new_pointer(struct server* server, struct wlr_input_device* device);



void server_cursor_init(struct server_cursor* cursor);

void server_cursor_destroy(struct server_cursor* cursor);

void server_cursor_motion(struct wl_listener* listener, void* data);

void server_cursor_motion_absolute(struct wl_listener* listener, void* data);

void server_cursor_button(struct wl_listener* listener, void* data);

void server_cursor_axis(struct wl_listener* listener, void* data);

void server_cursor_frame(struct wl_listener* listener, void* data);

void process_cursor_motion(struct server_cursor* cursor, uint32_t time);

void process_cursor_move(struct server_cursor* cursor);

void process_cursor_resize(struct server_cursor* cursor);

void reset_cursor_mode(struct server_cursor* cursor);

void begin_interactive(struct client_xdg_toplevel *toplevel, enum cursor_mode mode, uint32_t edges);
#endif //CURSOR_H
