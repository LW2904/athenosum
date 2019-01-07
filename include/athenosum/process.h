#ifndef OSU_PROCESS_H
#define OSU_PROCESS_H

#include "common.h"

#include <string.h>
#include <limits.h>

#ifdef OSU_ON_LINUX
	// Enable GNU extensions (process_vm_readv).
	// TODO: This is hacky and undocumented.
	#define __USE_GNU

	#include <sys/uio.h>
#endif /* OSU_ON_LINUX */

#ifdef OSU_ON_WINDOWS
	#include <tlhelp32.h>

	HANDLE game_proc;
#endif /* OSU_ON_WINDOWS */

void *osu_time_address;
pid_t osu_game_proc_id;

/**
 * Gets and returns the runtime of the currently playing song, internally
 * referred to as `maptime`.
 */
__attribute__ ((hot)) int32_t get_maptime(void);

/**
 * Returns the process id of the given process or zero if it was not found.
 */
unsigned long get_process_id(const char *name);

/**
 * Copies game memory starting at `base` for `size` bytes into `buffer`.
 * Internally, this is a wrapper for _read_game_memory, with argument checking.
 * Returns number of bytes read and copied.
 */
ssize_t read_game_memory(void *base, void *buffer, size_t size);

/**
 * Searches for a signature (sequence of bytes) in the process, returning the
 * address of the end (!) of the first occurence.
 */
void *find_pattern(const unsigned char *signature, unsigned int sig_len);

/**
 * Returns the address of the playback time in the address space of the game
 * process.
 * Windows: Scans memory for the address using a signature.
 * Linux: Returns static address (LINUX_TIME_ADDRESS).
 */
void *get_time_address(void);

#endif /* OSU_PROCESS_H */
