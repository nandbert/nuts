/** 
 *  \file      debug.c
 *  \author    Norbert Stoeffler
 *  \date      2001-2011
 *
 */
//#define DLOGGING
#ifdef __KERNEL__
#include		<linux/module.h>
#include		<linux/kernel.h>
#else
#include		<stdio.h>
#include		<stdlib.h>
#include		<stdarg.h>
#include		<string.h>
#endif

#include		"debug.h"
#include		"strmem.h"

#ifdef LINUX_GNU
#include		<execinfo.h>
#include		<sys/stat.h>
#include		<signal.h>
#endif

#ifdef ANDROID
#include		<android/log.h>
#define TAG		"ARE"
#endif


/*****************************************************************************
 *  kernel dummies
 ****************************************************************************/

#ifdef LINUX_KERNEL
#define exit		do_exit
#define	signal(x,y)	MUST_MSG(0,"don't have this in kernel");
#endif


/*****************************************************************************
 *  local variables
 ****************************************************************************/

static void (*lMustFmt)(const char *file, int line, const char *msg)=NULL;
static void (*lMustExit)(void)=NULL;

/*****************************************************************************
 *  local defines
 ****************************************************************************/
#define BYTESWAPP32(v)	((((v)>>24)&0x000000ff) | \
                         (((v)>> 8)&0x0000ff00) | \
                         (((v)<< 8)&0x00ff0000) | \
                         (((v)<<24)&0xff000000))

/*****************************************************************************
 *  local functions
 ****************************************************************************/

/****************************************************************************/
/*  our signal handler
 */
#ifdef UNIX_GNU
static void SigHandler(int nSig)
{
  const char *s=StrGen("signal %d received",nSig);

  void db_stop(void);

  db_stop();
	
  switch(nSig)
  {
  case SIGSEGV:
	  s="segmentation fault";
	  break;
  case SIGBUS:
	  s="bus error";
	  break;
  case SIGABRT:
	  s="abort";
	  break;
  }
	
  ERROR("%s",s);
}
#endif


/****************************************************************************/
/*  show the full map of our process
 *
 */
void nuts_PrintMaps(void)
{
	FILE *f=fopen("/proc/self/maps","r");
	if(!f) ERROR("cannot open /proc/self/map");

	char	buf[256];

	while(fgets(buf,sizeof(buf),f))
		nuts_printf("%s",buf);
	
	fclose(f);
}


/****************************************************************************/
/*  get offset between text addresses in elf and in memory
 *
 *  \return the offset
 */
size_t nuts_GetTextStart(void)
{
	FILE *f=fopen("/proc/self/maps","r");
	if(!f){
		nuts_printf("cannot open /proc/self/map\n");
		return 0;
	}

#if 0
	// show first 30 lines of maps
	char	buf1[256];
	for(int i=0;(long)fgets(buf1,sizeof(buf1),f)!=EOF && i<30;i++)
		if(!strncmp(".elf",buf1+strlen(buf1)-5,4))
			printf("%s %s",buf1,buf1+strlen(buf1)-5);
	fclose(f);
#endif

	char	buf[256]={"0x"};
	int		i;
	for(i=0;(long)fgets(buf+2,sizeof(buf)-2,f)!=EOF;i++)
		if( !strncmp(".elf",buf+strlen(buf)-5,4) && strstr(buf,"r-xp") ){
			//printf("%s",buf);
			break;
		}
	fclose(f);

	size_t	off;
	sscanf(buf,"%p",(void**)&off);		DLOG("offset=%zx",off);

	// heuristic: if off=0x400000 then the code not position independent and
	// addresses match the elf file
	if(off==0x400000)	off=0;

	return off;
}


/****************************************************************************/
/*  print stacktrace if possible (linux only)
 *
 *  \param  start start with this entry in stracktrace
 *  \param  end   stop <end> entries before the end of the stracktrace
 *  \param  func  startaddress of the current function
 */
static void PrintStackTrace(int start, int end, bool func)
{
#ifdef LINUX_KERNEL
	dump_stack();
#endif
#ifdef LINUX_GNU
	void		*bta[128];
	int			btl,i;
	char		**bts,*s;
	char		exe[256];
	char		progname[256];
	const char	*pathes[]={"","/usr/local/bin/"};
	struct stat	sbuf;

	start+=2;  /* remove ourselves */
	btl=backtrace(bta,LEN(bta));

	size_t		off=nuts_GetTextStart();

	if(btl>2)
	{
		bts=backtrace_symbols(bta,btl);
		if( (s=strchr(bts[0],'(')) )
		{
			strncpy(exe,bts[0],s-bts[0]);
			exe[s-bts[0]]=0;
			for(i=0;i<LEN(pathes);i++)
			{
				strcpy(progname,pathes[i]);
				strcat(progname,exe);					DLOG("progname %s",progname);
				if(stat(progname,&sbuf)==0)
					break;
			}
			if(i<LEN(pathes))
			{
				if(func)
				{
					nuts_printf("function: "); nuts_flush();
					system(mustSG("addr2line -f -s -e %s %p | head -1 | c++filt",progname,(size_t)bta[2]-off));
				}
				nuts_printf("stacktrace:\n"); nuts_flush();
				for(i=start;i<btl-end;i++)
				{
					int add=(size_t)bta[i]-off;			DLOG("bta[i]=%zx, add=%x",bta[i],add);
					system(mustSG("addr2line -f -s -e %s %p | awk '{if($0!~\":\") f=$0; "
								  "else { gsub(\" .discri.*)\",\"\",$0); printf($0\": \"); "
								  "system(\"c++filt \"f\" | tr -d \\\\\\\\n\"); "
								  "printf(\"() @ 0x%x\\n\");}}'",progname,
								  add-2,add));
				}
			}
			else
			{
				nuts_printf("sorry, no stracktrace. cannot find executable '%s'\n",exe); nuts_flush();
			}
		}
		else
		{
			nuts_printf("sorry, no stracktrace. cannot guess executable name\n"); nuts_flush();
		}
	}
#if 0
	char              **bts;
	if(btl>0)
	{
		if(bts)
		{
			for(i=start;i<btl-end;i++)
				nuts_printf("%s\n",bts[i]);
		}
	}
#endif
#else
	(void)start; (void)end; (void)func;
#endif
}


/*****************************************************************************
 *  exported functions
 ****************************************************************************/

/****************************************************************************/
/** hook for debugger: always set a breakpoint here
 */
void db_stop(void);

void db_stop(void)
{
}


#ifdef ANDROID
/***************************************
 *  
 **************************************/
bool android_use_printf()
{
    static bool init=FALSE, useprintf=FALSE;
    const char	*s;

    if(!init)
    {
		s=getenv("ANDROID_PRINTF");
		useprintf= s && !strcmp(s,"1");
		init=TRUE;
    }

    return useprintf;
}

#endif


/****************************************************************************/
/**
 *
 *  \param
 *  \return
 */
#ifndef ARDUINO
int nuts_printf(const char *format, ...)
{
	int		r=0;
	va_list	args;

	va_start(args, format);
#ifdef LINUX_KERNEL
	r=vprintk(format, args);
#elif defined ANDROID
	if(android_use_printf())
		r=vprintf(format,args);
	if(*format!='\n')
		r=__android_log_vprint(ANDROID_LOG_DEBUG,TAG,format,args);
#else
	r=vprintf(format, args);
#endif
	va_end(args);

	return r;
}
#endif

/****************************************************************************/
/**
 *
 *  \param
 *  \return
 */
void nuts_flush(void)
{
#ifdef LINUX_KERNEL
    // no flush required
#elif defined ANDROID
    if(android_use_printf())
	fflush(stdout);   
#else
    fflush(stdout);
#endif
}


/****************************************************************************/
/** set a different handler that is used when MUST detects an error
 *
 *  \param  pMustFmt print the error message
 *  \param  pMustExit finally call this function to abort
 */
void MUST_SetHandler(void *pMustFmt, void *pMustExit)
{
  lMustFmt=(void (*)(const char *file, int line, const char *msg))pMustFmt;
  lMustExit=(void (*)(void))pMustExit;
}


/****************************************************************************/
/*  assertion with emacs compile-mode compliant output
 *
 *  \param  file    source file
 *  \param  line    source line
 *  \param  msg     string, explaining the problem
 */
int mustImpl(const char *file, int line, const char *msg)
{
    static int			sExiting=0;

    if(msg&&!sExiting)
    {
	sExiting=1;
	if(lMustFmt)
	{
	    lMustFmt(file,line,msg);
	}
	else
	{
#ifdef ANDROID
	    if(android_use_printf())
			printf("\n%s:%d: %s\n",file,line,msg);

		__android_log_print(ANDROID_LOG_ERROR,TAG,"%s:%d: %s\n",file,line,msg);
#else
//			if(errno)
//				are_printf("\n%s:%d: %s (errno=%s)\n",file,line,msg,strerror(errno));
//			else
	    nuts_printf("\n%s:%d: %s\n",file,line,msg);
#endif
	}
	fflush(stdout);
	db_stop();
#ifdef LINUX_GNU
	PrintStackTrace(1,2,TRUE); // hide mustImpl() and the caller of main()
#endif
	if(lMustExit) lMustExit();
#ifdef ANDROID
	nuts_printf(DLOGTAG "commiting suicide: *0=0\n");
	*(u32*)0=0;
#endif
	exit(1);
	//abort();
    }
    return 0;
}


/****************************************************************************/
/** should never be called, used to tell the compiler that certain macros
 *  never return
 */
void mustNoRet(void)
{
	exit(0);
}


/****************************************************************************/
/*  private StrGen to avoid strange messages
 */
const char * mustSG(const char *fmt, ...)
{
  va_list       v_args;
  static char   buffer[1024];

  va_start(v_args, fmt);
  vsprintf( buffer,     fmt, v_args);
  MUST(strlen(buffer)<sizeof(buffer));
  va_end(v_args);

  return buffer;
}


/****************************************************************************/
/** External wrapper for internal PrintStackTrace
 */
void stackTrace(void)
{
  PrintStackTrace(0,2,FALSE);
}


/****************************************************************************/
/*  hexdump with strings
 *
 *  \param  pBase pointer to start of the memory
 *  \param  Offset sttaddress relative to pBase (justst for address listing)
 *  \param  Size in bytes
 *  \return 0
 */
int HexDump(const void *pBase, int Offset, int Size)
{
  const unsigned char	*pd= (const unsigned char*)pBase;
  int			i,a=Offset;

  ;   MUST(pBase||Offset);

  pd+=Offset;

  while(Size){
    nuts_printf(" 0x%04x: ",a);
    for(i=0;i<MIN(16,Size);i++)
      nuts_printf("%02x ",pd[i]);
    nuts_printf("  ");
    if(Size<16){
      nuts_printf("                                                "+3*Size);
      for(i=0;i<Size;i++,pd++,a++)
        nuts_printf("%c",(*pd>=0x20&&*pd<0x7f)?*pd:'\267');
      Size=0;
    }
    else{
      for(i=0;i<16;i++,pd++,a++)
        nuts_printf("%c",(*pd>=0x20&&*pd<0x7f)?*pd:'\267');
      Size-=16;
    }
    nuts_printf("\n");
  }
  return 0;
}


/****************************************************************************/
/** install a private segmentation fault handler
 */
void debugCatchSignals(void)
{
//	(void)SigHandler;
#ifdef UNIX_GNU
	signal(SIGSEGV, SigHandler);
	signal(SIGBUS , SigHandler);
	signal(SIGABRT, SigHandler);
#endif
}


/****************************************************************************/
/** dump rectangular memory area
 *
 *  \param start   address in dram
 *  \param stride
 *  \param dx,dy   dimension of the rectangle
 */
void debugDump(const void *start, int stride, int dx, int dy)
{
	u8                *pv;
	int               i;

	pv=(u8*) start;

	for(;dy>0;dy--){
		nuts_printf("\t%08x : ",pv);
		for(i=0;i<dx;i++)
			nuts_printf("%02x ",pv[i]);
		nuts_printf("\n");
		pv+=stride;
	}
}

/****************************************************************************/
/** dump rectangular memory area
 *
 *  \param start   address in dram
 *  \param stride
 *  \param dx,dy   dimension of the rectangle
 */
void debugDump32(void *start, int stride, int dx, int dy)
{
	u32               *pv;
	int               i;

	pv=(u32*) start;

	for(;dy>0;dy--){
		nuts_printf("\t%08x : ",pv);
		for(i=0;i<dx;i++)
			nuts_printf("%08x ",pv[i]);
		nuts_printf("\n");
		pv+=stride;
	}
}

/****************************************************************************/
/** dump rectangular memory area
 *
 *  \param start   address in dram
 *  \param stride
 *  \param dx,dy   dimension of the rectangle
 */
void debugDump32Swapped(void *start, int stride, int dx, int dy)
{
  u32               *pv;
  int               i;

  pv=(u32*) start;

  for(;dy>0;dy--){
    nuts_printf("\t");
    for(i=0;i<dx;i++)
      nuts_printf("%08x ",BYTESWAPP32(pv[i]));
    nuts_printf("\n");
    pv+=stride;
  }
}
