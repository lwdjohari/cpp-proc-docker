#ifndef XORA_ERROR_H
#define XORA_ERROR_H

/* xora_error.h */

#include <stddef.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C"
{
#endif

typedef enum XORA_ERR{
    XORA_OK = 0,
    XORA_CONN_OPEN_OK = 0,
    XORA_TX_COMMIT_OK = 0,
    XORA_ALLOCATION_FAILED = 1,
    XORA_ALREADY_ALLOCATED = 2,
    XORA_CONN_CLOSED = 3,
    XORA_CONN_ERR = 4,
    XORA_CONN_UNKNOWN = 5,
    XORA_TIMEOUT = 6,
    XORA_ERR = 7,
    XORA_NO_DATA_FOUND = 8,
    XORA_LOCK_TABLE_FAILED = 9,
    XORA_TX_ROLLBACK = 10,
    XORA_TX_CREATE_ERR = 11
}xora_err_t;


#ifdef __cplusplus
}
#endif

#endif