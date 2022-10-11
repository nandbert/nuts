/* -*- tab-width: 8 -*- */
/** 
 *  debugging: handle programming- and runtime-errors, provide some nice
 *  debug-printf macros "DLOGd.."	
 *
 *  \file      debug.h
 *  \author    Norbert Stoeffler
 *  \date      198X
 *
 */
#ifndef NUTS_DEBUG_H
#define NUTS_DEBUG_H

#include	"basic.h"

/*****************************************************************************
 *  helpers
 ****************************************************************************/

#ifndef DLOGCOND
#define DLOGCOND	1
#endif

#define DLNUM2STR_(exp)	#exp
#define DLNUM2STR(exp)	DLNUM2STR_(exp)
#define DLNUM2SYM_(p,x)	p##x
#define DLNUM2SYM(p,x)	DLNUM2SYM_(p,x)
#define DLOGTAG		__FILE__ ":" DLNUM2STR(__LINE__) ": "

EXTERN_C_BEGIN

void MUST_SetHandler(void *pMustFmt, void *pMustExit);

const char * mustSG(const char *fmt, ...);
int mustImpl(const char *file, int line, const char *msg);
//void mustNoRet(void) __attribute__ ((noreturn));
int nuts_printf(const char *format, ...);
void nuts_flush(void);

EXTERN_C_END

/*****************************************************************************
 * macros for runtime errors, allways enabled, also in final version
 ****************************************************************************/

/** terminate program with nice message. use this for run-time errors (could
 *  not open file etc. final \n is already included.
 *  \param ... as nuts_printf
 */
#define ERROR(...)   mustImpl(__FILE__,__LINE__,mustSG("ERROR: " __VA_ARGS__))

#define ERROR2(f,l,...)  mustImpl(f,l,mustSG(__VA_ARGS__))

/** check a condition that may fail at run-time. an example would be an ISS
 *  encountering an illegal IW
 */
#ifndef NO_MUSTS
#define CHECK(x) do{  if(!(x)) ERROR("check of ("#x") failed!"); }while(0)
#else
#define CHECK(x) do{  if(!(x)) ERROR("check failed!"); }while(0)
#endif

/** print a nice warning message
 *  \param ... as nuts_printf
 */
#define WARN(...)   do{ nuts_printf(DLOGTAG"*** WARN: " __VA_ARGS__); \
	nuts_printf("\n"); }while(0)


/*****************************************************************************
 * macros for compile time checks, allways enabled
 ****************************************************************************/

#if 0 // before C++11
#define CTMUST(x)	struct DLNUM2SYM(ctmust,__LINE__) \
    	    	    	{ int condition_check[(x)?1:-1]; };
#else
#define CTMUST(x)	static_assert(x)
#endif

#ifdef WIN32
#define nuts_offset(s,m) 	((int)(&((s*)0)->m))
#else
#define nuts_offset(s,m) 	__builtin_offsetof(s,m)
#endif

/** check if two components of two structs are located at the same offset.
    useful for casting of struct pointers.
*/
#define CTMUST_EQOFF(a,ca,b,cb)	CTMUST(nuts_offset(a,ca)==nuts_offset(b,cb)) \


/** check if two components of two structs are "twins", i.e. at the same
    address offset and of the same size. useful for casting of struct pointers.
*/
#define CTMUST_TWIN(a,ca,b,cb)	CTMUST(nuts_offset(a,ca)==nuts_offset(b,cb) \
				&& sizeof(((a*)0)->ca)==sizeof(((b*)0)->cb))

/** manually check the structs layout
 */
#define CTMUST_LAY(a,ca,o,s)	CTMUST(nuts_offset(a,ca)==(o) && sizeof(((a*)0)->ca)==(s))

/*****************************************************************************
 * macros to find programming errors that should be fixed in the final
 * version and can be disabled with switch NO_MUSTS for faster execution
 * ("debug assertions")
 ****************************************************************************/

#ifndef NO_MUSTS

/** assert that condition is true
 */
#define MUST(cond)   mustImpl(__FILE__,__LINE__,(cond)?(const char*)NULL:\
                     "assertion of ("#cond") failed!")

/** assert that a==b
 */
#define MUST_Eq(a,b) mustImpl(__FILE__,__LINE__,((a)==(b))?(const char*)NULL:\
		     mustSG(#a"!="#b": "#a"=%d ($%x), "#b"=%d ($%x)",(a),(a),(b),(b)))

/** assert that a<=b
 */
#define MUST_Le(a,b) mustImpl(__FILE__,__LINE__,((a)<=(b))?(const char*)NULL:\
                     mustSG(#a">"#b": "#a"=%d ($%x), "#b"=%d ($%x)",(a),(a),(b),(b)))

/** assert that a<b
 */
#define MUST_Lt(a,b) mustImpl(__FILE__,__LINE__,((a)<(b))?(const char*)NULL:\
                     mustSG(#a">="#b": "#a"=%d ($%x), "#b"=%d ($%x)",(a),(a),(b),(b)))

/** assert that a>=b
 */
#define MUST_Ge(a,b) mustImpl(__FILE__,__LINE__,((a)>=(b))?(const char*)NULL:\
                     mustSG(#a"<"#b": "#a"=%d ($%x), "#b"=%d ($%x)",(a),(a),(b),(b)))

/** assert that a>b
 */
#define MUST_Gt(a,b) mustImpl(__FILE__,__LINE__,((a)>(b))?(const char*)NULL:\
                     mustSG(#a"<="#b": "#a"=%d ($%x), "#b"=%d ($%x)",(a),(a),(b),(b)))

/** assert that a>=b
 */
#define MUST_Gef(a,b) mustImpl(__FILE__,__LINE__,((a)>=(b))?(const char*)NULL:\
		      mustSG(#a"<"#b": "#a"=%f, "#b"=%f",(double)(a),(double)(b)))

/** assert that x is in range [a,b]
 */
#define MUST_In(x,a,b) mustImpl(__FILE__,__LINE__,((a)<=(x)&&(x)<=(b))?\
		     (const char*)NULL:mustSG(#x"=%d not in ["#a","#b"]"\
		     "=[%d,%d]",(x),(a),(b)))

/** assert that x is in range [a,b], 64bit version
 */
#define MUST_IN64(x,a,b) mustImpl(__FILE__,__LINE__,((u64)(a)<=(u64)(x)&&(u64)(x)<=(u64)(b))?\
		     (const char*)NULL:mustSG(#x"=%llx not in ["#a","#b"]"\
		     "=[%llx,%llx]",(u64)(x),(u64)(a),(u64)(b)))

/** assert that x isin the range that can be kept in an unsigned bitfield
    with b bits
 */
#define MUST_INUBITS(x,b) (\
  mustImpl(__FILE__,__LINE__,((u64)(x)<=(1LL<<(b))-1)? \
  (const char*)NULL:mustSG(#x"=%llx not %d bit range",(u64)(x),b)) \
)


/** assert that x isin the range that can be kept in a signed bitfield
    with b bits
 */
#define MUST_INSBITS(x,b) (\
  mustImpl(__FILE__,__LINE__,( -(1LL<<(b))<=(s64)(x) && (s64)(x)<=(1LL<<(b))-1 )? \
  (const char*)NULL:mustSG(#x"=%llx not %d bit range",(u64)(x),b)) \
)

/** assert that condition is true, but provide the error message manually in
  * nuts_printf format
  */
#define MUST_MSG(cond,...) mustImpl(__FILE__,__LINE__,(cond)?\
                     (const char*)NULL:mustSG(__VA_ARGS__))

/** like MUST, but with a comma-operator at the right side. can be smuggled
  * into expressions
  */
#define MUST_(cond)		MUST(cond),
#define MUST_In_(x,a,b)		MUST_In(x,a,b),
#define MUST_MSG_(cond,...)	MUST_MSG(cond,__VA_ARGS__),

#else /* NO_MUSTS defined, all empty */

#define MUST(cond)
#define MUST_Eq(a,b)
#define MUST_Le(a,b)
#define MUST_Lt(a,b)
#define MUST_Ge(a,b)
#define MUST_Gt(a,b)
#define MUST_In(x,a,b)
#define MUST_MSG(cond,...)
#define MUST_(cond)
#define MUST_In_(x,a,b)
#define MUST_MSG_(cond,...)
#define MUST_INUBITS(x,b)
#define MUST_INSBITS(x,b)

#endif

/** default of a switch that handles all defined values of i. always active
    as there is no runtime penalty
 */
#define MUST_UNDEF(i) do{mustImpl(__FILE__,__LINE__,\
			 mustSG("case %d unhandled in switch",(i)));\
	/*mustNoRet(); trick the compiler */}while(0)



/*****************************************************************************
 * macros for "debug logging", i.e. nuts_printfs that can be quickly enabled and
 * disabled for a source file. has to be explicitely enabled for each source
 * file. files that use it should always be checked in with switch disabled
 ****************************************************************************/

#ifdef DLOGGING
#ifndef META

//#ifndef ANDROID
#define DLOGCR		nuts_printf("\n"); nuts_flush();
//#else
//#define DLOGCR
//#endif


/** log an expression, format string is generated internally
 */
#define DLOGd(exp)    do{if(DLOGCOND){nuts_printf(DLOGTAG"%s=%d",#exp,(int)(exp)); \
			DLOGCR; }}while(0)

/** log an expression in hex, format string is generated internally
 */
#define DLOGx(exp)   do{if(DLOGCOND){nuts_printf(DLOGTAG"%s=0x%x",#exp,(int)(exp));\
			DLOGCR; }}while(0)

/** log an expression in hex, format string is generated internally
 */
#define DLOG64d(exp)   do{if(DLOGCOND){nuts_printf(DLOGTAG"%s=0x%lld",#exp,(s64)(exp));\
	                  DLOGCR; }}while(0)

/** log an expression in hex, format string is generated internally
 */
#define DLOG64x(exp)   do{if(DLOGCOND){nuts_printf(DLOGTAG"%s=0x%llx",#exp,(u64)(exp));\
	                  DLOGCR; }}while(0)

/** log a pointer, format string is generated internally
 */
#define DLOGp(exp)   do{if(DLOGCOND){nuts_printf(DLOGTAG"%s=0x%p",#exp,(exp));\
			DLOGCR; }}while(0)

/** log an expression in float, format string is generated internally
 */
#define DLOGf(exp)   do{if(DLOGCOND){nuts_printf(DLOGTAG"%s=%f",#exp,(double)(exp));\
	                DLOGCR; }}while(0)

/** log an expression that is a c++ class, i.e. has a print method
 */
#define DLOGc(exp)   do{if(DLOGCOND){nuts_printf(DLOGTAG"%s=",#exp); (exp).print(stdout); \
			DLOGCR; }}while(0)

/** log an expression that is a struct and has a print method
 */
#define DLOGs(str,exp) do{if(DLOGCOND){nuts_printf(DLOGTAG"%s=",#exp); str##_print(&(exp)); \
			DLOGCR; }}while(0)

/** log with format, like printf
 */
#define DLOG(...)   do{if(DLOGCOND){nuts_printf(DLOGTAG __VA_ARGS__); DLOGCR; }}	\
			while(0)

/** log a stack trace
 */
#define DSTACK  do{if(DLOGCOND){nuts_printf(DLOGTAG"stack trace\n"); stackTrace(); }}while(0)

/** dump the memory according to expression, format string etc is generated
  * internally
  */
#define DDUMP(exp)   ({if(DLOGCOND){nuts_printf(DLOGTAG"dump of: %s =\n",#exp);\
	fDebugDump(&(exp),0,sizeof(exp)); nuts_flush(); }})

/** dump extended (rectangular memory area with stride).
 *  \param mem
 *  \param st	stride
 *  \param dx	width
 *  \param dy	height
 */
#define DDUMPX(mem,st,dx,dy) ({if(DLOGCOND){nuts_printf(DLOGTAG"dump of: %s/%d (%d,%d) =\n",#mem,st,dx,dy); \
		        debugDump(mem,st,dx,dy); nuts_flush(); }})

#define DDUMP32X(mem,st,dx,dy) ({if(DLOGCOND){nuts_printf(DLOGTAG"dump of: %s/%d (%d,%d) =\n",#mem,st,dx,dy); \
		debugDump32(mem,st,dx,dy); nuts_flush(); }})

/** C++ version of DLOG to utilize overloaded << operators of fancy classes
 */
#define DOUT(exp)    do{if(DLOGCOND){nuts_printf(DLOGTAG"%s=\n",#exp); std::cout<<exp; DLOGCR; }}while(0)

#else // META

#include	<meta/tblog.h>

#define DLOG			TBLOG
#define DLOGd			TBLOGd
#define DLOGx			TBLOGx
#define DLOGp			TBLOGx
#define DLOGs(str,exp)
#define DDUMPX			TBDUMPX

#endif // META

#else /* ! DLOGGING all empty */
#define DLOGd(exp)
#define DLOGx(exp)
#define DLOG64d(exp)
#define DLOG64x(exp)
#define DLOGp(exp)
#define DLOGf(exp)
#define DLOGc(exp)
#define DLOGs(str,exp)
#define DLOG(...)
#define DSTACK
#define DDUMP(exp)
#define DDUMPX(mem,st,dx,dy)
#define DLOG_BEGIN
#define DLOG_END
#define DOUT(exp)
#endif

#ifndef assert
#define assert(x) MUST(x)
#endif

/*****************************************************************************
 *  very simple timing instrumentation
 ****************************************************************************/

#if defined TLOGGING

#include		"timer.h"

static tTimer	TLtimer;
extern tTimer	g_TLstamp;
extern bool	g_TLinit;

#define TLSTART	startTimer(&TLtimer);

#define TLOG(text) do{ double d=stopTimer(&TLtimer); \
	nuts_printf(DLOGTAG"TLOG(%s): %.1f ms\n",(text),d); nuts_flush(); }while(0)
								    

#define TSLOG(text) do{ double d; if(!g_TLinit) { startTimer(&g_TLstamp); g_TLinit=TRUE; } \
    d=stopTimer(&g_TLstamp); nuts_printf(DLOGTAG"TIMESTAMP(%s): %d ms\n",d); \
    nuts_flush(); }while(0)

#else

#define TLSTART
#define TLOG(text)
#define TSLOG(text)

#endif

/*****************************************************************************
 *  exported functions
 ****************************************************************************/

EXTERN_C_BEGIN

void debugDump(const void *start, int stride, int dx, int dy);
void stackTrace(void);

void debugDump32(void *start, int stride, int dx, int dy);
void debugDump32Swapped(void *start, int stride, int dx, int dy);

void debugCatchSignals(void);

EXTERN_C_END

#endif /* DEBUG_H */
