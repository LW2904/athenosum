#include "athenosum/beatmap.h"

/**
 * Parses a raw beatmap line into a beatmap_meta struct pointed to by *meta.
 * Returns the number of tokens read.
 */
static int parse_meta_line(char *line, struct osu_beatmap_meta *meta);

/**
 * Parses a key:value set into *meta.
 */
static void parse_meta_token(char *key, char *value,
			     struct osu_beatmap_meta *meta);

/**
 * Parses a raw hitobject line into a hitpoint struct pointed to by *point.
 * Returns the number of tokens read.
 */
static int parse_hitobject_line(char *line, struct osu_beatmap_meta *meta,
				struct osu_hitobject *point);

static int parse_extras_field(char *extras, struct osu_hitobject *object);


/**
 * Populates *start and *end with data from hitpoint *point.
 */
static void hitpoint_to_action(const char *keys, struct osu_hitpoint *point,
			       struct osu_action *start,
			       struct osu_action *end);

/**
 * Returns a randomly generated number in the range of [0, range], while
 * attemting to constrain it outside of a bound(ary) given in percent (]0, 1[),
 * in a given number of rounds.
 */
static int generate_number(int range, int rounds, double bound);

/**
 * Searches for a file or folder in `base`, matching all directory entries
 * against `partial`. The best match is returned through *out_file.
 * Returns the length of the matched path or zero on failure.
 */
static size_t find_partial_file(char *base, char *partial, char **out_file);

/**
 * Given a base, returns the number of concurrent characters which match
 * partial.
 */
static int partial_match(char *base, const char *partial);

const char col_keys[9] = "asdfjkl[";

size_t find_beatmap(char *base, char *partial, char **map)
{
	if (!base || !partial || !map) {
		osu_debug("received null pointer");
		return 0;
	}

	char *folder = NULL;
	size_t folder_len = 0;

	if (!(folder_len = find_partial_file(base, partial, &folder))) {
		osu_debug("couldn't find folder (%s)", partial);
		return 0;
	}

	size_t map_len = 256, base_len = strlen(base);
	*map = malloc(map_len);

	/* Absolute path to our base. */
	strcpy(*map, base);
	/* A.p. to the beatmap folder. */
	strcpy(*map + base_len, folder);
	/* Add a trailing separator and terminating zero. */
	strcpy(*map + base_len + folder_len,
	       (char[2]){ (char)OSU_SEPARATOR, '\0' });

	free(folder);

	char *beatmap = NULL;
	size_t beatmap_len = 0;

	if (!(beatmap_len = find_partial_file(*map, partial, &beatmap))) {
		osu_debug("couldn't find beatmap in %s", *map);
		return 0;
	}

	/* This is now the absolute path to our beatmap. */
	strcpy(*map + base_len + folder_len + 1, beatmap);

	free(beatmap);

	map_len = base_len + folder_len + 1 + beatmap_len;

	/* Change block size to fit what we really need. */
	*map = realloc(*map, map_len + 1);

	/* Verify that the file we found is a beatmap.
	   TODO: This is really crude right now */
	if (strcmp(*map + map_len - 4, ".osu") != 0) {
		osu_debug("%s is probably not a beatmap", *map);

		free(*map);

		return 0;
	}

	return map_len;
}

size_t parse_beatmap(char *file, struct osu_hitobject **objects,
		     struct osu_beatmap_meta **meta)
{
	if (!file || !objects || !meta) {
		osu_debug("received null pointer");
		return 0;
	}

	errno = 0;
	FILE *stream;

	if (!(stream = fopen(file, "r"))) {
		osu_debug("couldn't open file '%s': %d", file, errno);
		return 0;
	}

	*objects = NULL;
	*meta = calloc(1, sizeof(struct osu_beatmap_meta));

	char *line = malloc(OSU_BMP_MAX_LINE_LEN);
	char *section = malloc(OSU_BMP_MAX_LINE_LEN);

	size_t num_parsed = 0;
	struct osu_hitobject cur_object;

	while (fgets(line, OSU_BMP_MAX_LINE_LEN, stream)) {
		if (!(strcmp(section, "[Metadata]\n"))) {
			parse_meta_line(line, *meta);
		} else if (!(strcmp(section, "[Difficulty]\n"))) {
			parse_meta_line(line, *meta);
		} else if (!(strcmp(section, "[HitObjects]\n"))) {
			parse_hitobject_line(line, *meta, &cur_object);

			/* TODO: This is inefficient as all hell */
			*objects = realloc(*objects, ++num_parsed *
						     sizeof(struct osu_hitobject));
			objects[0][num_parsed - 1] = cur_object;
		}

		if (line[0] == '[')
			strcpy(section, line);
	}
}

/* TODO: This function is not thread safe. */
static int parse_meta_line(char *line, struct osu_beatmap_meta *meta)
{
	int i = 0;
	char *ln = strdup(line);
	char *token = NULL, *key = NULL, *value = NULL;

	/* Metadata lines come in key:value pairs */
	token = strtok(ln, ":");
	while (token != NULL) {
		switch (i++) {
		case 0: key = strdup(token);
			break;
		case 1: value = strdup(token);

			parse_meta_token(key, value, meta);

			break;
		default: osu_debug("beatmap line '%s' has unexpected format",
				   line);
			goto exit;
		}

		token = strtok(NULL, ":");
	}

	exit:
	free(ln);
	free(key);
	free(token);
	free(value);

	return i;
}

static void parse_meta_token(char *key, char *value,
			     struct osu_beatmap_meta *meta)
{
	if (!key || !value || !meta) {
		osu_debug("received null pointer");
		return;
	}

	errno = 0;

	/* Always ignore last two characters since .osu files are CRLF by
	   default */
	if (!(strcmp(key, "Title"))) {
		value[strlen(value) - 2] = '\0';

		strcpy(meta->title, value);
	} else if (!(strcmp(key, "Artist"))) {
		value[strlen(value) - 2] = '\0';

		strcpy(meta->artist, value);
	} else if (!(strcmp(key, "Version"))) {
		value[strlen(value) - 2] = '\0';

		strcpy(meta->version, value);
	} else if (!(strcmp(key, "BeatmapID"))) {
		meta->map_id = (int)strtol(value, NULL, 10);
	} else if (!(strcmp(key, "BeatmapSetID"))) {
		meta->set_id = (int)strtol(value, NULL, 10);
	} else if (!(strcmp(key, "CircleSize"))) {
		meta->circle_size = (int)strtol(value, NULL, 10);
	}

	if (errno == ERANGE) {
		osu_debug(
			"something probably went wrong while converting '%s' to long (%d)",
			value, errno);
	}
}

/* TODO: This function is not thread safe. */
static int parse_hitobject_line(char *line, struct osu_beatmap_meta *meta,
				struct osu_hitobject *object)
{
	int secval = 0, end_time = 0, slider = 0, i = 0;
	char *ln = strdup(line), *token = NULL;

	token = strtok(ln, ",");
	while (token != NULL) {
		secval = (unsigned int)strtol(token, NULL, 10);

		switch (i++) {
			/* X */
		case 0: object->x = secval;
			break;
			/* Y */
		case 1: object->y = secval;
			/* Start time */
		case 2: object->time = secval;
			break;
			/* Type */
		case 3: object->type = secval;
			break;
		case 4: /* Hitsound, we don't care */
			break;
			/* Extra string, depends on type */
		case 5: parse_extras_field(token, object);
			break;
		default: osu_debug("hitobject line '%s' has unexpected format",
				   line);
			goto exit;
		}

		token = strtok(NULL, ",");
	}

	exit:
	free(ln);
	free(token);

	return i;
}

static int parse_extras_field(char *extras, struct osu_hitobject *object)
{
	char *extr = strdup(extras), *token = NULL;

	if (object->type & OSU_TYPE_SLIDER) {
		char *slider_type = strtok(extr, "|");

		if (!(strcmp(slider_type, "L"))) {
			object->slider_type = OSU_SLIDER_LINEAR;
		} else if (!(strcmp(slider_type, "P"))) {
			object->slider_type = OSU_SLIDER_PERFECT;
		} else if (!(strcmp(slider_type, "B"))) {
			object->slider_type = OSU_SLIDER_BEZIER;
		} else object->slider_type = OSU_SLIDER_UNKNOWN;

		/* TODO: Implement slider parsing logic. */
	} else if (object->type & OSU_TYPE_SPINNER) {
	}
}

/* TODO: Inefficient as it reallocated memory for every parsed line. Allocate
 * 	 memory in chunks and copy it to adequately sited buffer once done. */
size_t parse_beatmap_legacy(char *file, struct osu_hitpoint **points,
			    struct osu_beatmap_meta **meta)
{
	if (!points || !meta || !file) {
		osu_debug("received null pointer");
		return 0;
	}

	FILE *stream;

	if (!(stream = fopen(file, "r"))) {
		osu_debug("couldn't open file %s", file);
		return 0;
	}

	*points = NULL;
	*meta = calloc(1, sizeof(struct osu_beatmap_meta));

	const size_t line_len = 256;
	char *line = malloc(line_len);

	struct osu_hitpoint cur_point;
	size_t hp_size = sizeof(struct osu_hitpoint), num_parsed = 0;

	char cur_section[128];

	/* TODO: This loop body is all kinds of messed up. */
	while (fgets(line, (int)line_len, stream)) {
		if (!(strcmp(cur_section, "[Metadata]\n"))) {
			parse_meta_line(line, *meta);
		} else if (!(strcmp(cur_section, "[Difficulty]\n"))) {
			parse_meta_line(line, *meta);
		} else if (!(strcmp(cur_section, "[HitObjects]\n"))) {
			parse_hitobject_line(line, meta[0]->circle_size,
					     &cur_point);

			*points = realloc(*points, ++num_parsed * hp_size);
			points[0][num_parsed - 1] = cur_point;
		}

		if (line[0] == '[')
			strcpy(cur_section, line);
	}

	free(line);
	fclose(stream);

	return num_parsed;
}

size_t parse_hitpoints(size_t count, size_t columns,
		       struct osu_hitpoint **points,
		       struct osu_action **actions)
{
	/* Allocate enough memory for all actions at once. */
	*actions = malloc((2 * count) * sizeof(struct osu_action));

	struct osu_hitpoint *cur_point;
	size_t num_actions = 0, i = 0;

	char *key_subset = malloc(columns + 1);
	key_subset[columns] = '\0';

	const size_t col_size = sizeof(col_keys) - 1;
	const size_t subset_offset = (col_size / 2) - (columns / 2);

	memmove(key_subset, col_keys + subset_offset,
		col_size - (subset_offset * 2));

	if (columns % 2) {
		memmove(key_subset + columns / 2 + 1, key_subset + columns / 2,
			columns / 2);
		key_subset[columns / 2] = ' ';
	}

	while (i < count) {
		cur_point = (*points) + i++;

		/* Don't care about the order here. */
		struct osu_action *end = *actions + num_actions++;
		struct osu_action *start = *actions + num_actions++;

		hitpoint_to_action(key_subset, cur_point, start, end);
	}

	free(key_subset);

	/* TODO: Free unused *actions memory (if any). */

	return num_actions;
}

static void hitpoint_to_action(const char *keys, struct osu_hitpoint *point,
			       struct osu_action *start, struct osu_action *end)
{
	end->time = point->end_time;
	start->time = point->start_time;

	/* Keyup */
	end->down = 0;
	/* Keydown */
	start->down = 1;

	char key = keys[point->column];

	end->key = key;
	start->key = key;
}

/* TODO: Implement a more efficient sorting algorithm. */
int sort_actions(int total, struct osu_action **actions)
{
	int min, i, j;
	struct osu_action *act = *actions, tmp;

	for (i = 0; i < (total - 1); i++) {
		min = i;

		for (j = i; j < total; j++)
			if ((act + j)->time < (act + min)->time) min = j;

		tmp = act[i];
		act[i] = act[min];
		act[min] = tmp;
	}

	return i - total + 1;
}

/* TODO: This function is stupid, fix it and add actual humanization. */
void humanize_hitpoints(int total, struct osu_hitpoint **points, int level)
{
	if (!level) {
		return;
	}

	int i, offset;
	struct osu_hitpoint *p = NULL;
	for (i = 0; i < total; i++) {
		p = *points + i;

		/* [0, level] */
		offset = generate_number(level, OSU_RNG_ROUNDS,
					 OSU_RNG_BOUNDARY);

		/* [-(level / 2), (level / 2)] */
		offset -= (level / 2);

		p->end_time += offset;
		p->start_time += offset;
	}
}

static int generate_number(int range, int rounds, double bound)
{
	int rn = rand() % range;

	/* Min and max percentage of the range we will use with our
	   constraint. */
	double minr = 0.5 - (bound / 2);
	double maxr = 0.5 + (bound / 2);

	for (int i = 0; i < rounds; i++) {
		int in = rn > (range * minr) && rn < (range * maxr);

		rn += (in ? (rand() % (int)(range * minr)) : 0) *
		      (rn < (range * 0.5) ? -1 : 1);
	}

	return rn;
}

static size_t find_partial_file(char *base, char *partial, char **out_file)
{
	if (!base || !partial || !out_file) {
		osu_debug("received null pointer");
		return 0;
	}

	DIR *dp;
	struct dirent *ep;

	if (!(dp = opendir(base))) {
		osu_debug("couldn't open directory %s", base);
		return 0;
	}

	const int file_len = 256;
	*out_file = malloc(file_len);

	int best_match = 0;

	while ((ep = readdir(dp))) {
		char *name = ep->d_name;
		int score = partial_match(name, partial);

		if (score > best_match) {
			best_match = score;

			strcpy(*out_file, name);
		}
	}

	closedir(dp);

	return strlen(*out_file);
}

/* TODO: I'm certain there's a more elegant way to go about this. */
static int partial_match(char *base, const char *partial)
{
	int i = 0;
	int m = 0;
	while (*base) {
		char c = partial[i];
		if (c == '.') {
			i++;
			continue;
		}

		if (*base++ == c) {
			i++;
			m++;
		}
	}

	return m;
}
