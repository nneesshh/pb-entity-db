#pragma once

/* Export functions from the DLL */
#ifndef MY_DB_EXTERN
# if defined(WIN32) || defined(_WIN32)
/* Windows - set up dll import/export decorators. */
#  if defined(MY_DB_BUILDING_SHARED)
/* Building shared library. */
#   define MY_DB_EXTERN __declspec(dllexport)
#  elif defined(MY_DB_USING_SHARED)
/* Using shared library. */
#   define MY_DB_EXTERN __declspec(dllimport)
#  else
/* Building static library. */
#    define MY_DB_EXTERN /* nothing */
#  endif
# elif __GNUC__ >= 4
#  define MY_DB_EXTERN __attribute__((visibility("default")))
# else
#  define MY_DB_EXTERN /* nothing */
# endif
#endif

/*EOF*/