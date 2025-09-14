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
    XORA_ALLOCATION_FAILED = 1,
    XORA_ALREADY_ALLOCATED = 2,
    XORA_CONN_CLOSED = 3,
    XORA_CONN_ERR = 4,
    XORA_CONN_UNKNOWN = 5,
    XORA_TIMEOUT = 6,
    XORA_ERR = 7
}xora_err_t;


#ifdef __cplusplus
}
#endif

#endif