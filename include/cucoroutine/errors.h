/*! @file errors.h
 * Error handling.
 */
#pragma once

#include <stdint.h>

/*!
 * Error type
 */
typedef int32_t cu_err_t; 

/*!
 * Global error variable such as errno.
 */
extern cu_err_t cu_errno;
void cu_seterrno(cu_err_t err);
void cu_ok();
void cu_fail();

enum {
    AE_OK = 0, ///< no error
    AE_FAIL, ///< some error
    AE_INIT, ///< initialisation failed 
    AE_GLIB, ///< glib function failed
    AE_PTHREAD, ///< pthread function failed
    AE_MALLOC, ///< memory is over, malloc returned NULL
    AE_CHNFULL, ///< channel is full
};


#define cu_error_if(err, cond) if (cond) {cu_seterrno(err);}
#define cu_error_if_and_return(err, cond, ret) if (cond) {cu_seterrno(err); return ret;}
