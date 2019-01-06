#ifndef OSU_COMMON_H
#define OSU_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <sys/types.h>

#define OSU_KEY_RETURN 0xFF0D
#define OSU_KEY_ESCAPE 0xFF1B

#ifdef _WIN32
  #define OSU_ON_WINDOWS

  #include "windows.h"

  #define OSU_HOME_ENV "USERPROFILE"
  #define OSU_SIGNATURE "\xDB\x5D\xE8\x8B\x45\xE8\xA3"
 
  #define OSU_SEPARATOR '\\'
  #define OSU_DEFAULT_PATH "\\AppData\\Local\\osu!\\Songs\\"
#endif /* _WIN32 */

#ifdef __linux__
  #define OSU_ON_LINUX

  #include <X11/Xlib.h>
  #include <X11/extensions/XTest.h>

  #define OSU_HOME_ENV "HOME"
  #define OSU_LINUX_TIME_ADDRESS 0x36e5ba4

  #define OSU_SEPARATOR '/'
  #define OSU_DEFAULT_PATH "/osufolder/Songs/"
#endif /* __linux__ */

#ifdef OSU_DEBUG

  #define osu_debug(...)\
      printf("[osu_debug] [%s:%s] ", __FILE__, __func__);\
      printf(__VA_ARGS__);\
      putchar('\n');\

#else

  #define osu_debug(...)\
      ;\

#endif /* DEBUG */

/* Suggested default */
#define OSU_TAPTIME_MS 15

#define OSU_COLS_WIDTH 512
#define OSU_MAX_LINE_LENGTH 1024

#endif /* OSU_COMMON_H */
