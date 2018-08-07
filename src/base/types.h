#ifndef __PLATFORM_TYPES_H__
#define __PLATFORM_TYPES_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t     u8;
typedef int8_t      s8;
typedef uint16_t    u16;
typedef int16_t     s16;
typedef uint32_t    u32;
typedef int32_t     s32;
typedef uint64_t    u64;
typedef int64_t     s64;

/* DLLs in Windows should use the standard (Pascal) calling convention */
#ifndef MYAPI_CALL
#if defined(WIN32) || defined(_WIN32)
#define MYAPI_CALL __stdcall
#else
#define MYAPI_CALL
#endif
#endif

/* Export functions from the DLL */
#ifndef MY_EXTERN
# if defined(WIN32) || defined(_WIN32)
  /* Windows - set up dll import/export decorators. */
#  if defined(MY_BUILDING_SHARED)
   /* Building shared library. */
#   define MY_EXTERN __declspec(dllexport)
#  elif defined(MY_USING_SHARED)
    /* Using shared library. */
#   define MY_EXTERN __declspec(dllimport)
#  else
    /* Building static library. */
#    define MY_EXTERN /* nothing */
#  endif
# elif __GNUC__ >= 4
#  define MY_EXTERN __attribute__((visibility("default")))
# else
#  define MY_EXTERN /* nothing */
# endif
#endif

#ifdef _MSC_VER /* msvc */
# pragma warning(disable : 4786)
# ifndef INLINE
# define INLINE __inline
# endif
#else  /* gcc */
# ifndef INLINE
# define INLINE inline
# endif
#endif

#if defined(WIN32) || defined(_WIN32)

#else
#  define _atoi64(val)	strtoll(val, NULL, 10)
#endif

#define MYDLL_FUNCTION(ret) extern "C" MY_EXTERN ret MYAPI_CALL
#define MYDLL_METHOD(ret) virtual ret MYAPI_CALL

#ifdef __cplusplus
}
#endif

#endif /* __PLATFORM_TYPES_H__ */
