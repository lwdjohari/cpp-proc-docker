#ifndef XORA_PROC_EMP_FETCH_H
#define XORA_PROC_EMP_FETCH_H
/* xora_emp.h — public API */

#include "xora_error.h"
#include "xora_contex.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /* Array-fetch up to `cap` rows; fills rows and sets *out_count (0..cap). */
  xora_err_t xora_emp_fetch(xora_conn_t *ctx,
                            xora_emp_row_t *rows,
                            int cap,
                            int *out_count,
                            int batch_size);

#ifdef __cplusplus
}
#endif
#endif
