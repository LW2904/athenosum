#ifndef OSU_BEATMAP_H
#define OSU_BEATMAP_H

#include "athenosum/common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include <inttypes.h>
#include <sys/types.h>

#define OSU_RNG_ROUNDS 50
#define OSU_RNG_BOUNDARY 0.5

#define OSU_TYPE_SLIDER 128

struct osu_beatmap_meta {
	int set_id;
	int map_id;
	int columns;
	char title[256];
	char artist[256];
	char version[256];
};

struct osu_hitpoint {
	int column;
	int end_time;
	int start_time;
};

struct osu_action {
	int down;
	char key;
	int time;
};

/**
 * Searches for a beatmap in DEFAULT_OSU_PATH + base given a part of the
 * file name, and stores the absolute path to it in *map.
 * Returns the length of the path stored, or zero on failure.
 */
size_t find_beatmap(char *base, char *partial, char **map);

/**
 * Parse a beatmap file (*.osu) into an array of hitpoint structs pointed to by 
 * **points and a metadata struct.
 * Returns the number of points parsed and stored.
 */
size_t parse_beatmap(char *file, struct osu_hitpoint **points,
		     struct osu_beatmap_meta **meta);

/**
 * Parses a total of count hitpoints from **points into **actions.
 * Returns the number of actions parsed and stored, which should be count * 2.
 */
int parse_hitpoints(size_t count, size_t columns, struct osu_hitpoint **points,
		    struct osu_action **actions);

/**
 * Sort the array of actions given through **actions by time.
 * Returns nonzero on failure.
 */
int sort_actions(int count, struct osu_action **actions);

/**
 * Add a randomized delay of magnitude level to the hitpoints.
 */
void humanize_hitpoints(int total, struct osu_hitpoint **points, int level);

#endif /* OSU_BEATMAP_H */
