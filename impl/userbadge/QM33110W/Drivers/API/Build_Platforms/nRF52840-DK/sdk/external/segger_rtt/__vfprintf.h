/* Stub header for __vfprintf.h when using RTT formatting */
/* This header is only needed when PRINTF_BUFFER_SIZE == 0, */
/* but is included unconditionally in SEGGER_RTT_Syscalls_SES.c */

#ifndef __VFPRINTF_H
#define __VFPRINTF_H

#include <stdarg.h>

/* Minimal declarations needed when using RTT formatting */
/* These won't be used when PRINTF_USE_SEGGER_RTT_FORMATTING=1 */

typedef struct {
  int string;
  int maxchars;
  int (*output_fn)(int, void*);
} __printf_t;

typedef __printf_t* __printf_tag_ptr;

int __vfprintf(__printf_t *iod, const char *fmt, va_list args);

#endif /* __VFPRINTF_H */


