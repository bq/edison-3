/**
 * @file   drError.h
 * @brief  Error id definitions
 *
 * <Copyright goes here>
 */

#ifndef __DRERROR_H__
#define __DRERROR_H__

/**
 * Driver fatal error codes.
 */
/* Declare the error type as a pointer to a struct, so that the compiler can do
 * type checking and emit a warning when we are assign or compare two different
 * error types.
 */
typedef struct drError { int dummy; } * drError_t;

#define E_DR_OK               ((drError_t) 0) /**< Success */
#define E_DR_IPC              ((drError_t) 1) /**< IPC error */
#define E_DR_INTERNAL         ((drError_t) 2) /**< Internal error */
#define E_DR_BUSY             ((drError_t) 3) /**< Expect to be called again in same thread state */
    /* ... add more error codes when required */


#endif // __DRERROR_H__


