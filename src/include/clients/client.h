#ifndef CLIENT_H
#define CLIENT_H

#include <wlr/types/wlr_xdg_shell.h>
#include <stdlib.h>
enum WINDOW_MODE {
	WINDOW_FLOATING,
	WINDOW_TILING,
	WINDOW_FULLSCREEN,
	WINDOW_HOLDING,
};

struct client_xdg_toplevel {

	struct wlr_xdg_toplevel *xdg_toplevel;
	struct wl_list link;
	struct server *server;
	struct wlr_scene_tree *scene_tree;
	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener commit;
	struct wl_listener destroy;

	//window size / coords
	struct wlr_box* box;
	struct wlr_box grab_box;
	uint32_t resize_edges;

	struct wl_listener request_move;
	struct wl_listener request_resize;
	struct wl_listener request_maximize;
	struct wl_listener request_fullscreen;
	struct wl_listener request_minimize;
	struct wl_listener set_title;
	struct wl_listener show_window_menu;
	struct wl_listener set_app_id;
	struct wl_listener set_parent;

	enum WINDOW_MODE mode;
	//tree
	struct node* leaf;
};

struct client_xdg_popup {
	struct wlr_xdg_popup *xdg_popup;
	struct wlr_scene_tree *scene_tree;

	struct wl_listener commit;
	struct wl_listener destroy;
	struct wl_listener reposition;
};
void client_popup_commit(struct wl_listener *listener, void *data);

void client_new_xdg_toplevel(struct wl_listener* listener, void* data);
void client_new_xdg_popup(struct wl_listener* listener, void* data);

void handle_toplevel_map(struct wl_listener* listener, void* data);
void handle_toplevel_unmap(struct wl_listener* listener, void* data);
void handle_toplevel_commit(struct wl_listener* listener, void* data);
void handle_toplevel_request_move(struct wl_listener* listener, void* data);
void handle_toplevel_request_resize(struct wl_listener* listener, void* data);
void handle_toplevel_request_fullscreen(struct wl_listener* listener, void* data);
void handle_toplevel_request_maximize(struct wl_listener* listener, void* data);

void handle_toplevel_request_minimize(struct wl_listener* listener, void* data);
void handle_toplevel_set_app_id(struct wl_listener* listener, void* data);
void handle_toplevel_set_title(struct wl_listener* listener, void* data);
void handle_toplevel_set_parent(struct wl_listener* listener, void* data);
void handle_toplevel_show_window_menu(struct wl_listener* listener, void* data);


void client_xdg_toplevel_destroy(struct wl_listener* listener, void* data);
void client_xdg_popup_destroy(struct wl_listener* listener, void* data);
void client_popup_reposition(struct wl_listener* listener, void* data);

//focus in window under a cursor
void focus_toplevel(struct client_xdg_toplevel* toplevel);

struct client_xdg_toplevel *desktop_toplevel_at(
		struct server *server, double lx, double ly,
		struct wlr_surface **surface, double *sx, double *sy);


void set_focus(struct client_xdg_toplevel* toplevel);

struct wlr_box* client_get_geometry(struct client_xdg_toplevel* toplevel);

void client_set_size(struct client_xdg_toplevel* toplevel, uint32_t width, uint32_t height);

void arrange_windows(struct server* server);


void client_set_floating_mode(struct client_xdg_toplevel* toplevel);
void client_set_tiling_mode(struct client_xdg_toplevel* toplevel);
void client_set_holding_mode(struct client_xdg_toplevel* toplevel);
#endif //CLIENT_H
