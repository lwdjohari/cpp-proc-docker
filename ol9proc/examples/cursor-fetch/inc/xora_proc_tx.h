/* Pro*C transaction functions (context-scoped)
 * xora_proc_tx.h
 *
 * Summary:
 *   - Begin tx variants: READ WRITE / READ ONLY (+ READ COMMITTED / SERIALIZABLE)
 *   - End tx: COMMIT / ROLLBACK
 *   - Savepoints:
 *
 * Standards:
 *   - All functions switch to the given sql_context and execute the statement.
 *   - Return xora_error_t on success; on error return Oracle sqlcode (<0). For invalid
 *     runtime savepoint identifiers, returns -20001 (client-side validation).
 */

#ifndef XORA_TX_H
#define XORA_TX_H

#include "xora_proc_contex.h"
#include "xora_error.h"

/* READ WRITE (session default isolation) */
int xora_tx_begin_rw(xora_conn_t *conn);

/* READ WRITE, ISOLATION LEVEL READ COMMITTED */
int xora_tx_begin_rw_rc(xora_conn_t *conn);

/* READ WRITE, ISOLATION LEVEL SERIALIZABLE */
int xora_tx_begin_rw_ser(xora_conn_t *conn);

/* READ ONLY (session default isolation) */
int xora_tx_begin_ro(xora_conn_t *conn);

/* READ ONLY, ISOLATION LEVEL READ COMMITTED */
int xora_tx_begin_ro_rc(xora_conn_t *conn);

/* READ ONLY, ISOLATION LEVEL SERIALIZABLE */
int xora_tx_begin_ro_ser(xora_conn_t *conn);

/* TX Commit */
xora_err_t xora_tx_commit(xora_conn_t *conn);

/* TX Rollback */
xora_err_t xora_tx_rollback(xora_conn_t *conn);


/* Savepoints 
 * Notes:
 *  - Oracle syntax requires an identifier token, not a bind.
 *  - SAVEPOINT <IDENT>  /  ROLLBACK TO SAVEPOINT <IDENT>
 *  - Basic identifier rule (conservative):
 *    1..30 bytes; first: A–Z / a–z / _ ; then: A–Z / a–z / 0–9 / _ $ #
 *    We upcase before issuing.
 *  - On invalid name: returns -20001 (no roundtrip).
 */
int xora_tx_savepoint(xora_conn_t *conn, const char *name);
int xora_tx_rollback_to(xora_conn_t *conn, const char *name);

#endif 
