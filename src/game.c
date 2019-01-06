#include "athenosum/game.h"

static size_t get_env_var(char *name, char **out_var);

__attribute__ ((hot)) inline void send_keypress(int key, int down)
{
#ifdef OSU_ON_LINUX
	KeyCode keycode = XKeysymToKeycode(display, key);

	if (!keycode)
		return;

	XTestFakeKeyEvent(display, (unsigned)keycode, down, CurrentTime);

	XFlush(display);
#endif /* OSU_ON_LINUX */

#ifdef OSU_ON_WINDOWS
	INPUT in;

	in.type = INPUT_KEYBOARD;

	in.ki.time = 0;
	in.ki.wScan = 0;
	in.ki.dwExtraInfo = 0;
	in.ki.dwFlags = down ? 0 : KEYEVENTF_KEYUP;
	in.ki.wVk = VkKeyScanEx(key, GetKeyboardLayout(0)) & 0xFF;

	SendInput(1, &in, sizeof(INPUT));
#endif /* OSU_ON_WINDOWS */
}

void tap_key(int key)
{
	send_keypress(key, 1);
	nanosleep((struct timespec[]){{ 0, 10000000L }}, NULL);
	send_keypress(key, 0);
}

size_t get_osu_path(char **out_path)
{
	if (!out_path) {
		osu_debug("received null pointer");
		return 0;
	}

	char *home;
	size_t home_len;

	if (!(home_len = get_env_var(OSU_HOME_ENV, &home))) {
		osu_debug("failed fetching home path");
		return 0;
	}

	// Subtract one since sizeof includes the terminating null.
	size_t path_len = home_len + (sizeof(OSU_DEFAULT_PATH) - 1);
	*out_path = malloc(path_len + 1);

	strcpy(*out_path, home);
	// We overwrite the old terminating null here.
	strcpy(*out_path + home_len, OSU_DEFAULT_PATH);

	free(home);

	return path_len;
}

void do_setup()
{
#ifdef OSU_ON_LINUX
	if (!(display = XOpenDisplay(NULL))) {
		printf("failed to open X display\n");

		return;
	}

	osu_debug("opened X display (%#x)", (unsigned)(intptr_t)display);
#endif /* OSU_ON_LINUX */

#ifdef OSU_ON_WINDOWS
	if (!(game_proc = OpenProcess(PROCESS_VM_READ, 0, game_proc_id))) {
		printf("failed to get handle to game process\n");
		return;
	}
	
	osu_debug("got handle to game process with ID %d",
		(int)game_proc_id);
#endif /* OSU_ON_WINDOWS */
}

static size_t get_env_var(char *name, char **out_var)
{
	if (!out_var || !name) {
		osu_debug("received null pointer");
		return 0;
	}

	size_t var_len = 0;
	char *var = getenv(name);

	// getenv() returns NULL if the variable could not be found.
	if (!var || !(var_len = strlen(var))) {
		osu_debug("environmental variable (%s) does not exist", name);

		return 0;
	}

	*out_var = malloc(var_len + 1);
	// Copy because getenv returns a pointer to internal memory.
	strcpy(*out_var, var);

	return var_len;
}
