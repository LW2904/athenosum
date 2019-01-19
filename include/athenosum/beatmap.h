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

#define OSU_BMP_MAX_LINE_LEN 256

enum osu_type_values {
	OSU_TYPE_CIRCLE = 1,
	OSU_TYPE_SLIDER = 2,
	OSU_TYPE_COMBO = 4,
	OSU_TYPE_SPINNER = 8,
	OSU_TYPE_HOLD = 128,
};

enum osu_slider_type {
	OSU_SLIDER_LINEAR = 0,
	OSU_SLIDER_PERFECT = 1,
	OSU_SLIDER_BEZIER = 2,
	OSU_SLIDER_UNKNOWN = 3
};

struct osu_beatmap_meta {
	int set_id;
	int map_id;
	char title[256];
	char artist[256];
	char version[256];

	/* Difficulty information */
	int circle_size;
};

struct osu_hitpoint {
	int column;
	int end_time;
	int start_time;
};

struct osu_hitobject {
	/* Common properties */
	int x;
	int y;
	int time;
	int type;

	/* Only for osu!standard sliders */
	int slider_repeat;
	int slider_pixel_length;
	enum osu_slider_type slider_type;
	struct osu_slider_point **slider_curve_points;

	/* Only for osu!standard spinners */
	int spinner_end_time;

	/* Only for osu!mania hold notes */
	int hold_end_time;
};

struct osu_slider_point {
	int x;
	int y;
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
size_t parse_beatmap_legacy(char *file, struct osu_hitpoint **points,
			    struct osu_beatmap_meta **meta);

size_t parse_beatmap(char *file, struct osu_hitobject **objects,
		     struct osu_beatmap_meta **meta);

/**
 * Parses a total of count hitpoints from **points into **actions.
 * Returns the number of actions parsed and stored, which should be count * 2.
 */
size_t parse_hitpoints(size_t count, size_t columns,
		       struct osu_hitpoint **points,
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
