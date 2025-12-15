#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>
#include <wlr/backend/session.h>
#include <assert.h>
#include <wlr/types/wlr_layer_shell_v1.h>

struct server {
	struct wl_display *wl_display;
	struct wlr_backend *backend;
	struct wl_event_loop* wl_event_loop;
	struct wlr_session *wlr_session;

	struct wlr_renderer *renderer;
	struct wlr_allocator *allocator;

	struct wlr_compositor *compositor;


	//проба окон

	//client
	struct wl_list clients;
	struct wlr_xdg_shell *xdg_shell;

	struct wl_list layers;
	struct wlr_layer_shell_v1 *layer_shell;

	struct wl_listener new_xdg_toplevel;
	struct wl_listener new_xdg_popup;

	struct wl_listener new_layer_surface;
	//struct wl_list toplevels;

	struct client_xdg_toplevel* current_focus;


	const char* socket;

	//output
	struct server_output *output;
	//наши моники
	struct wl_list outputs;
	struct wl_listener new_output;
	struct wlr_output_layout *output_layout;
	struct wlr_scene* scene;
	struct wlr_scene_output_layout* scene_layout;


	//inputs
	struct wlr_seat* seat;

	struct wl_list keyboards;
	struct wl_list pointers;

	struct wl_listener new_input;

	struct server_keyboard* keyboard;

	struct server_cursor *cursor;

	struct node* root;

};



bool server_init(struct server* server);
bool server_start(struct server* server);
void server_run(struct server* server);

void server_destroy(struct server* server);


#endif //SERVER_H
