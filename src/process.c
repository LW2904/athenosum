#include "athenosum/process.h"

static inline void *check_chunk(const unsigned char *sig, size_t sig_size,
				unsigned char *buf, size_t buf_size);

/**
 * Copies game memory at base for size bytes into buffer.
 * Inlined, hot version without argument validation.
 * Returns number of bytes read.
 */
static inline ssize_t _read_game_memory(void *base, void *buffer,
					size_t size);

__attribute__ ((hot)) int32_t get_maptime()
{
	int32_t time = 0;
	size_t size = sizeof(int32_t);

	// This function is called in tight loops, use the faster, insecure
	// read_game_memory since we know our arguments are valid.
	if (!(_read_game_memory(osu_time_address, &time, size)))
		return 0;

	return time;
}

ssize_t read_game_memory(void *base, void *buffer, size_t size)
{
	if (!base || !buffer || !size)
		return 0;

	ssize_t read = 0;

	if (!(read = _read_game_memory(base, buffer, size)))
		return 0;

	return read;
}

static inline ssize_t _read_game_memory(void *base, void *buffer,
					size_t size)
{
	ssize_t read = 0;

#ifdef OSU_ON_LINUX
	struct iovec local[1];
	struct iovec remote[1];

	local[0].iov_len = size;
	local[0].iov_base = buffer;

	remote[0].iov_len = size;
	remote[0].iov_base = base;

	read = process_vm_readv(osu_game_proc_id, local, 1, remote, 1, 0);
#endif /* OSU_ON_LINUX */

#ifdef OSU_ON_WINDOWS
	ReadProcessMemory(game_proc, (LPCVOID)base, buffer, size,
		(SIZE_T *)&read);
#endif /* OSU_ON_WINDOWS */

	return read;
}

unsigned long get_process_id(const char *name)
{
	unsigned long proc_id = 0;

#ifdef OSU_ON_LINUX
	char *cmd = (char *)calloc(1, 200);
	sprintf(cmd, "pidof %s", name);

	FILE *f = popen(cmd, "r");
	size_t read = fread(cmd, 1, 200, f);

	fclose(f);

	proc_id = read ? (unsigned long)atoi(cmd) : 0;

	free(cmd);
#endif /* OSU_ON_LINUX */

#ifdef OSU_ON_WINDOWS
	HANDLE proc_list = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 entry = { 0 };
	entry.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(proc_list, &entry)) {
		goto end;
	}

	while (Process32Next(proc_list, &entry)) {
		if (_stricmp((char *)entry.szExeFile, name) == 0) {
			proc_id = (unsigned long)entry.th32ProcessID;

			goto end;
		}
	}

end:
	CloseHandle(proc_list);
#endif /* OSU_ON_WINDOWS */

	osu_debug("process ID for %s is %ld", name, proc_id);

	return proc_id;
}

void *get_time_address()
{
#ifdef OSU_ON_WINDOWS
	void *osu_time_address = NULL;
	void *time_ptr = find_pattern((unsigned char *)SIGNATURE,
		sizeof(SIGNATURE) - 1);

	if (!ReadProcessMemory(game_proc, (void *)time_ptr, &osu_time_address,
		sizeof(DWORD), NULL))
	{
		return NULL;
	}

	return osu_time_address;
#endif

#ifdef OSU_ON_LINUX
	return (void *)OSU_LINUX_TIME_ADDRESS;
#endif
}

void *find_pattern(const unsigned char *signature, unsigned int sig_len)
{
	const size_t read_size = 4096;
	unsigned char chunk[read_size];

	// Get reasonably sized chunks of memory...
	for (size_t off = 0; off < INT_MAX; off += read_size - sig_len) {
		if (!(read_game_memory((void *)off, chunk, read_size))) {
			continue;
		}

		// ...and check if they contain our signature.
		void *hit = check_chunk(signature, sig_len, chunk, read_size);

		if (hit)
			return (void *)((intptr_t)off + (intptr_t)hit);
	}

	return NULL;
}

// TODO: Use a more efficient pattern matching algorithm.
static inline void *check_chunk(const unsigned char *sig, size_t sig_size,
				unsigned char *buf, size_t buf_size)
{
	// Iterate over the buffer...
	for (size_t i = 0; i < buf_size; i++) {
		int hit = true;

		// ...to check if it includes the pattern/sig.
		for (size_t j = 0; j < sig_size && hit; j++) {
			hit = buf[i + j] == sig[j];
		}

		if (hit) {
			return (void *)(i + sig_size);
		}
	}

	return NULL;
}
