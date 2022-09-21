/* -*- tab-width: 8 -*- */
/**
 *  handle strings
 *	
 *  \file      strmem.c
 *  \author    Norbert Stoeffler
 *  \date      2001-2011
 *
 */

//#define DLOGGING
#include	"strmem.h"
#include	"debug.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<stdarg.h>


/*****************************************************************************
 *  local variables
 ****************************************************************************/

/*****************************************************************************
 *  exported functions
 ****************************************************************************/

/****************************************************************************/
/** return a pointer to a temporary string. the pointer can be used directly
 *  as filename in fopen() etc. but becomes invalid with the next call (use
 *  strdup(StrGen(...)) for a permanent string)
 *
 *  \param  fmt as printf
 *  \return the string
 */
const char * StrGen(const char *fmt, ...)
{
  va_list       v_args;
  static char   buffer[1024];

  va_start(v_args,fmt);
  vsnprintf(buffer,sizeof(buffer),fmt,v_args);
  MUST(strlen(buffer)<sizeof(buffer));
  va_end(v_args);

  return buffer;
}


/****************************************************************************/
/** as StrGen, for cases where 2 tmp strings are needed at once
 *
 *  \param  fmt as printf
 *  \return the string
 */
const char * StrGen2(const char *fmt, ...)
{
  va_list       v_args;
  static char   buffer[1024];

  va_start(v_args,fmt);
  vsnprintf(buffer,sizeof(buffer),fmt,v_args);
  MUST(strlen(buffer)<sizeof(buffer));
  va_end(v_args);

  return buffer;
}


/****************************************************************************/
/*  used internally by macro StrGenAt
 */
const char * impStrGenAt(char *buffer, int size, const char *fmt, ...)
{
  va_list       v_args;

  va_start(v_args,fmt);
  vsnprintf(buffer,size,fmt,v_args);
  MUST((int)strlen(buffer)<size);
  va_end(v_args);

  return buffer;
}


