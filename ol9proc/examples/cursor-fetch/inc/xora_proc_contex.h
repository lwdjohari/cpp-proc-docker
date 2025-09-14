#ifndef XORA_PROC_CTX_H
#define XORA_PROC_CTX_H
/* xora_proc_ctx.h — only for .pc files; carries sql_context & VARCHARs */


typedef struct xora_conn {
  sql_context ctx;
  char        user[32];
  char        pass[32];
  char        db[128];
  int         broken;
} xora_conn_t;

#endif
