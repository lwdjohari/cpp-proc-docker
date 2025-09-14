#ifndef XORA_CONTEXT_H
#define XORA_CONTEXT_H
/* xora_context.h (public API; no Pro*C tokens/types here) */

#include <stddef.h>
#include "xora_error.h"


#ifdef __cplusplus
extern "C" {
#endif

/* Forward decl: real definition lives in proc-only header */
typedef struct xora_conn xora_conn_t;

/* Create a connection handle (disconnected). Caller receives ownership. */
xora_err_t xora_conn_create(xora_conn_t** out,
                            const char* user,
                            const char* pass,
                            const char* db);

/* Open / close the DB connection for this handle (per-thread). */
xora_err_t xora_conn_open(xora_conn_t* ctx);
void       xora_conn_close(xora_conn_t* ctx);

/* Quick check the connection liveness; fast probe. */
xora_err_t xora_conn_is_open( xora_conn_t* ctx);

/* Destroy the handle and free resources. Closes if still open. */
void       xora_conn_destroy(xora_conn_t** ctxp);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
