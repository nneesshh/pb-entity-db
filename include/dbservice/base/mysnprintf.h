#ifndef __MYSNPRINTF_H__
#define __MYSNPRINTF_H__

#ifdef __cplusplus 
extern "C" { 
#endif 

#if HAVE_CONFIG_H
#include <config.h>
#endif	/* HAVE_CONFIG_H */

#define HAVE_STDARG_H	1
#define HAVE_LONG_LONG_INT	1
#define HAVE_UNSIGNED_LONG_LONG_INT	1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#if !HAVE_VSNPRINTF
	int o_vsnprintf(char *, size_t, const char *, va_list);
#endif	/* !HAVE_VSNPRINTF */

#if !HAVE_SNPRINTF
	int o_snprintf(char *, size_t, const char *, ...);
#endif	/* !HAVE_SNPRINTF */

#if !HAVE_VASPRINTF
	//int o_vasprintf(char **, const char *, va_list);
#endif	/* !HAVE_VASPRINTF */

#if !HAVE_ASPRINTF
	int o_asprintf(char **, const char *, ...);
#endif	/* !HAVE_ASPRINTF */

#ifdef __cplusplus 
}
#endif 

#endif
