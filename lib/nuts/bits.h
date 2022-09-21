/* -*- tab-width: 8 -*- */
/** 
 *  manipulate bits and access memory with correct endianness	
 *
 *  \file      bits.h
 *  \author    Norbert Stoeffler
 *  \date      2001-2011
 *
 */

#ifndef BITS_H
#define BITS_H

/*****************************************************************************
 *  macros
 ****************************************************************************/

/** return a bitfield from a value
   \param v the value
   \param m msg
   \param l lsb
*/
#define BITS(v,m,l)	(((m)-(l)==31)?(v):((v)>>(l))&((1<<((m)-(l)+1))-1))

/** return a bitfield from a 64bit value
   \param v the value
   \param m msg
   \param l lsb
*/
#define BITS64(v,m,l)	(((m)-(l)==63)?(v):((v)>>(l))&((1ULL<<((m)-(l)+1))-1))

#undef BIT
/** return a bit from a value
   \param v the value
   \param b the bit position
*/
#define BIT(v,b)	(((v)>>(b))&1)

/** set bits inside a variable without touching the other bits
   \param i the variable to insert to
   \param m msb of the bitfield
   \param l lsb of the bitfield
   \param v value to inserted into the bitfield
 */
#define SETBITS(i,m,l,v)(((m)-(l)==31)?(i)=(v):\
			((i)&=~(((1<<((m)-(l)+1))-1)<<(l)),(i)|=(v)<<(l)))

/** set a single bit inside a variable without touching the other bits
   \param i the variable to insert to
   \param b the bit position
   \param v value to inserted into the bitfield
 */
#define SETBIT(i,b,v)	((i)&=~(1<<(b)),(i)|=(v)<<(b))

/** signextend the lowest w bits of a value v
   \param v the value to be extended
   \param w the current bitwidth
*/
#define SGNEXT(v,w)	(BIT(v,(w)-1)?~((1<<(w))-1)|(v):(v))

/* quick and dirty, this probably can be implemented more efficiently
 */
#define SBITS(v,m,l)	SGNEXT(BITS(v,m,l),(m)-(l)+1)


/* the following macros assume a little endian core at the moment. TODO: add
 * a variant for big endian cores
 */
#ifndef NUTS_BIG_ENDIAN

/** read a 16 bit value in big endian
 *  \param  a the source pointer (u8 *)
 */
#define BERU16(a) (*(((u8*)(a))+1) | *((u8*)(a)) << 8)

/** read a 16 bit signed value in big endian
 *  \param  a the source pointer (u8 *)
 */
#define BERS16(a) ((s16)(*(((u8*)(a))+1) | *((u8*)(a)) << 8))

/** read a 32 bit value in big endian
 *  \param  a the source pointer (u8 *)
 */
#define BERU32(a) ((u32)(*(((u8*)(a))+3) <<  0 | \
                         *(((u8*)(a))+2) <<  8 | \
		         *(((u8*)(a))+1) << 16 | \
                         *(((u8*)(a))+0) << 24))

/** read a 32 bit signed value in big endian
 *  \param  a the source pointer (u8 *)
 */
#define BERS32(a) ((s32)(*(((u8*)(a))+3) <<  0 | \
                         *(((u8*)(a))+2) <<  8 | \
		         *(((u8*)(a))+1) << 16 | \
                         *(((u8*)(a))+0) << 24))

/** write a 16 bit value in big endian
 *  \param  a the destination pointer (u8 *)
 *  \param  v the value to write
 */
#define BEW16(a,v) ({\
    *(((u8*)(a))+1) = ((v) >> 0)&0xff;\
    *(((u8*)(a))+0) = ((v) >> 8)&0xff;})

/** write a 32 bit value in big endian
 *  \param  a the destination pointer (u8 *)
 *  \param  v the value to write
 */
#define BEW32(a,v) do{\
    *(((u8*)(a))+3) = ((v) >>  0)&0xff;\
    *(((u8*)(a))+2) = ((v) >>  8)&0xff; \
    *(((u8*)(a))+1) = ((v) >> 16)&0xff; \
    *(((u8*)(a))+0) = ((v) >> 24)&0xff;}while(0)


/** read a 16 bit value in little endian
 *  \param  a the source pointer (u8 *)
 */
#define LERU16(a) (*(u16*)(a))

/** read a 16 bit signed value in little endian
 *  \param  a the source pointer (u8 *)
 */
#define LERS16(a) (*(s16*)(a))

/** read a 32 bit value in little endian
 *  \param  a the source pointer (u8 *)
 */
#define LERU32(a) (*(u32*)(a))

/** read a 32 bit signed value in little endian
 *  \param  a the source pointer (u8 *)
 */
#define LERS32(a) (*(s32*)(a))

/** write a 16 bit value in little endian
 *  \param  a the destination pointer (u8 *)
 *  \param  v the value to write
 */
#define LEW16(a,v) (*(u16*)(a)=(v))

/** write a 32 bit value in little endian
 *  \param  a the destination pointer (u8 *)
 *  \param  v the value to write
 */
#define LEW32(a,v) (*(u32*)(a)=(v))

#endif /* not NUTS_BIG_ENDIAN */

#endif /* BITS_H */
