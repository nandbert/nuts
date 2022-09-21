/* -*- tab-width: 8 -*- */
/** 
 *  some basic types and macros	
 *
 *  \file      basic.h
 *  \author    Norbert Stoeffler
 *  \date      199X
 *
 */

#ifndef BASIC_H
#define BASIC_H

/*****************************************************************************
 *  switches depending on target, OS, compiler, ...
 ****************************************************************************/

#include	"flavor.h"

/*****************************************************************************
 *  fallbacks for common constants
 ****************************************************************************/

#ifndef TRUE
#define TRUE		(1)
#endif

#ifndef FALSE
#define FALSE		(0)
#endif

#ifndef NULL
#define NULL		(0)
#endif


/*****************************************************************************
 *  fallbacks for common macros
 ****************************************************************************/

#ifndef MIN
#define MIN(a,b)	((a)>(b)?(b):(a))
#endif

#ifndef MAX
#define MAX(a,b)	((a)>(b)?(a):(b))
#endif

#ifndef ABS
#define ABS(x)		((int)(x)>=0?(x):-(x))
#endif

#ifndef SIGN
#define SIGN(x)		((x)>=0?1:-1)
#endif


/*****************************************************************************
 *  some nuts specials, boldly poluting the namespace
 ****************************************************************************/

#ifndef PAD

#ifndef NUTS_HIDDEN

#define CLIP(x,min,max)	((x)<(min)?(min):(x)>(max)?(max):(x))

#define LEN(x)		((int)(sizeof(x)/sizeof((x)[0])))
#define ISIN(x,l,u)	((l)<=(x)&&(x)<=(u))
#define PAD(x,p)	(MUST_((p)==2||(p)==4||(p)==8||(p)==16||(p)==32)\
			(((int)(x)+(p)-1)&~((p)-1)))
#define TRUNC(x,p)	(MUST_((p)==2||(p)==4||(p)==8||(p)==16||(p)==32)\
			((x)&~((p)-1)))

#endif

#ifdef __cplusplus
#define EXTERN_C	extern "C"
#define EXTERN_C_BEGIN	extern "C" {
#define EXTERN_C_END	}
#else
#define EXTERN_C	extern
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif

#endif

/*****************************************************************************
 *  bitsized types
 ****************************************************************************/

#if !defined LINUX_KERNEL || !defined __ASM_ARM_TYPES_H
#ifndef __cplusplus
#ifndef __HAVE_BOOL
#define __HAVE_BOOL
typedef int			bool;
#endif
#endif

#ifndef __HAVE_BITTYPES
#define __HAVE_BITTYPES
typedef unsigned char		u8;
typedef signed char		s8;
typedef unsigned short		u16;
typedef signed short		s16;
typedef unsigned int		u32;
typedef signed int		s32;
typedef unsigned long long	u64;
typedef long long		s64;

typedef volatile unsigned char	vu8;
typedef volatile signed char	vs8;
typedef volatile unsigned short	vu16;
typedef volatile signed short	vs16;
typedef volatile unsigned int	vu32;
typedef volatile signed int	vs32;
typedef volatile unsigned long long	vu64;
typedef volatile long long	vs64;
#endif
#endif

#define MAX_U8			(0xff)
#define MAX_U16			(0xffff)
#define MAX_U32			(0xffffffff)
#define MAX_U64			(0xffffffffffffffffLL)

#define MAX_S8			(0x7f)
#define MAX_S16			(0x7fff)
#define MAX_S32			(0x7fffffff)
#define MAX_S64			(0x7fffffffffffffffLL)

#define MIN_S8			(-0x80)
#define MIN_S16			(-0x8000)
#define MIN_S32			(-MAX_S32-1)
#define MIN_S64			(-MAX_S64-1)

#endif /* BASIC_H */
