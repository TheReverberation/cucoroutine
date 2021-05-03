/*! @file errors.h
 * Error handling.
 */
#pragma once

#include <stdint.h>

/*!
 * Error type
 */
typedef int32_t aio_err_t; 

/*!
 * Global error variable such as errno.
 */
extern aio_err_t aio_errno;
void aio_seterrno(aio_err_t err);
void aio_ok();
void aio_fail();

enum {
    AE_OK = 0, ///< no error
    AE_FAIL, ///< some error
    AE_INIT, ///< initialisation failed 
    AE_GLIB, ///< glib function failed
    AE_PTHREAD, ///< pthread function failed
    AE_MALLOC, ///< memory is over, malloc returned NULL
    AE_CHNFULL, ///< channel is full
};


#define aio_error_if(err, cond) if (cond) {aio_seterrno(err);}
#define aio_error_if_and_return(err, cond, ret) if (cond) {aio_seterrno(err); return ret;}
