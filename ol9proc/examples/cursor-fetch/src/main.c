#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xora_error.h"
#include "xora_alloc.h"
#include "xora_contex.h"
#include "xora_proc_emp.h"
#include "xora_proc_emp_fetch.h"

static const char *get_env_or(const char *key, const char *defv)
{
    const char *v = getenv(key);
    return (v && *v) ? v : defv;
}

static void usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s [user] [pass] [//host:1521/SERVICE] [cap] [batch]\n"
            "Env fallbacks: ORA_USER, ORA_PASS, ORA_DB\n"
            "Defaults: scott tiger //localhost:1521/FREE 64 128\n",
            prog);
}

int main(int argc, char **argv)
{
    const char *user = (argc > 1) ? argv[1] : get_env_or("ORA_USER", "scott");
    const char *pass = (argc > 2) ? argv[2] : get_env_or("ORA_PASS", "tiger");
    const char *db = (argc > 3) ? argv[3] : get_env_or("ORA_DB", "//host.docker.internal:1521/FREEPDB1");

    int ulen = user ? (int)strlen(user) : 0;
    int plen = pass ? (int)strlen(pass) : 0;
    int dlen = db   ? (int)strlen(db)   : 0;

    fprintf(stderr, "ORA_USER: %.*s(%d) ORA_PASS: %.*s(%d) ORA_DB: %.*s(%d)\n",
        ulen, user ? user : "", ulen,
        plen, pass ? pass : "", plen,
        dlen, db   ? db   : "", dlen);

    int cap = (argc > 4) ? atoi(argv[4]) : 64;
    int batch = (argc > 5) ? atoi(argv[5]) : 128;

    if (cap <= 0)
        cap = 64;
    if (batch <= 0)
        batch = 128;

    xora_conn_t *conn = NULL;

    /* Create handle */
    if (xora_conn_create(&conn, user, pass, db) != XORA_OK || !conn)
    {
        fprintf(stderr, "xora_conn_create failed\n");
        return 1;
    }

    /* Open connection */
    if (xora_conn_open(conn) != XORA_CONN_OPEN_OK)
    {
        fprintf(stderr, "xora_conn_open failed\n");
        xora_conn_destroy(&conn);
        return 1;
    }

    /* Allocate output buffer */
    xora_emp_row_t *rows = XORA_ALLOC_ARRAY(xora_emp_row_t, cap);

    int out_count = 0;

    xora_err_t rc = xora_emp_fetch(conn, rows, cap, &out_count, batch);
    if (rc != XORA_OK)
    {
        fprintf(stderr, "xora_emp_fetch failed (rc=%d)\n", rc);
        xora_free(rows);
        xora_conn_close(conn);
        xora_conn_destroy(&conn);
        return 1;
    }

    /* Print results */
    printf("Fetched %d row(s):\n", out_count);
    printf("%-6s  %-50s  %10s\n", "EMPNO", "ENAME", "SAL");
    printf("------  --------------------------------------------------  ----------\n");
    for (int i = 0; i < out_count; ++i)
    {
        printf("%-6d  %-50s  %10.2f\n", rows[i].empno, rows[i].ename, rows[i].salary);
    }

    xora_free(rows);
    xora_conn_close(conn);
    xora_conn_destroy(&conn);

    return 0;
}
