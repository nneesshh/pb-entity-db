#ifndef __PLATFORM_UTILITIES_H__
#define __PLATFORM_UTILITIES_H__

#include "types.h"

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN 1
# include <windows.h>
# include <tchar.h>
# include <time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int util_strcmp_case(const char* a, const char* b);
extern void util_sleep(unsigned int milliseconds);

INLINE u16 read16_le(const u8* b)
{
	return b[0] + (b[1] << 8);
}

INLINE u16 read16_be(const u8* b)
{
	return (b[0] << 8) + b[1];
}

INLINE u32 read32_le(const u8* b)
{
	return read16_le(b) + (read16_le(b + 2) << 16);
}

INLINE u32 read32_be(const u8* b)
{
	return (read16_be(b) << 16) + read16_be(b + 2);
}

/// Converts an 80-bit IEEE 754 floating point number to a u32.
INLINE u32 readLD_be(const u8* b)
{
	u32 mantissa = read32_be(b + 2);
	u8 exp = 30 - b[1];
	u32 last = 0;
	while (exp--)
	{
		last = mantissa;
		mantissa >>= 1;
	}
	if (last & 0x1)
	{
		mantissa++;
	}
	return mantissa;
}

#if defined(WIN32) || defined(_WIN32)
HMODULE util_load_windows_system_library(const TCHAR *library_name);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __PLATFORM_UTILITIES_H__ */