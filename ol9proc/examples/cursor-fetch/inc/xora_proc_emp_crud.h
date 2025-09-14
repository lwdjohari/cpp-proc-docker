#ifndef XORA_PROC_EMP_CRUD_H
#define XORA_PROC_EMP_CRUD_H
/* xora_emp.h — public API */

#include "xora_error.h"
#include "xora_contex.h"
#include "xora_proc_emp.h"
#ifdef __cplusplus
extern "C"
{
#endif

/* Txn helpers (still caller-controlled) */
xora_err_t xora_tx_commit(xora_conn_t *h);
xora_err_t xora_tx_rollback(xora_conn_t *h);

/* Next-id (MAX(id)+1) — no locks here */
xora_err_t xora_emp_next_id(xora_conn_t *h, int *out_empno);

/* INSERT helpers
 * - autoid: does MAX+1 + INSERT internally (no locks, no commit)
 * - with_id: INSERT using caller-provided id (no MAX+1, no commit)
 */
xora_err_t xora_emp_create_autoid(xora_conn_t *h,
                                  const xora_emp_row_t *in,
                                  int *out_empno);

xora_err_t xora_emp_create_with_id(xora_conn_t *h,
                                   const xora_emp_row_t *in,
                                   int explicit_empno,
                                   int *out_empno);

/* Batch helper (no commit, no locks): does MAX+1 + INSERT for each row */
xora_err_t xora_emp_batch_create_autoid(xora_conn_t *h,
                                        const xora_emp_row_t *rows,
                                        int count);

xora_err_t xora_create_employee_with_lock(xora_conn_t *conn,xora_emp_row_t *row, int *empid);

/* Read / Update / Delete */
xora_err_t xora_emp_get_by_id(xora_conn_t *h, int empno,
                              xora_emp_row_t *out, int *found);

xora_err_t xora_emp_update(xora_conn_t *h, const xora_emp_row_t *in);

xora_err_t xora_emp_delete(xora_conn_t *h, int empno);

#ifdef __cplusplus
}
#endif
#endif
