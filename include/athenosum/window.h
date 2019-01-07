#ifndef OSU_WINDOW_H
#define OSU_WINDOW_H

#include "common.h"

#include <stdlib.h>

#ifdef OSU_ON_WINDOWS
	extern HWND osu_game_window;
	extern HANDLE game_proc;
#endif /* OSU_ON_WINDOWS */

#ifdef OSU_ON_LINUX
	extern Display *osu_display;
	extern Window osu_game_window;
#endif /* OSU_ON_LINUX */

/**
 * Finds the main window of a process with a given ID and stores the
 * OS-appropiate handle to it in *out_window.
 */
int find_window(unsigned long process_id, void **out_window);

/**
 * Fetches the title of the game window. *title is expected to point to a
 * region of memory that is writable.
 */
__attribute__ ((hot)) int get_window_title(char **title, int title_len);

#endif /* OSU_WINDOW_H */
