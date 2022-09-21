/* -*- tab-width: 8 -*- */
/** 
 *  some utility functions for strings
 *
 *  \file      string.h
 *  \author    Norbert Stoeffler
 *  \date      2001-2011
 *
 */

#ifndef STRING_H
#define STRING_H

#include	"basic.h"
#include	<string.h>

/*****************************************************************************
 *  defines
 ****************************************************************************/

#define CmdLineOpt(i,opt,n)\
	((!strncmp("-" opt,argv[(i)],MAX(strlen(argv[(i)]),2)))&&argc-(i)>(n))

#define StrGenAt(buffer, ...) impStrGenAt(buffer,sizeof(buffer),__VA_ARGS__)

#define CLEAR(x)	memset(&(x),0,sizeof(x))


/*****************************************************************************
 *  exported functions
 ****************************************************************************/

EXTERN_C_BEGIN

const char * StrGen(const char *fmt, ...);
const char * StrGen2(const char *fmt, ...);
const char * impStrGenAt(char *buffer, int size, const char *fmt, ...);

EXTERN_C_END

#endif /* UTIL_H */
