#include "include/seat.h"
#include "include/input/cursor.h"
#include "include/server.h"
#include "include/input/input.h"

struct seat* seat_create(struct server* server, char* name) {
	struct wl_display* display = server->wl_display;

	struct seat* seat = calloc(1, sizeof(struct seat));
	seat->server = server;

	//create a seat and add a struct to seats list
	seat->wlr_seat = wlr_seat_create(display, name);
	wl_list_insert(&server->seats, &seat->link);


	wl_list_init(&seat->keyboards);


	seat->cursor = server_cursor_init(seat->server);



	seat->request_set_primary_selection.notify = seat_request_set_primary_selection;
	wl_signal_add(&seat->wlr_seat->events.request_set_primary_selection,
		&seat->request_set_primary_selection);

	seat->current_focus = NULL;

	return seat;
}

void seat_request_set_primary_selection(struct wl_listener *listener, void *data) {
	struct seat *seat = wl_container_of(listener, seat, request_set_primary_selection);
	struct wlr_seat_request_set_primary_selection_event *event = data;
	wlr_seat_set_primary_selection(seat->wlr_seat, event->source, event->serial);
}

void seat_destroy(struct seat* seat) {
	wl_list_remove(&seat->request_set_primary_selection.link);
	wl_list_remove(&seat->keyboards);
	wl_list_remove(&seat->link);
	server_cursor_destroy(seat->cursor);
	keyboard_destroy(seat->keyboard);
	free(seat);
}
