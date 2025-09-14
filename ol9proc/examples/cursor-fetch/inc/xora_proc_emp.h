#ifndef XORA_PROC_EMP_H
#define XORA_PROC_EMP_H
/* xora_emp.h — public API */

#include "xora_error.h"
#include "xora_contex.h"

#ifdef __cplusplus
extern "C"
{
#endif
  
  typedef struct XoraEmpRow
  {
    int empno;
    double salary;
    char ename[51]; /* +1 already accounted */
    short ename_is_null;
  } xora_emp_row_t;


  typedef struct __XoraEmpRowInd
  {
    short empno;
    short sal;
    short ename;
  } emp_row_ind_t;


#ifdef __cplusplus
}
#endif
#endif
