#ifndef INPUT_H
#define INPUT_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_output.h>

extern struct server* server;



struct server_keyboard {
	struct wl_list link;
	struct server *server;
	struct wlr_keyboard *wlr_keyboard;

	struct wl_listener modifiers;
	struct wl_listener key;
	struct wl_listener destroy;
};

void server_new_input(struct wl_listener* listener, void* data);

void server_new_keyboard(struct server* server, struct wlr_input_device* device);


void keyboard_handle_modifiers(struct wl_listener* listener, void* data);
void keyboard_handle_key(struct wl_listener* listener, void* data);
void keyboard_handle_destroy(struct wl_listener* listener, void* data);
static bool handle_keybinding(struct server *server, xkb_keysym_t sym);
#endif //INPUT_H
