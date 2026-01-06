#include "include/server.h"

#include <wlr/render/allocator.h>

#include "include/input/input.h"
#include "include/output.h"
#include "include/input/cursor.h"
#include "include/clients/client.h"

bool server_init(struct server* server) {
	wlr_log_init(WLR_DEBUG, NULL);
	//creating a wayland display
	server->wl_display = wl_display_create();
	if (!server->wl_display) {
		wlr_log(WLR_ERROR, "Failed to create wl_display");
	}

	server->wl_event_loop = wl_display_get_event_loop(server->wl_display);
	if (!server->wl_event_loop) {
		wlr_log(WLR_ERROR, "Failed to get wl_event_loop");
	}

	server->wlr_session = wlr_session_create(server->wl_event_loop);
	if (!server->wlr_session) {
		wlr_log(WLR_INFO, "Failed to create wlr_session");
	}
	//
	server->backend = wlr_backend_autocreate(server->wl_event_loop, &server->wlr_session);
	if (!server->backend) {
		wlr_log(WLR_ERROR, "Failed to create backend");
	}
	//
	server->renderer = wlr_renderer_autocreate(server->backend);
	if (!server->renderer) {
		wlr_log(WLR_ERROR, "Failed to create wlr_renderer");
	}
	//
	server->allocator = wlr_allocator_autocreate(server->backend, server->renderer);
	if (!server->allocator) {
		wlr_log(WLR_ERROR, "Failed to create wlr_allocator");
	}

	server->compositor = wlr_compositor_create(server->wl_display, 5, server->renderer);
	if (!server->compositor) {
		wlr_log(WLR_ERROR, "Failed to create wlr_compositor");
	}
	wlr_subcompositor_create(server->wl_display);
	wlr_data_device_manager_create(server->wl_display);
	wlr_renderer_init_wl_display(server->renderer, server->wl_display);

	server->scene = wlr_scene_create();
	server->output_layout = wlr_output_layout_create(server->wl_display);

	server->scene_layout = wlr_scene_attach_output_layout(server->scene, server->output_layout);


	wl_list_init(&server->outputs);
	server->new_output.notify = server_new_output;
	wl_signal_add(&server->backend->events.new_output, &server->new_output);


	//trees

	wl_list_init(&server->clients);
	wl_list_init(&server->shell_layers);

	server->xdg_shell = wlr_xdg_shell_create(server->wl_display, 3);
	server->layer_shell = wlr_layer_shell_v1_create(server->wl_display, 1);

	if (!server->layer_shell) {
		wlr_log(WLR_ERROR, "Failed to create wlr_layer_shell");
	}
	else {
		wlr_log(WLR_INFO, "Created wlr_layer_shell");
	}

	//input
	wl_list_init(&server->keyboards);

	server->new_input.notify = server_new_input; // <---- там проверка и
	wl_signal_add(&server->backend->events.new_input, &server->new_input);

	//подвязываем сервер к курсору
	server->cursor = calloc(1, sizeof(struct server_cursor));
	server->cursor->server = server;
	server_cursor_init(server->cursor);

	server->seat = wlr_seat_create(server->wl_display, "seat0");

	server->socket = wl_display_add_socket_auto(server->wl_display);

	if (!server->socket) {
		wlr_backend_destroy(server->backend);
		return false;
	}
	wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s",
			server->socket);

	return true;

}

bool server_start(struct server* server) {
	wlr_log(WLR_INFO, "Starting backend on wayland display '%s'",
			server->socket);

	if (!wlr_backend_start(server->backend)) {
		wlr_log(WLR_ERROR, "Failed to start backend");
		wlr_backend_destroy(server->backend);
		return false;
	}

	server->new_xdg_toplevel.notify = client_new_xdg_toplevel;
	wl_signal_add(&server->xdg_shell->events.new_toplevel, &server->new_xdg_toplevel);
	server->new_xdg_popup.notify = client_new_xdg_popup;
	wl_signal_add(&server->xdg_shell->events.new_popup, &server->new_xdg_popup);

	//server->new_layer_surface.notify = client_new_layer_surface;
	//wl_signal_add(&server->layer_shell->events.new_surface, &server->new_layer_surface);

	server->current_focus = NULL;

	return true;

}
void server_run(struct server* server) {
	wlr_log(WLR_INFO, "Running compositor on wayland display '%s'",
			server->socket);
	wl_display_run(server->wl_display);
}



void server_destroy(struct server* server) {
	server_cursor_destroy(server->cursor);

	//clear listeners
	wl_list_remove(&server->new_input.link);
	wl_list_remove(&server->new_output.link);
	wl_list_remove(&server->new_xdg_toplevel.link);
	wl_list_remove(&server->new_xdg_popup.link);
	//wl_list_remove(&server->new_layer_surface.link);

	wl_list_remove(&server->keyboards);
	//wlr_seat_destroy(server->seat);

	wlr_allocator_destroy(server->allocator);
	wlr_renderer_destroy(server->renderer);
	wlr_backend_destroy(server->backend);

	wl_display_destroy(server->wl_display);

	//не забыть удалить курсоры листы и прочую
}

void create_decoration(struct wl_listener* listener, void* data) {
	struct wlr_xdg_toplevel_decoration_v1* d = data;
	struct client_xdg_toplevel* toplevel = d->toplevel->base->data;
}




