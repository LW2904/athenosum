#include "athenosum/game.h"
#include "athenosum/beatmap.h"

#include <stdio.h>

int main()
{
	char *default_path;
	if (!(get_osu_path(&default_path))) {
		printf("failed fetching osu! path\n");
	} else printf("got osu! path: '%s'\n", default_path);

	char *map;
	if (!(find_beatmap(default_path, "raja - the li", &map))) {
		printf("couldn't find beatmap\n");
	} else printf("found beatmap at '%s'\n", map);

	return 0;
}