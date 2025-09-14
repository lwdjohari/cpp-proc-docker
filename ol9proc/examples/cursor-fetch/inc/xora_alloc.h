#ifndef XORA_ALLOC_H
#define XORA_ALLOC_H
/* xora_alloc.h */

#include <limits.h>
#include <stddef.h>
#include <stdint.h> /* for SIZE_MAX */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

  static inline void *xora_malloc(size_t size)
  {
    void *p = malloc(size);
    if (!p && size)
    {
      fprintf(stderr, "FATAL: malloc(%zu) failed\n", size);
      abort();
    }
    return p;
  }

  static inline void *xora_calloc(size_t nmemb, size_t size)
  {
    /* overflow guard based on SIZE_MAX */
    if (size && nmemb > SIZE_MAX / size)
    {
      fprintf(stderr, "FATAL: calloc overflow (%zu,%zu)\n", nmemb, size);
      abort();
    }
    void *p = calloc(nmemb, size);
    if (!p && nmemb && size)
    {
      fprintf(stderr, "FATAL: calloc(%zu,%zu) failed\n", nmemb, size);
      abort();
    }
    return p;
  }

  static inline void *xora_realloc(void *ptr, size_t size)
  {
    void *p = realloc(ptr, size);
    if (!p && size != 0)
    {
      fprintf(stderr, "FATAL: realloc(%p,%zu) failed\n", ptr, size);
      abort();
    }
    return p;
  }

  /* Typed array helpers  */
  static inline size_t xora_size_mul(size_t n, size_t elem)
  {
    if (elem && n > SIZE_MAX / elem)
    {
      fprintf(stderr, "FATAL: size overflow (%zu * %zu)\n", n, elem);
      abort();
    }
    return n * elem;
  }

/* Allocate array of T (uninitialized / zero-initialized / resize) */
#define XORA_ALLOC_ARRAY(T, n) ((T *)xora_malloc(xora_size_mul((n), sizeof(T))))
#define XORA_CALLOC_ARRAY(T, n) ((T *)xora_calloc((n), sizeof(T)))
#define XORA_RESIZE_ARRAY(ptr, T, n) ((T *)xora_realloc((ptr), xora_size_mul((n), sizeof(T))))

/* Define char[def+1] with additional +1 for NUL terminator */
#define XORA_ARRSTR(n) char[(n) + 1]

/* Free + NULL (safe in single-line ifs via do/while guard) */
#define xora_free(ptr) \
  do                   \
  {                    \
    free(ptr);         \
    (ptr) = NULL;      \
  } while (0)

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
