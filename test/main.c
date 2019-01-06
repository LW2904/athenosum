#include "athenosum/beatmap.h"

#include <stdio.h>

int main()
{
	char *map = NULL;
	size_t read = find_beatmap(OSU_DEFAULT_PATH, "raja", &map);

	printf("%s\n", map);

	free(map);

	return 0;
}