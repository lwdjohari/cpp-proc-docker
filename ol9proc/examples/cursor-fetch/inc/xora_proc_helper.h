#ifndef XORA_PROC_HELPER_H
#define XORA_PROC_HELPER_H
/* xora_proc_helper.h — only for .pc files */

// #ifndef XORA_PROC
// # error "This header is Pro*C-only. Compile .pc with proc define=XORA_PROC"
// #endif

#include <stdio.h>
#include <string.h>
#include <stddef.h>

// #include <sqlca.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /* ---- sqlca helpers (read-only) ---- */

#define XORA_ORA_BREAK_ON_NODATA()                     \
  do                                                   \
  {                                                    \
    if (sqlca.sqlcode == 1403 || sqlca.sqlcode == 100) \
      break;                                           \
  } while (0)

  /* Error only (sqlcode < 0) */
#define XORA_ORA_OK(step)                                                               \
  ((sqlca.sqlcode < 0) ? (fprintf(stderr, "[ORA-ERR] step=%s code=%ld msg=%.*s\n",      \
                                  (step), (long)sqlca.sqlcode,                          \
                                  (int)sqlca.sqlerrm.sqlerrml, sqlca.sqlerrm.sqlerrmc), \
                          0)                                                            \
                       : 1)

/* Error and Warning (sqlcode != 0) */
#define XORA_ORA_OK_STRICT(step)                                                         \
  ((sqlca.sqlcode != 0) ? (fprintf(stderr, "[ORA-%s] step=%s code=%ld msg=%.*s\n",       \
                                   (sqlca.sqlcode < 0 ? "ERR" : "WARN"),                 \
                                   (step), (long)sqlca.sqlcode,                          \
                                   (int)sqlca.sqlerrm.sqlerrml, sqlca.sqlerrm.sqlerrmc), \
                           0)                                                            \
                        : 1)

  static inline int xora_ora_ok(const char *step)
  {
    if (sqlca.sqlcode < 0)
    {
      fprintf(stderr, "[ORA] step=%s code=%ld msg=%.*s\n",
              step, (long)sqlca.sqlcode,
              (int)sqlca.sqlerrm.sqlerrml, sqlca.sqlerrm.sqlerrmc);
      return 0;
    }
    return 1;
  }

  static inline int xora_ora_truncated(void)
  {
    return (sqlca.sqlwarn[0] == 'W' && sqlca.sqlwarn[1] == 'W');
  }

  static inline long xora_ora_rows(void) { return sqlca.sqlerrd[2]; }

  // /* ---- VARCHAR helpers ---- */

  // static inline void xora_ut8_copy_bounded(char *dst, size_t cap, const char *src) {
  //     if (!dst || cap == 0) return;
  //     if (!src) { dst[0] = '\0'; return; }
  //     size_t n = strnlen(src, cap - 1);
  //     memcpy(dst, src, n);
  //     dst[n] = '\0';
  // }

  /* Safety copy for fixed char[N] destinations */
  static inline void xora_ut8_copy_bounded(char *dst, const char *src, size_t dst_size)
  {
    if (!dst || dst_size == 0)
      return;
    if (!src)
    {
      dst[0] = '\0';
      return;
    } // optional: guard NULL src
    size_t i = 0;
    for (; i + 1 < dst_size && src[i]; ++i)
      dst[i] = src[i];
    dst[i] = '\0';
  }

  static inline void xora_memcpy(char *dst, size_t dst_size, const char *src, size_t src_len)
  {
    if (!dst || dst_size == 0)
      return;
    size_t n = (src_len < (dst_size - 1)) ? src_len : (dst_size - 1);
    if (src && n)
      memcpy(dst, src, n);
    dst[n] = '\0';
    /* optional: zero-pad tail
       if (n + 1 < dst_size) memset(dst + n + 1, 0, dst_size - (n + 1)); */
  }

  static inline void xora_varchar_set(VARCHAR *v, size_t cap, const char *s)
  {
    size_t n = s ? strlen(s) : 0;
    if (n > cap)
      n = cap;
    if (n)
      memcpy(v->arr, s, n);
    v->len = (unsigned short)n;
  }

  static inline void xora_varchar_to_c(char *dst, size_t cap, const VARCHAR *v)
  {
    if (!dst || cap == 0)
      return;
    size_t n = v ? (size_t)v->len : 0;
    if (n >= cap)
      n = cap - 1;
    if (n)
      memcpy(dst, v->arr, n);
    dst[n] = '\0';
  }

#ifdef __cplusplus
}
#endif

#endif
