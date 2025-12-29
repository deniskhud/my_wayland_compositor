#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct keybinding {
	uint32_t keycode;
	uint32_t modifiers;


	struct keybinding *next;

} kb;

typedef struct map {
	kb *keys;
} mp;

bool config_data_load(const char* filename);
bool config_data_destroy(const struct server* server);

void parse_line(const char* line);

#endif //PARSER_H
