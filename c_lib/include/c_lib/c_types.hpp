/*----------------------------------------------------------------------------
    c_types.hpp
----------------------------------------------------------------------------*/
#ifndef _C_TYPES_HPP_
#define _C_TYPES_HPP_

#include <tc_config.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
#if __PLATFORM__ == __P64LE__ || __PLATFORM__ == __P64BE__
typedef unsigned long  uint64_t;
#else
typedef unsigned long long uint64_t;
#endif

typedef signed char  int8_t;
typedef short int16_t;
typedef int   int32_t;
#if __PLATFORM__ == __P64LE__ || __PLATFORM__ == __P64BE__
typedef long  int64_t;
#else
typedef long long int64_t;
#endif

#endif

#endif
