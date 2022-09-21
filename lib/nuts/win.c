/* -*- tab-width: 8 -*- */
/** 
 *  \file      win.c
 *  \author    Norbert Stoeffler
 *  \date      2001-2011
 *
 */
//#define DLOGGING

#include	"win.h"
#include	"debug.h"
#include	"list.h"
#include	"bits.h"
#ifndef NO_X11
#include	<X11/Xlib.h>
#include	<X11/Xutil.h>
#include	<X11/Xatom.h>
#include	<X11/keysym.h>
#include	<unistd.h>
#include	<string.h>
#include	<stdio.h>
#include	<stdarg.h>
#include	<malloc.h>


/*****************************************************************************
 *  local types
 ****************************************************************************/

typedef struct Win_sX {
  tNode			Node;
  tWin			*pThat;
  Window		Win;
  GC			Gc;
  XImage		*XImage;
  Pixmap		Pixmap;
  u8			*Buf;
  u8			*Buf2;
  int			X,Y,Z;
  tLnkList		Rects;
  tLnkList		Lines;
  tLnkList		Texts;
  bool			Redraw;
  bool			AutoZoom;
  bool			FreeGfx;
} Win_tX;

typedef struct {
  tNode			Node;
  int			X,Y,Dx,Dy;
  int			Col;
} tRect;

typedef struct {
  tNode			Node;
  int			X0,Y0,X1,Y1;
  int			Col;
} tLine;


typedef struct {
  tNode			Node;
  int			X,Y,Len,Col;
  bool			Big;
  const char		*Text;
} tText;


/*****************************************************************************
 *  local variables
 ****************************************************************************/

static Display		*lDisplay=NULL;
static Font		lFont,lBigFont;
static int		lDepth;
static int		lBpl;
static tLnkList		lWinList;


/*****************************************************************************
 *  local functions
 ****************************************************************************/

/****************************************************************************/
/** search through the list of Wins for the one corresponding to an X Window
 *  id
 *
 *  \param Win the X id
 *  \return pointer to the Win
 */
static tWin * FindWin(Window Win)
{
  Win_tX	*px;

  LNKLIST_FOR(lWinList,px)
    if(px->Win==Win) break;
  MUST(px);

  return px->pThat;
}

/****************************************************************************/
/** generate the color according to display depth
 *
 *  \param pThat the Win
 */
static void mySetForeground(Display *display, GC gc, int col)
{
  int		xcol,r,g,b;
    
  switch(lDepth){
  case 32:
  case 24:
      xcol=col;
      break;
  case 16:
      r=(col>>16)&0xff;
      g=(col>> 8)&0xff;
      b=(col>> 0)&0xff;
      r=CLIP(r>>3,0,31); g=CLIP(g>>2,0,63); b=CLIP(b>>3,0,31);
      xcol=(r<<11)|(g<<5)|(b<<0);
      break;
  }
  XSetForeground(display,gc,xcol);
}

/****************************************************************************/
/** redraw a window i.e. put the bitmap and the draw all attached graphics
 *  primitives
 *
 *  \param pThat the Win
 */
static void Redraw(tWin *pThat)
{
  tRect		*pr;
  tLine		*pl;
  tText		*pt;

  XPutImage(lDisplay,pThat->pX->Win,pThat->pX->Gc,pThat->pX->XImage,
	    0,0,0,0,pThat->Dx,pThat->Dy);

  LNKLIST_FOR(pThat->pX->Rects,pr){
    mySetForeground(lDisplay,pThat->pX->Gc,pr->Col);
    XDrawRectangle(lDisplay,pThat->pX->Win,pThat->pX->Gc,
		   (pr->X-pThat->pX->X)*pThat->pX->Z,
		   (pr->Y-pThat->pX->Y)*pThat->pX->Z,
		   pr->Dx*pThat->pX->Z,
		   pr->Dy*pThat->pX->Z);
  }

  LNKLIST_FOR(pThat->pX->Lines,pl){
    mySetForeground(lDisplay,pThat->pX->Gc,pl->Col);
    XDrawLine(lDisplay,pThat->pX->Win,pThat->pX->Gc,
	      (pl->X0-pThat->pX->X)*pThat->pX->Z+pThat->pX->Z/2,
	      (pl->Y0-pThat->pX->Y)*pThat->pX->Z+pThat->pX->Z/2,
	      (pl->X1-pThat->pX->X)*pThat->pX->Z+pThat->pX->Z/2,
	      (pl->Y1-pThat->pX->Y)*pThat->pX->Z+pThat->pX->Z/2);
  }

  LNKLIST_FOR(pThat->pX->Texts,pt){
    mySetForeground(lDisplay,pThat->pX->Gc,pt->Col);
    XSetFont(lDisplay,pThat->pX->Gc,pt->Big?lBigFont:lFont);
    XDrawString(lDisplay,pThat->pX->Win,pThat->pX->Gc,pt->X,pt->Y,
		pt->Text,pt->Len);
  }

#if 1
  /* copy in pixmap */
  XPutImage(lDisplay,pThat->pX->Pixmap,pThat->pX->Gc,pThat->pX->XImage,
	    0,0,0,0,pThat->Dx,pThat->Dy);

  LNKLIST_FOR(pThat->pX->Rects,pr){
    mySetForeground(lDisplay,pThat->pX->Gc,pr->Col);
    XDrawRectangle(lDisplay,pThat->pX->Pixmap,pThat->pX->Gc,
		   (pr->X-pThat->pX->X)*pThat->pX->Z,
		   (pr->Y-pThat->pX->Y)*pThat->pX->Z,
		   pr->Dx*pThat->pX->Z,
		   pr->Dy*pThat->pX->Z);
  }

  LNKLIST_FOR(pThat->pX->Lines,pl){
    mySetForeground(lDisplay,pThat->pX->Gc,pl->Col);
    XDrawLine(lDisplay,pThat->pX->Pixmap,pThat->pX->Gc,
	      (pl->X0-pThat->pX->X)*pThat->pX->Z+pThat->pX->Z/2,
	      (pl->Y0-pThat->pX->Y)*pThat->pX->Z+pThat->pX->Z/2,
	      (pl->X1-pThat->pX->X)*pThat->pX->Z+pThat->pX->Z/2,
	      (pl->Y1-pThat->pX->Y)*pThat->pX->Z+pThat->pX->Z/2);
  }

  LNKLIST_FOR(pThat->pX->Texts,pt){
    mySetForeground(lDisplay,pThat->pX->Gc,pt->Col);
    XSetFont(lDisplay,pThat->pX->Gc,pt->Big?lBigFont:lFont);
    XDrawString(lDisplay,pThat->pX->Pixmap,pThat->pX->Gc,pt->X,pt->Y,
		pt->Text,pt->Len);
  }
#endif

  XFlush(lDisplay);

  pThat->pX->Redraw=FALSE;
}


/****************************************************************************/
/** free all graphics primitives attached to a window
 *
 *  \param pThat the Win
 *  \return
 */
static void FreeGfx(tWin *pThat)
{
  tText		*pt;

  LnkList_Free(&pThat->pX->Rects);
  LnkList_Free(&pThat->pX->Lines);

  LNKLIST_FOR(pThat->pX->Texts,pt)
    free((char*)pt->Text);
  LnkList_Free(&pThat->pX->Texts);
}


/****************************************************************************/
/** apply the zoom factor to the pixel buffers
 *
 *  \param the Win
 */
static void Zoom(tWin *pThat)
{
  int	        ox,oy,x,y,z,b,xi,yi;

  z=pThat->pX->Z;
  ox=pThat->pX->X;
  oy=pThat->pX->Y;

  DLOGd(z);
  if(z==0)
    memcpy(pThat->pX->Buf,pThat->pX->Buf2,pThat->Dx*pThat->Dy*lBpl);
  else{
    for(y=0;y<pThat->Dy/z;y++)
      for(x=0;x<pThat->Dx/z;x++)
	for(yi=0;yi<z;yi++)
	for(xi=0;xi<z;xi++)
	  for(b=0;b<lBpl;b++)
	    pThat->pX->Buf[((y*z+yi)*pThat->Dx+(x*z+xi))*lBpl+b]=
	      pThat->pX->Buf2[((y+oy)*pThat->Dx+x+ox)*lBpl+b];
  }
  Redraw(pThat);
}


/****************************************************************************/
/** copy the back buffer to a second one (used when the main buffer is
 *  changed by zoom)
 *
 *  \param the Win
 */
static void Backup(tWin *pThat)
{
  memcpy(pThat->pX->Buf2,pThat->pX->Buf,pThat->Dx*pThat->Dy*lBpl);
}


/****************************************************************************/
/** apply the autozoom: if enabled go through autozoom list and zoom all,
 *  otherwise only zoom pThat
 *
 *  \param pThat the Win
 */
static void AutoZoom(tWin *pThat)
{
  int	        x,y,z;
  Win_tX	*px;

  if(pThat->pX->AutoZoom){
    z=pThat->pX->Z;
    x=pThat->pX->X;
    y=pThat->pX->Y;

    LNKLIST_FOR(lWinList,px){
      if(px->AutoZoom){
	if(px!=pThat->pX){
	  if(px->Z==1)
	    Backup(px->pThat);
	  px->Z=z;
	  px->X=x;
	  px->Y=y;
	}
	Zoom(px->pThat);
      }
    }
  }
  else
    Zoom(pThat);

  (void)AutoZoom;
}


/****************************************************************************/
/** handle an X event
 *
 *  \param  pEv pointer to the event
 *  \return the returncode of an installed handler or NIL
 */
static int HandleEvent(XEvent *pEv)
{
  int			result=NIL;
  tWin			*pThat;
  double		z;
  char			buffer[20];
  int			cc,key;
  KeySym		keysym;
  XComposeStatus	compose;

  /* dispatch event
   */
  switch(pEv->type){
  case ButtonPress:
    DLOGd(pEv->xbutton.button);
    pThat=FindWin(pEv->xany.window);
    switch(pEv->xbutton.button){
    case 1:
      if(pThat->Click.pFunc)
	result=(pThat->Click.pFunc)
	  (pThat->Click.pCtx,pThat,
	   CLIP(pEv->xbutton.x/pThat->pX->Z+pThat->pX->X,0,pThat->Dx-1),
	   CLIP(pEv->xbutton.y/pThat->pX->Z+pThat->pX->Y,0,pThat->Dy-1));
      break;
    case 3:
      return -2;
    case 4:
      if(pThat->pX->Z==1)
	Backup(pThat);
      z=pThat->pX->Z;
      if(pEv->xbutton.x<8)		pEv->xbutton.x=0;
      if(pEv->xbutton.x>pThat->Dx-9)	pEv->xbutton.x=pThat->Dx;
      if(pEv->xbutton.y<8)		pEv->xbutton.y=0;
      if(pEv->xbutton.y>pThat->Dy-9)	pEv->xbutton.y=pThat->Dy;
      /* [nost: 724] */
      pThat->pX->X+=pEv->xbutton.x/(z*(z+1));
      pThat->pX->Y+=pEv->xbutton.y/(z*(z+1));
      pThat->pX->Z++;
      AutoZoom(pThat);
      break;
    case 5:
      if(pThat->pX->Z>1){
	if(pThat->pX->Z==2){
	  pThat->pX->X=0;
	  pThat->pX->Y=0;
	  pThat->pX->Z=1;
	}
	else{
	  z=pThat->pX->Z;
	  /* [nost: 724] */
	  pThat->pX->X=MAX(0,pThat->pX->X-pEv->xbutton.x/(z*(z-1)));
	  pThat->pX->Y=MAX(0,pThat->pX->Y-pEv->xbutton.y/(z*(z-1)));
	  pThat->pX->Z--;
	}
	AutoZoom(pThat);
      }
      break;
    }
    DLOGd(pThat->pX->Z);
    DLOGd(pThat->pX->X);
    DLOGd(pThat->pX->Y);
    break;

  case KeyPress:
    pThat=FindWin(pEv->xany.window);
    if(pThat->Key.pFunc){
      cc=XLookupString(&pEv->xkey,buffer,sizeof(buffer),&keysym,&compose);
      if(ISIN(keysym,XK_space,XK_asciitilde))
	key=buffer[0];
      else if(keysym==XK_Left)
	key=WIN_KEY_LEFT;
      else if(keysym==XK_Right)
	key=WIN_KEY_RIGHT;
      else if(keysym==XK_Up)
	key=WIN_KEY_UP;
      else if(keysym==XK_Down)
	key=WIN_KEY_DOWN;
      else
	key=WIN_KEY_NONE;

      if(key!=WIN_KEY_NONE)
	result=pThat->Key.pFunc(pThat->Key.pCtx,pThat,
				pEv->xbutton.x/pThat->pX->Z+pThat->pX->X,
				pEv->xbutton.y/pThat->pX->Z+pThat->pX->Y,
				key);
    }
    break;
  }

  return result;
}


/****************************************************************************/
/**  default click handler
 *
 *  \param pThat our pic
 *  \param pWin  the window
 *  \param X,Y   coordinates of the click
 *  \return      NIL
 */
static int Click(void *pUndef, tWin *pWin, int X, int Y)
{
    printf("(x,y)= %3d,%3d\n",X,Y);

    return NIL;
}
#endif

/*****************************************************************************
 *  exported functions
 ****************************************************************************/

/****************************************************************************/
/** open (i.e. create from scratch) a window
 *
 *  \param pThat the Win
 *  \param Name	 initial name of the window
 *  \param Dx,Dy dimensions
 *  \return
 */
void Win_OpenXY(tWin *pThat, const char *Name, int Dx, int Dy)
{
#ifndef NO_X11
  unsigned long white, black;
  Window        root;
  int		screen;
  XGCValues	gcv;

  MUST_In(Dx,4,4096); MUST_In(Dy,1,2048);

  if(!lDisplay){
    lDisplay=XOpenDisplay(NULL);
    if(!lDisplay)    ERROR("Can't open display");

    lDepth=DefaultDepth(lDisplay,DefaultScreen(lDisplay));
    switch(lDepth){
    case 32:
    case 24:
      lBpl=4;
      break;
    case 16:
      lBpl=2;
      break;
    case 15:
    default:
      ERROR("unsupported depth %d",lDepth);
      break;
    }

    lFont=0;
    lBigFont=0;
//    lFont=XLoadFont(lDisplay,"9x15");
    lFont=XLoadFont(lDisplay,"fixed");
    lBigFont=XLoadFont(lDisplay,"-*-*-*-r-*-*-34-*-*-*-*-*-*-*");

    LnkList_Init(&lWinList);
  }

  screen =	DefaultScreen(lDisplay);
  root =	RootWindow(lDisplay,screen);
  white =	WhitePixel(lDisplay,screen);
  black =	BlackPixel(lDisplay,screen);

  memset(pThat,0,sizeof(*pThat));

  pThat->pX=NEW(Win_tX);
  pThat->pX->pThat=pThat;
  LnkList_Init(&pThat->pX->Rects);
  LnkList_Init(&pThat->pX->Lines);
  LnkList_Init(&pThat->pX->Texts);

  LnkList_Add(&lWinList,pThat->pX);

  pThat->Name=strdup(Name);
  pThat->Dx=Dx;
  pThat->Dy=Dy;

  pThat->pX->Win=XCreateSimpleWindow(lDisplay,root,0,0,Dx,Dy,0,
				     black,white);
  MUST(pThat->pX->Win);

  XSelectInput(lDisplay,pThat->pX->Win, 0
	       | ExposureMask
	       | ButtonPressMask
//	       | PointerMotionMask
	       | KeyPressMask
	       );

  XSetStandardProperties(lDisplay,pThat->pX->Win,Name,
			 Name,None,NULL,0,0);

  XMapWindow(lDisplay,pThat->pX->Win);

  pThat->pX->Gc=XCreateGC(lDisplay,pThat->pX->Win,0,&gcv);

  XFlush(lDisplay);

  pThat->pX->X=pThat->pX->Y=0;
  pThat->pX->Z=1;

  pThat->pX->Buf=malloc(Dx*Dy*lBpl);  MUST(pThat->pX->Buf);
  pThat->pX->Buf2=malloc(Dx*Dy*lBpl);  MUST(pThat->pX->Buf2);
  pThat->pX->XImage=XCreateImage(lDisplay,CopyFromParent,lDepth,ZPixmap,0,
				 (char*)(pThat->pX->Buf),pThat->Dx,pThat->Dy,
				 lBpl*8,lBpl*pThat->Dx);

  pThat->pX->Pixmap=XCreatePixmap(lDisplay,pThat->pX->Win,
				  pThat->Dx,pThat->Dy,lDepth);

  pThat->pX->FreeGfx=TRUE;

  WIN_SETH(pThat->Click,Click,NULL);
#endif
}

/****************************************************************************/
/** set window title
 *
 * \param pThat the Win
 * \param name  new window title
 */
void Win_TitleSet(tWin *pThat, const char *Name)
{
#ifndef NO_X11
  XTextProperty windowName;

  if(pThat->Name)
    free((char*)pThat->Name);
  pThat->Name=strdup(Name);

  windowName.value    = (unsigned char *)pThat->Name;
  windowName.encoding = XA_STRING;
  windowName.format   = 8;
  windowName.nitems   = strlen((char *) windowName.value);

  XSetWMName(lDisplay, pThat->pX->Win, &windowName);
#endif
}


/****************************************************************************/
/** set autozoom mode
 *
 * \param pThat the Win
 * \param On    turn it on or off
 */
void Win_AutoZoom(tWin *pThat, bool On)
{
#ifndef NO_X11
  pThat->pX->AutoZoom=On;
#endif
}


/****************************************************************************/
/** set autoclear mode
 *
 * \param pThat the Win
 * \param On    turn it on or off
 */
void Win_AutoClear(tWin *pThat, bool On)
{
#ifndef NO_X11
  pThat->pX->FreeGfx=On;
#endif
}


/****************************************************************************/
/** clear a window
 *
 *  \param pThat the Win
 */
void Win_ClearGfx(tWin *pThat)
{
#ifndef NO_X11
  ;   MUST(pThat);
  FreeGfx(pThat);
#endif
}


/****************************************************************************/
/** new window with dimensions Dx,Dy
 *
 *  \param name window name
 *  \param Dx,Dy dimensions
 *  \return pointer to the new Win
 */
tWin * Win_NewXY(const char *Name, int Dx, int Dy)
{
#ifndef NO_X11
  tWin	*pThat=NEW(tWin);

  Win_OpenXY(pThat,Name,Dx,Dy);

  return pThat;
#else
  return NULL;
#endif
}


/****************************************************************************/
/** new window that matches a pic
 *
 *  \param Name window name
 *  \param pPic pointer to pic from where the dimensions are derived
 *  \return pointer to the new Win
 */
tWin * Win_New(const char *Name, const tPic *pPic)
{
  return Win_NewXY(Name,pPic->Dx,pPic->Dy);
}


/****************************************************************************/
/** new window with initial zoom
 *
 *  \param  name window name
 *  \param  pPic pointer to pic from where the dimensions are derived
 *  \param  Zoom initial zoom
 *  \return pointer to the new Win
 */
tWin * Win_NewZ(const char *Name, const tPic *pPic, int Zoom)
{
#ifndef NO_X11
  tWin		*pThat;

  MUST_In(Zoom,1,16);

  pThat=Win_NewXY(Name,pPic->Dx*Zoom,pPic->Dy*Zoom);
  pThat->pX->Z=Zoom;

  return pThat;
#else
  return NULL;
#endif
}


/****************************************************************************/
/** new window with autozoom
 *
 *  \param  name window name
 *  \param  pPic pointer to pic from where the dimensions are derived
 *  \return pointer to the new Win
 */
tWin * Win_NewAZ(const char *Name, const tPic *pPic)
{
#ifndef NO_X11
  tWin		*pThat;

  pThat=Win_New(Name,pPic);
  pThat->pX->AutoZoom=TRUE;

  return pThat;
#else
  return NULL;
#endif
}


/****************************************************************************/
/** wait for and handle events. exit on right mouse click or if a handler
 *  returns a non-NIL value
 *
 *  \return the return value from a handler
 */
int Win_Wait()
{
#ifndef NO_X11
  int		result=NIL;
  XEvent	ev;
  Win_tX	*px;

  if(lDisplay){

    LNKLIST_FOR(lWinList,px)
      if(px->Redraw)
	Redraw(px->pThat);

    while(result==NIL){

      /* wait for next event
       */
      XNextEvent(lDisplay,&ev);    DLOGd(ev.type);

      do{
	if(ev.type==Expose)
	  FindWin(ev.xany.window)->pX->Redraw=TRUE;
	else
	  result=HandleEvent(&ev);
      }while(result==NIL && XCheckMaskEvent(lDisplay,0xffffffff,&ev));

      LNKLIST_FOR(lWinList,px)
	if(px->Redraw)
	  Redraw(px->pThat);
    }
  }

  return (result>=NIL)?result:NIL;
#else
  return NIL;
#endif
}


/****************************************************************************/
/** handle all pending events and then return immediately
 */
void Win_Refresh()
{
#ifndef NO_X11
  XEvent	ev;
  Win_tX	*px;

  if(!lDisplay) return;

  while(XCheckMaskEvent(lDisplay,0xffffffff,&ev)){
    if(ev.type==Expose){
      FindWin(ev.xany.window)->pX->Redraw=TRUE;
    }
    else
      HandleEvent(&ev);
  }

  LNKLIST_FOR(lWinList,px)
    if(px->Redraw)
      Redraw(px->pThat);
#endif
}


/****************************************************************************/
/** attach a new rectangle to a window
 *
 *  \param pThat     the Win
 *  \param X,Y,Dx,Dy dimensions of the rectangle
 *  \param Col       color as 24bit RGB value
 *  \return          handle that can be used to remove the rectangle again
 */
void * Win_Rect(tWin *pThat, int X, int Y, int Dx, int Dy, int Col)
{
#ifndef NO_X11
  tRect		*pr;

  ;   MUST(pThat);

  LnkList_Add(&pThat->pX->Rects,pr=NEW(tRect));
  pr->X=X;
  pr->Y=Y;
  pr->Dx=Dx;
  pr->Dy=Dy;
  pr->Col=Col;
  pThat->pX->Redraw=TRUE;

  return pr;
#else
  return NULL;
#endif
}


/****************************************************************************/
/** attach a new rectangle with absolute corner coordinates to a window
 *
 *  \param pThat       the Win
 *  \param X0,Y0,X1,Y1 corners of the rectangle
 *  \param Col         color as 24bit RGB value
 *  \return            handle that can be used to remove the rectangle again
 */
void * Win_RectA(tWin *pThat, int X0, int Y0, int X1, int Y1, int Col)
{
#ifndef NO_X11
  tRect		*pr;

  ;   MUST(pThat);

  LnkList_Add(&pThat->pX->Rects,pr=NEW(tRect));
  pr->X=X0;
  pr->Y=Y0;
  pr->Dx=X1-X0;
  pr->Dy=Y1-Y0;
  pr->Col=Col;
  pThat->pX->Redraw=TRUE;

  return pr;
#else
  return NULL;
#endif
}


/****************************************************************************/
/** remove N rectangles from a window
 *
 *  \param pThat the Win
 *  \param pRect handle of the _last_ rectangle to remove
 *  \param N     number of rectangles to remove
 */
void Win_RectDel(tWin *pThat, void *pRect, int N)
{
#ifndef NO_X11
  if(!pRect) return;

  ;   MUST(pThat);

  LnkList_FreeNodes(&pThat->pX->Rects,pRect,N);

  pThat->pX->Redraw=TRUE;
#endif
}


/****************************************************************************/
/** attach a new line to a window
 *
 *  \param pThat the Win
 *  \param X0,Y0 startpoint of the line
 *  \param X1,Y1 endpoint of the line
 *  \param Col   color as 24bit RGB value
 *  \return      handle that can be used to remove the line again
 */
void * Win_Line(tWin *pThat, int X0, int Y0, int X1, int Y1, int Col)
{
#ifndef NO_X11
  tLine		*pl;

  ;   MUST(pThat);

  LnkList_Add(&pThat->pX->Lines,pl=NEW(tLine));
  pl->X0=X0;
  pl->Y0=Y0;
  pl->X1=X1;
  pl->Y1=Y1;
  pl->Col=Col;
  pThat->pX->Redraw=TRUE;

  return pl;
#else
  return NULL;
#endif
}


/****************************************************************************/
/** remove N lines from a window
 *
 *  \param pThat the Win
 *  \param pLine handle of the _last_ line to remove
 *  \param N     number of lines to remove
 */
void Win_LineDel(tWin *pThat, void *pLine, int N)
{
#ifndef NO_X11
  if(!pLine) return;

  ;   MUST(pThat);

  LnkList_FreeNodes(&pThat->pX->Lines,pLine,N);

  pThat->pX->Redraw=TRUE;
#endif
}


/****************************************************************************/
/** attach a new text to a window
 *
 *  \param pThat the Win
 *  \param X,Y   bot left corner
 *  \param Col   color as 24bit RGB value
 *  \param Text	 a string (which is copied, so it can be a tmpstring)
 *  \return      handle that can be used to remove the text again
 */
void * Win_Text(tWin *pThat, int X, int Y, int Col, const char *Text, ...)
{
#ifndef NO_X11
  va_list       v_args;
  static char   buffer[1024];
  tText		*pt;

  va_start(v_args,Text);
  vsprintf(buffer,Text,v_args);
  MUST(strlen(buffer)<sizeof(buffer));
  va_end(v_args);

  ;   MUST(pThat);

  LnkList_Add(&pThat->pX->Texts,pt=NEW(tText));
  pt->X=X;
  pt->Y=Y;
  pt->Col=Col;
  pt->Big=FALSE;
  pt->Text=strdup(buffer);
  pt->Len=strlen(buffer);
  pThat->pX->Redraw=TRUE;

  return pt;
#else
  return NULL;
#endif
}


/****************************************************************************/
/** attach a new text to a window that is displayed in big letters (hopefully
 *  this works on other systems than mine :-)
 *
 *  \param pThat the Win
 *  \param X,Y   bot left corner
 *  \param Col   color as 24bit RGB value
 *  \param Text	 a string (which is copied, so it can be a tmpstring)
 *  \return      handle that can be used to remove the text again
 */
void * Win_BigText(tWin *pThat, int X, int Y, int Col, const char *Text, ...)
{
#ifndef NO_X11
  va_list       v_args;
  static char   buffer[1024];
  tText		*pt;

  va_start(v_args,Text);
  vsprintf(buffer,Text,v_args);
  MUST(strlen(buffer)<sizeof(buffer));
  va_end(v_args);

  ;   MUST(pThat);

  LnkList_Add(&pThat->pX->Texts,pt=NEW(tText));
  pt->X=X;
  pt->Y=Y;
  pt->Col=Col;
  pt->Big=TRUE;
  pt->Text=strdup(Text);
  pt->Len=strlen(Text);
  pThat->pX->Redraw=TRUE;

  return pt;
#else
  return NULL;
#endif
}


/****************************************************************************/
/** Flush all pending draws of graphic primitives. normally not needed
 *
 *  \param pThat the Win
 */
void Win_Render(tWin *pThat)
{
#ifndef NO_X11
  ;   MUST(pThat);

  Redraw(pThat);
#endif
}


/****************************************************************************/
/** change the zoom factor of a window
 *
 *  \param pThat the Win
 *  \param Z zoom factor (in [1,16])
 *  \param X,Y offset of the top left corner of the visible area
 */
void Win_Zoom(tWin *pThat, int Z, int X, int Y)
{
#ifndef NO_X11
  ;   MUST(pThat);  MUST_In(Z,1,16);
  pThat->pX->Z=Z;
  pThat->pX->X=X;
  pThat->pX->Y=Y;
#endif
}


/****************************************************************************/
/** clear a window
 *
 *  \param pThat the Win
 */
void Win_Clear(tWin *pThat)
{
#ifndef NO_X11
  ;   MUST(pThat);
  memset(pThat->pX->Buf,0,pThat->Dx*pThat->Dy*lBpl);
  FreeGfx(pThat);

  Redraw(pThat);
#endif
}


/****************************************************************************/
/** read a window content (including graphic primitives) back to a Pic U32
 *
 *  \param pThat the Win
 *  \param pPic	 a Pic
 */
void Win_Dump(tWin *pThat, tPic *pPic)
{
#ifndef NO_X11
  XImage	*xi;
  int		x,y;

  ;   MUST(pThat); MUST_Eq(pThat->Dx,pPic->Dx); MUST_Eq(pThat->Dy,pPic->Dy);

  if(pThat->pX->Redraw)
    Redraw(pThat);

  xi=XGetImage(lDisplay,pThat->pX->Pixmap,0,0,pThat->Dx,pThat->Dy,
	       AllPlanes,ZPixmap);

  DLOGd(xi->width);
  DLOGd(xi->height);
  DLOGd(xi->byte_order);
  DLOGd(xi->bitmap_unit);
  DLOGd(xi->bitmap_bit_order);
  DLOGd(xi->depth);
  DLOGd(xi->bytes_per_line);
  DLOGd(xi->bits_per_pixel);

  for(y=0;y<pThat->Dy;y++){
    for(x=0;x<pThat->Dx;x++){
      //BEW32(pPEL8(pPic,x*4,y),XGetPixel(xi,x,y));
      BEW32(pPEL32(pPic,x,y),((u32*)xi->data)[y*pThat->Dx+x]);
    }
  }

  XDestroyImage(xi);
#endif
}


/****************************************************************************/
/** show a pic with U8 pixels in grey
 *
 *  \param pThat the Win
 *  \param pPic  a Pic
 */
void Win_ShowU8(tWin *pThat, const tPic *pPic)
{
#define SHOW r=g=b=pp[x];
#include "win_tpl.c"
#undef SHOW
}


/****************************************************************************/
/** show a pic with S8 pixels. positive pixels are shaded in red, negatives
 *  in blue
 *
 *  \param pThat the Win
 *  \param pPic  a Pic
 */
void Win_ShowS8(tWin *pThat, const tPic *pPic)
{
    int		v;	(void)v;

#define SHOW v=(s8)pp[x]; if(v>0){r=v*2;b=0;}else{b=-v*2;r=0;};
#include "win_tpl.c"
#undef SHOW
}


/****************************************************************************/
/** show a pic with dark U8 pixels in grey
 *
 *  \param pThat the Win
 *  \param pPic  a Pic
 *  \param Shift to fit the pixels into 8bit
 */
void Win_ShowU8Shl(tWin *pThat, const tPic *pPic, int Shift)
{
#define SHOW r=g=b=pp[x]<<Shift;
#include "win_tpl.c"
#undef SHOW
}


/****************************************************************************/
/** show a pic with U16 pixels in grey
 *
 *  \param pThat the Win
 *  \param pPic  a Pic
 *  \param Shift to fit the pixels into 8bit
 */
void Win_ShowU16(tWin *pThat, const tPic *pPic, int Shift)
{
#define SHOW r=g=b=*(u16*)(pp+2*x)>>Shift;
#include "win_tpl.c"
#undef SHOW
}


/****************************************************************************/
/** show a pic with S16 pixels. positive pixels are shaded in red, negatives
 *  in blue
 *
 *  \param pThat the Win
 *  \param pPic  a Pic
 *  \param Shift to fit the pixels into 8bit
 */
void Win_ShowS16(tWin *pThat, const tPic *pPic, int Shift)
{
    int		v;	(void)v;

#define SHOW v=*(s16*)(pp+2*x)>>Shift;if(v>0){r=v;b=0;}else{b=-v;r=0;}
#include "win_tpl.c"
#undef SHOW
}


/****************************************************************************/
/** show a pic with U32 pixels in grey
 *
 *  \param pThat the Win
 *  \param pPic  a Pic
 *  \param Shift to fit the pixels into 8bit
 */
void Win_ShowU32(tWin *pThat, const tPic *pPic, int Shift)
{
#define SHOW r=g=b=(*(s32*)(pp+4*x))>>Shift;
#include "win_tpl.c"
#undef SHOW
}


/****************************************************************************/
/** show a pic with U32 pixels in grey
 *
 *  \param pThat the Win
 *  \param pPic  a Pic
 *  \param Shift to fit the pixels into 8bit
 */
void Win_ShowS32(tWin *pThat, const tPic *pPic, int Shift)
{
    int	v;	(void)v;
#define SHOW v=(*(s32*)(pp+4*x))>>Shift; if(v>0){r=v;b=0;}else{b=-v;r=0;} 
#include "win_tpl.c"
#undef SHOW
}


/****************************************************************************/
/** show a pic with U32 interpreting the bytes im memory as XRGB
 *
 *  \param pThat the Win
 *  \param pPic  a Pic
 */
void Win_ShowXRGB(tWin *pThat, const tPic *pPic)
{
#define SHOW r=pp[4*x+1]; g=pp[4*x+2]; b=pp[4*x+3];
#include "win_tpl.c"
#undef SHOW
}

/****************************************************************************/
/** show a pic with U32 interpreting the bytes im memory as RGBX
 *
 *  \param pThat the Win
 *  \param pPic  a Pic
 */
void Win_ShowRGBX(tWin *pThat, const tPic *pPic)
{
#define SHOW r=pp[4*x+0]; g=pp[4*x+1]; b=pp[4*x+2];
#include "win_tpl.c"
#undef SHOW
}

/****************************************************************************/
/** show a pic with U32 interpreting the bytes im memory as RGBX
 *
 *  \param pThat the Win
 *  \param pPic  a Pic
 */
void Win_ShowBGRX(tWin *pThat, const tPic *pPic)
{
#define SHOW r=pp[4*x+2]; g=pp[4*x+1]; b=pp[4*x+0];
#include "win_tpl.c"
#undef SHOW
}

/****************************************************************************/
/** show a pic with unaligned 3 byte pixels (OpenVINO!??)
 *
 *  \param pThat the Win
 *  \param pPic  a Pic
 */
void Win_ShowBGR(tWin *pThat, const tPic *pPic)
{
#define SHOW r=pp[3*x+2]; g=pp[3*x+1]; b=pp[3*x+0];
#include "win_tpl.c"
#undef SHOW
}

/****************************************************************************/
/** show a pic with U32 interpreting the values as 24 bit RGB
 *
 *  \param pThat the Win
 *  \param pPic  a Pic
 */
void Win_ShowRGB888(tWin *pThat, const tPic *pPic)
{
#define SHOW r=pp[3*x+0]; g=pp[3*x+1]; b=pp[3*x+2];
#include "win_tpl.c"
#undef SHOW
}

/****************************************************************************/
/** show a pic with U16 interpreting the values as 565 RGB
 *
 *  \param pThat the Win
 *  \param pPic  a Pic
 */
void Win_ShowRGB565(tWin *pThat, const tPic *pPic)
{
//#define SHOW {u16 pix = pp[2*x+0]<<8 | pp[2*x+1];
#define SHOW {u16 pix = ((u16*)pp)[x];	\
	r = (pix>>11)&0x1f; r<<=3;				\
	g = (pix>> 5)&0x3f; g<<=2;				\
	b = (pix>> 0)&0x1f; b<<=3;				\
}
#include "win_tpl.c"
#undef SHOW
}

/****************************************************************************/
/** show an U8 pic as abs/angle image with 16 directions color coded and
 *  brightness proportional to 3 bit length
 *
 *  \param pThat the Win
 *  \param pPic  a Pic
 */
void Win_ShowAA(tWin *pThat, const tPic *pPic)
{
#if 0
  /* rainbow */
  static int	lut[16][3]={
    {0xFF,0xFF,0xFF},
    {0xFF,0x00,0x00},
    {0xFF,0x00,0x80},
    {0xFF,0x00,0xC0},
    {0xA0,0x00,0xFF},
    {0x60,0x00,0xFF},
    {0x00,0x00,0xFF},
    {0x00,0x60,0xFF},
    {0x00,0xFF,0xFF},
    {0x00,0xFF,0x80},
    {0x00,0xFF,0x00},
    {0x80,0xFF,0x00},
    {0xE0,0xFF,0x00},
    {0xFF,0xA0,0x00},
    {0xFF,0x80,0x00},
    {0xFF,0x40,0x00},
  };
#else
  /* shuffled */
  static int	lut[16][3]={
    {0xFF,0xFF,0xFF},
    {0x00,0xFF,0xFF},
    {0xFF,0x00,0x00},
    {0x00,0xFF,0x80},
    {0xFF,0x00,0x80},
    {0x00,0xFF,0x00},
    {0xFF,0x00,0xC0},
    {0x80,0xFF,0x00},
    {0xA0,0x00,0xFF},
    {0xE0,0xFF,0x00},
    {0x60,0x00,0xFF},
    {0xFF,0xA0,0x00},
    {0x00,0x00,0xFF},
    {0xFF,0x80,0x00},
    {0x00,0x60,0xFF},
    {0xFF,0x40,0x00},
  };
#endif
  int		a,l;	(void)a; (void)l; (void)lut;

#define SHOW  a=pp[x]; l=(a>>4)&7; a&=0xf; \
  if(l){r=(lut[a][0]*l)>>3;g=(lut[a][1]*l)>>3;b=(lut[a][2]*l)>>3;}\
  else {r=g=b=0;}

#include "win_tpl.c"
#undef SHOW
}


/****************************************************************************/
/** show an U8 pic as angular image with 3 bit angle (i.e. the input for
 *  SHAP)
 *
 *  \param pThat the Win
 *  \param pPic  a Pic
 */
void Win_ShowA3(tWin *pThat, const tPic *pPic)
{
  static int	lut[16][3]={
//    {0xFF,0xFF,0xFF}, // white +
    {0x00,0xFF,0xFF}, // cyan +
    {0xFF,0x00,0x00}, // red +
    {0x00,0xFF,0x80}, // green 1
//    {0xFF,0x00,0x80}, // magenta 1
//    {0x00,0xFF,0x00}, // green 2 +
//    {0xFF,0x00,0xC0}, // magenta 2
//    {0x80,0xFF,0x00}, // green 3
    {0xc0,0x00,0xFF}, // violet +
    {0xE0,0xFF,0x00}, // yellow +
    {0x60,0x00,0xFF}, // dark violet
    {0xFF,0xA0,0x00}, // orange 1 +
    {0x00,0x00,0xFF}, // dark blue +
//    {0xFF,0x80,0x00}, // orange 2
//    {0x00,0x60,0xFF}, // blue
//    {0xFF,0x40,0x00}, // dark orange
  };
  int		a,l;	(void)a; (void)l; (void)lut;

#define SHOW  a=pp[x]; l=a&0x8; a&=0x7; \
  if(l){r=lut[a][0];g=lut[a][1];b=lut[a][2];}\
  else {r=g=b=0;}

#include "win_tpl.c"
#undef SHOW
}


/****************************************************************************/
/**
 */
tWin * Win_ShowMemU8(const char *Name, void *pDat, int S, int Dx, int Dy, int Zoom)
{
    tPic	pic;
    tWin	*pw;

    Pic_Create(&pic,S,Dx,Dy,pDat);
    Win_ShowU8(pw=Win_NewZ(Name,&pic,Zoom),&pic);

    return pw;
}


/****************************************************************************/
/**
 */
tWin * Win_ShowMemRGBX(const char *Name, void *pDat, int S, int Dx, int Dy)
{
    tPic	pic;
    tWin	*pw;

    Pic_Create(&pic,S,Dx,Dy,pDat);
    Win_ShowRGBX(pw=Win_New(Name,&pic),&pic);

    return pw;
}


