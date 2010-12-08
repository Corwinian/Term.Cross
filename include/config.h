#ifndef _CONFIG_H_
#define _CONFIG_H_

#define HAVE_LIMITS_H

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#define __PLATFORM__ 1 //platform type
#define __P32LE__ 0 // 32-bit litle endian
#define __P64LE__ 1 // 64-bit litle endian
#define __P32BE__ 2 // 32-bit big endian
#define __P64BE__ 3 // 64-bit big endian

#define DIRD '/'					//dir delimeter
#define PATHD ':'					//path delimeter
#define MAX_PATH PATH_MAX
#define MAX_FNAME NAME_MAX
#define MAX_EXT NAME_MAX
#define MAX_DIR NAME_MAX
#define MAX_DRIVE 3
/* #undef HAVE_DRIVE */
/* #undef STRICMP */
#define STRCASECMP
#define HAVE_STDINT_H

#define LOG_FORMAT "%d %t %e %m"
//typedef unsigned char byte;
//#define BYTE byte

#endif /*_CONFIG_H_ */
