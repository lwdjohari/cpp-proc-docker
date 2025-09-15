#ifndef XORA_PROC_HELPER_H
#define XORA_PROC_HELPER_H

#include <stdio.h>
#include <string.h>
#include <stddef.h>



#ifdef __cplusplus
extern "C"
{
#endif

/*  sqlca helpers (read-only)  */

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
