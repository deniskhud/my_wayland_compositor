#ifndef ACTION_H
#define ACTION_H
#include <stdint.h>
#include "server.h"

enum action_type {
	CLOSE_WINDOW,
	FOCUS_LEFT,
	FOCUS_RIGHT,
	OPEN_TERMINAL,
	UNKNOWN
} action_type;

struct bilding {
	enum action_type type;
	uint32_t modifiers;
	uint32_t keycode;
	struct wl_list link;
};

uint32_t parse_modifier(const char* token) {
	if (strcmp(token, "ALT") == 0) return WLR_MODIFIER_ALT;
	if (strcmp(token, "CTRL") == 0) return WLR_MODIFIER_CTRL;
	if (strcmp(token, "SHIFT") == 0) return WLR_MODIFIER_SHIFT;
	if (strcmp(token, "SUPER") == 0) return WLR_MODIFIER_LOGO;

	return 0;
}

enum action_type parse_keycode(const char* token) {
	if (strcmp(token, "close_window") == 0) return CLOSE_WINDOW;
	if (strcmp(token, "focus_left") == 0) return FOCUS_LEFT;
	if (strcmp(token, "focus_right") == 0) return FOCUS_RIGHT;
	if (strcmp(token, "open_terminal") == 0) return OPEN_TERMINAL;
	return UNKNOWN;
}







#endif //ACTION_H
