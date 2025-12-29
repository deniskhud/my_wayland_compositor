#include "parser.h"
#include "src/include/server.h"

bool config_data_load(const char* filename) {
	FILE* fp = fopen(filename, "r");
	if (!fp) {
		perror("error opening config file");

		return false;
	}

	char line[128];

	while (fgets(line, sizeof(line), fp)) {
		line[strcspn(line, "\n")] = 0;
		printf("%s", line);
	}

	fclose(fp);

	return true;
}




