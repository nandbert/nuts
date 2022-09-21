/* -*- tab-width: 8 -*- */
/** 
 *  build flavor	
 *
 *  \file      flavor.h
 *  \author    Norbert Stoeffler
 *  \date      200X
 *
 */

#ifndef FLAVOR_H
#define FLAVOR_H

/*****************************************************************************
 *  defines
 ****************************************************************************/

#if defined __KERNEL__
# define LINUX_KERNEL
#elif !defined __PPC__ && !defined __leon__ && !defined ANDROID && \
    !defined WIN32 && !defined META  && !defined AVR && !defined __XTENSA__ && \
    !defined ARDUINO
#ifdef __gnu_linux__
# define LINUX_GNU
#endif
# define UNIX_GNU
#else
# define NO_X11
#endif

#endif /* FLAVOR_H */
