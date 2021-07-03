/*! @file errors.h
 * Error handling.
 */
#pragma once

#include <errno.h>
#include <stdint.h>

typedef int32_t cu_err_t;

typedef enum {
    CU_EOK = 0,
    __CU_MASK__ = 0x100,
    CU_EFAIL,
};

void cu_seterr(cu_err_t);
cu_err_t cu_geterr();


