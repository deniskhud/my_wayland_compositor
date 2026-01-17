#ifndef SEAT_H
#define SEAT_H
#include <wayland-util.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_primary_selection.h>
struct seat {
	struct server* server;
	struct wlr_seat* wlr_seat;
	struct client_xdg_toplevel* current_focus;

	struct {
		struct client_xdg_toplevel* client;
		//coords
		double x, y;

	} grab_area;

	struct server_cursor* cursor;
	struct server_keyboard* keyboard;

	struct wl_list keyboards;
	struct wl_list link;

	struct wl_listener request_set_primary_selection;
};

struct seat* seat_create(struct server* server, char* name);
void seat_request_set_primary_selection(struct wl_listener *listener, void *data);
void seat_destroy(struct seat* seat);
#endif //SEAT_H
