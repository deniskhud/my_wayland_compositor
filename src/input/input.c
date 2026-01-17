#include "../include/input/input.h"

#include <unistd.h>

#include "../include/server.h"
#include "../include/input/cursor.h"
#include "../include/output.h"
#include "../include/clients/client.h"
#include "src/include/seat.h"

void server_new_input(struct wl_listener* listener, void* data) {
	struct server* server = wl_container_of(listener, server, new_input);

	struct wlr_input_device* device = data;
	struct seat* seat = server->seat;
	switch (device->type) {
		case WLR_INPUT_DEVICE_KEYBOARD:
			seat->keyboard = server_new_keyboard(seat, device);
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
	if (!wl_list_empty(&seat->keyboards)) {
		caps |= WL_SEAT_CAPABILITY_KEYBOARD;
	}
	wlr_seat_set_capabilities(server->seat->wlr_seat, caps);
}


struct server_keyboard* server_new_keyboard(struct seat* seat, struct wlr_input_device* device) {
	struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);

	struct server_keyboard *keyboard = calloc(1, sizeof(struct server_keyboard));
	keyboard->seat = seat;
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
	/*keyboard->destroy.notify = keyboard_handle_destroy;
	wl_signal_add(&device->events.destroy, &keyboard->destroy);*/

	//set a keyboard to the seat
	wlr_seat_set_keyboard(seat->wlr_seat, keyboard->wlr_keyboard);


	wl_list_insert(&seat->keyboards, &keyboard->link);
	wlr_log(WLR_INFO, "new keyboard added");

	return keyboard;
}


void keyboard_handle_modifiers(struct wl_listener* listener, void* data) {

	struct server_keyboard *keyboard =
		wl_container_of(listener, keyboard, modifiers);

	wlr_seat_set_keyboard(keyboard->seat->wlr_seat, keyboard->wlr_keyboard);
	/* Send modifiers to the client. */
	wlr_seat_keyboard_notify_modifiers(keyboard->seat->wlr_seat,
		&keyboard->wlr_keyboard->modifiers);
}
void keyboard_handle_key(struct wl_listener* listener, void* data) {
	struct server_keyboard *keyboard =
		wl_container_of(listener, keyboard, key);
	struct seat *seat = keyboard->seat;
	struct wlr_keyboard_key_event *event = data;
	struct server *server = seat->server;
	struct wlr_seat *wlr_seat = seat->wlr_seat;

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

	//TODO set current focus to seat struct
	if ((modifiers & WLR_MODIFIER_LOGO )
		&& event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		for (int i = 0; i < nsyms; i++) {
			if (syms[i] == XKB_KEY_c) {
				if (seat->current_focus->mode != WINDOW_FLOATING) {
					client_set_floating_mode(seat->current_focus);
				}
				else {
					client_set_tiling_mode(seat->current_focus);
				}

			}
		}
	}
	if ((modifiers & WLR_MODIFIER_CTRL) && event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		struct wlr_pointer_button_event* e = server->seat->cursor->event;
		if (e->state == WL_POINTER_BUTTON_STATE_PRESSED) {
			if (seat->current_focus->mode == WINDOW_TILING) {
				client_set_holding_mode(seat->current_focus);
			}

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
		wlr_seat_set_keyboard(wlr_seat, keyboard->wlr_keyboard);
		wlr_seat_keyboard_notify_key(wlr_seat, event->time_msec,
			event->keycode, event->state);
	}
}


void keyboard_destroy(struct server_keyboard* keyboard){
	if (keyboard == NULL) return;
	wl_list_remove(&keyboard->modifiers.link);
	wl_list_remove(&keyboard->key.link);
	wl_list_remove(&keyboard->link);
	//wl_list_remove(&keyboard->destroy.link);
	free(keyboard);
}

static bool handle_keybinding(struct server *server, xkb_keysym_t sym) {
	struct seat* seat = server->seat;
	switch (sym) {
		case XKB_KEY_Escape:
			wl_display_terminate(server->wl_display);

			break;

		case XKB_KEY_q: {
			//check if any client exist (if not check it will crash)
			if (wl_list_empty(&server->clients)) {
				seat->current_focus = NULL;
				break;
			}
			// otherwise, send close to client
			struct client_xdg_toplevel *toplevel = seat->current_focus;
			if (seat->current_focus != NULL) wlr_xdg_toplevel_send_close(toplevel->xdg_toplevel);

		}
			break;

		case XKB_KEY_F2:
		case XKB_KEY_w:
			run_window("firefox");
		case XKB_KEY_e:
				run_window("dolphin");
			break;
		case XKB_KEY_f:
			run_window("foot");
			break;

		case XKB_KEY_d:
			if (wl_list_empty(&server->clients)) break;

			uint32_t result = wlr_xdg_toplevel_set_maximized(seat->current_focus->xdg_toplevel, true);
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