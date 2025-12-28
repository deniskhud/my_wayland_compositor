#include "../include/input.h"

#include <unistd.h>

#include "../include/server.h"
#include "../include/cursor.h"
#include "../include/output.h"
#include "../include/client.h"
void server_new_input(struct wl_listener* listener, void* data) {
	struct server* server = wl_container_of(listener, server, new_input);

	struct wlr_input_device* device = data;

	switch (device->type) {
		case WLR_INPUT_DEVICE_KEYBOARD:
			server_new_keyboard(server, device);
			break;
		case WLR_INPUT_DEVICE_POINTER:
			server_new_pointer(server, device);
			break;
		case WLR_INPUT_DEVICE_TOUCH:
			server_new_pointer(server, device);
		default:
			break;
	}
	uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
	if (!wl_list_empty(&server->keyboards)) {
		caps |= WL_SEAT_CAPABILITY_KEYBOARD;
	}
	wlr_seat_set_capabilities(server->seat, caps);
}

void server_new_keyboard(struct server* server, struct wlr_input_device* device) {
	struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);

	struct server_keyboard *keyboard = calloc(1, sizeof(struct server_keyboard));
	keyboard->server = server;
	keyboard->wlr_keyboard = wlr_keyboard;


	struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	struct xkb_keymap *keymap = xkb_keymap_new_from_names(context, NULL,
		XKB_KEYMAP_COMPILE_NO_FLAGS);

	wlr_keyboard_set_keymap(keyboard->wlr_keyboard, keymap);
	xkb_keymap_unref(keymap);
	xkb_context_unref(context);
	wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);


	keyboard->modifiers.notify = keyboard_handle_modifiers;
	wl_signal_add(&wlr_keyboard->events.modifiers, &keyboard->modifiers);
	keyboard->key.notify = keyboard_handle_key;
	wl_signal_add(&wlr_keyboard->events.key, &keyboard->key);
	keyboard->destroy.notify = keyboard_handle_destroy;
	wl_signal_add(&device->events.destroy, &keyboard->destroy);

	//set a keyboard to the seat
	wlr_seat_set_keyboard(server->seat, keyboard->wlr_keyboard);


	wl_list_insert(&server->keyboards, &keyboard->link);
	wlr_log(WLR_INFO, "new keyboard added");
}



void keyboard_handle_modifiers(struct wl_listener* listener, void* data) {

	struct server_keyboard *keyboard =
		wl_container_of(listener, keyboard, modifiers);

	wlr_seat_set_keyboard(keyboard->server->seat, keyboard->wlr_keyboard);
	/* Send modifiers to the client. */
	wlr_seat_keyboard_notify_modifiers(keyboard->server->seat,
		&keyboard->wlr_keyboard->modifiers);
}
void keyboard_handle_key(struct wl_listener* listener, void* data) {
	struct server_keyboard *keyboard =
		wl_container_of(listener, keyboard, key);
	struct server *server = keyboard->server;
	struct wlr_keyboard_key_event *event = data;

	struct wlr_seat *seat = server->seat;

	/* Translate libinput keycode -> xkbcommon */
	uint32_t keycode = event->keycode + 8;
	if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		wlr_log(WLR_INFO, "The key %d has been pressed ", keycode);
	}

	/* Get a list of keysyms based on the keymap for this keyboard */
	const xkb_keysym_t *syms;
	int nsyms = xkb_state_key_get_syms(
			keyboard->wlr_keyboard->xkb_state, keycode, &syms);

	bool handled = false;
	uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->wlr_keyboard);

	if ((modifiers & WLR_MODIFIER_LOGO )
		&& event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		for (int i = 0; i < nsyms; i++) {
			if (syms[i] == XKB_KEY_c) {
				if (server->current_focus->mode != WINDOW_FLOATING) {
					client_set_floating_mode(server->current_focus);
				}
				else {
					client_set_tiling_mode(server->current_focus);
				}

			}
		}
	}
	if ((modifiers & WLR_MODIFIER_CTRL) && event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		struct wlr_pointer_button_event* e = server->cursor->event;
		if (e->state == WL_POINTER_BUTTON_STATE_PRESSED) {
			client_set_holding_mode(server->current_focus);
		}
	}
	if ((modifiers & WLR_MODIFIER_ALT) &&
			event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {

		for (int i = 0; i < nsyms; i++) {
			handled = handle_keybinding(server, syms[i]);
		}
			}

	if (!handled) {
		/* Otherwise, we pass it along to the client. */
		wlr_seat_set_keyboard(seat, keyboard->wlr_keyboard);
		wlr_seat_keyboard_notify_key(seat, event->time_msec,
			event->keycode, event->state);
	}
}
void keyboard_handle_destroy(struct wl_listener* listener, void* data) {
	wlr_log(WLR_INFO, "Destroying keyboard");
	struct server_keyboard* keyboard = wl_container_of(listener, keyboard, destroy);
	wl_list_remove(&keyboard->modifiers.link);
	wl_list_remove(&keyboard->key.link);
	wl_list_remove(&keyboard->link);
	wl_list_remove(&keyboard->destroy.link);
	free(keyboard);
	wlr_log(WLR_INFO, "Destroyed keyboard");

}

static bool handle_keybinding(struct server *server, xkb_keysym_t sym) {

	switch (sym) {
		case XKB_KEY_Escape:
			wl_display_terminate(server->wl_display);

			break;
		case XKB_KEY_F1:

			if (wl_list_length(&server->clients) < 2) {
				break;
			}
			struct client_xdg_toplevel *next_toplevel =
				wl_container_of(server->clients.prev, next_toplevel, link);
			focus_toplevel(next_toplevel);
			break;
		case XKB_KEY_q: {
			//check if any client exist (if not check it will crash)
			if (wl_list_empty(&server->clients)) break;
			// otherwise, send close to client
			struct client_xdg_toplevel *toplevel = wl_container_of(server->clients.next, toplevel, link);
			if (toplevel->xdg_toplevel != NULL) wlr_xdg_toplevel_send_close(toplevel->xdg_toplevel);
		}
			break;

		case XKB_KEY_F2:
		case XKB_KEY_w:
			run_window("firefox");
		case XKB_KEY_e:
				run_window("nautilus");
			break;
		case XKB_KEY_f:
			run_window("foot");
			break;

		case XKB_KEY_d:
			if (wl_list_empty(&server->clients)) break;

			uint32_t result = wlr_xdg_toplevel_set_maximized(server->current_focus->xdg_toplevel, true);
			wlr_log(WLR_INFO, "Fullscreen set to true");
		case XKB_KEY_h:
			run_window("waybar");
		default:
			return false;
	}
	return true;
}

static void run_window(const char* name) {
	pid_t pid = fork();

	if (pid == 0) {
		// child
		setsid();

		setenv("WAYLAND_DISPLAY", "wayland-0", 1);
		// setenv("XDG_RUNTIME_DIR", "/run/user/1000", 1);

		char *const argv[] = {
			(char *)name,
			NULL
		};
		execvp(name, argv);
		perror("execvp");
		_exit(1);
	}
	else if (pid < 0) {
		perror("fork");
	}
}