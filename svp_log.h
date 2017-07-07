#ifndef SRC_SVP_LOG_H_
#define SRC_SVP_LOG_H_

#ifdef __pnacl__
#define	isascii(c)	(((c) & ~0x7f) == 0)	/* If C is a 7 bit value.  */
#endif

#include "svp_types.h"
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>

namespace SVP {

template<typename... Args>
str_t string_format(const i8_t* fmt, Args... args) {

    size_t size = snprintf(nullptr, 0, fmt, args...);
    str_t buf;
    buf.reserve(size + 1);
    buf.resize(size);
    snprintf(&buf[0], size + 1, fmt, args...);
    return buf;
}

};	// namespace SVP

#define STR SVP::string_format
#define VERBOSE(message) printf("%s\n", message)
#define SVP_ERROR(log_message, ...) syslog(LOG_ERR, log_message "\n", ##__VA_ARGS__)
#define SVP_INFO(log_message, ...)  syslog(LOG_INFO, log_message "\n", ##__VA_ARGS__)
#define SVP_DEBUG(log_message, ...) syslog(LOG_DEBUG, log_message "\n", ##__VA_ARGS__)
#define SVP_PROC SVP_INFO("%s", __FUNCTION__)
#define ASSERT(x) SVP_INFO("%s : " #x " = 0x%X", __FUNCTION__, x); assert(x)

#endif /* SRC_SVP_LOG_H_ */
