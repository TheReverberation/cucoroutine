#include <threads.h>

#include "errors.h"

thread_local cu_err_t cu_errno = CU_EOK;

void cu_seterr(cu_err_t err) {
    cu_errno = err;
}

cu_err_t cu_geterr() {
    return cu_errno;
}

void cu_reseterr() {
    cu_errno = CU_EOK;
}

cu_err_t cu_geterr_and_reset() {
    cu_err_t err = cu_errno;
    cu_errno = CU_EOK;
    return err;
}
