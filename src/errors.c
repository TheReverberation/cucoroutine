#include "errors.h"

int32_t aio_errno = AE_OK;

void aio_seterrno(int32_t err) {
    aio_errno = err;
}

void aio_ok() {
    aio_errno = AE_OK;
}

void aio_fail() {
    aio_errno = AE_FAIL;
}