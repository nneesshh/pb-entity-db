#include "utilities.h"

#include <ctype.h>

int
util_strcmp_case(const char* a, const char* b) {
  char c, d;
  while (*a && *b) {
    c = tolower(*a++);
    d = tolower(*b++);

    if (c != d) {
      return c - d;
    }
  }

  c = tolower(*a);
  d = tolower(*b);
  return (c - d);
}

void
util_sleep(unsigned int milliseconds) {
#if defined(WIN32) || defined(_WIN32)
	Sleep(milliseconds);
#else
	int seconds = milliseconds / 1000;
	int useconds = (milliseconds % 1000) * 1000;

	sleep(seconds);
	usleep(useconds);
#endif
}

#if defined(WIN32) || defined(_WIN32)
HMODULE
util_load_windows_system_library(const TCHAR *library_name) {
	TCHAR path[MAX_PATH];
	unsigned n;
	n = GetSystemDirectory(path, MAX_PATH);
	if (n == 0 || n + _tcslen(library_name) + 2 >= MAX_PATH)
		return 0;
	_tcscat(path, TEXT("\\"));
	_tcscat(path, library_name);
	return LoadLibrary(path);
}
#endif


/** -- EOF -- **/