/* -*- tab-width: 8 -*- */
/** 
 *	
 *
 *  \file      regrw.h
 *  \author    Norbert Stoeffler
 *  \date      2001-2011
 *
 */

#ifndef REGRW_H
#define REGRW_H

#include		"basic.h"

/*****************************************************************************
 *  defines
 ****************************************************************************/

#define REG32R(a)	(*(volatile u32*)(a))
#define REG16R(a)	(*(volatile u16*)(a))
#define REG8R(a)	(*(volatile u8*)(a))

#define REG32W(a,v)	(*(volatile u32*)(a)=(u32)(v))
#define REG16W(a,v)	(*(volatile u16*)(a)=(u16)(v))
#define REG8W(a,v)	(*(volatile u8*)(a)=(u8)(v))

#endif /* REGRW_H */
