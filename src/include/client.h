#ifndef CLIENT_H
#define CLIENT_H

#include <wlr/types/wlr_xdg_shell.h>
#include <stdlib.h>

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

	struct wl_listener request_move;
	struct wl_listener request_resize;
	struct wl_listener request_maximize;
	struct wl_listener request_fullscreen;

	//tree
	struct node* leaf;
};

struct client_xdg_popup {
	struct wlr_xdg_popup *xdg_popup;
	struct wl_listener init;
	struct wl_listener destroy;
};

void client_new_xdg_toplevel(struct wl_listener* listener, void* data);
void client_new_xdg_popup(struct wl_listener* listener, void* data);

void xdg_toplevel_map(struct wl_listener* listener, void* data);
void xdg_toplevel_unmap(struct wl_listener* listener, void* data);
void xdg_toplevel_commit(struct wl_listener* listener, void* data);
void xdg_toplevel_request_move(struct wl_listener* listener, void* data);
void xdg_toplevel_request_resize(struct wl_listener* listener, void* data);
void xdg_toplevel_request_fullscreen(struct wl_listener* listener, void* data);
void xdg_toplevel_request_maximize(struct wl_listener* listener, void* data);


void client_xdg_toplevel_destroy(struct wl_listener* listener, void* data);
void client_xdg_popup_destroy(struct wl_listener* listener, void* data);

struct server_layer_view {
	struct server *server;

	struct wlr_layer_shell_v1 *layer_shell;
};




void focus_toplevel(struct client_xdg_toplevel* toplevel);
struct client_xdg_toplevel *desktop_toplevel_at(
		struct server *server, double lx, double ly,
		struct wlr_surface **surface, double *sx, double *sy);

struct wlr_box* client_get_geometry(struct client_xdg_toplevel* toplevel);

void client_set_size(struct client_xdg_toplevel* toplevel, uint32_t width, uint32_t height);


void arrange_windows(struct server* server);

#endif //CLIENT_H
