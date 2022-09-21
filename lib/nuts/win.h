/* -*- tab-width: 8 -*- */
/** 
 *  Windows that can show "Pics" and render line objects on top (debug
 *  visualization for image processing algos). In contrast to OpenCV, the
 *  line objects are not rendered into pixels, so they stay thin when
 *  zooming in (much better!!! ;-)
 *
 *  \file      win.h
 *  \author    Norbert Stoeffler
 *  \date      2001-2015
 *
 */
#ifndef WIN_H
#define WIN_H

#include	"pic.h"

/*****************************************************************************
 *  constants
 ****************************************************************************/

enum {
  NIL=-1,
  WIN_KEY_NONE=0,
  WIN_KEY_LEFT=-1,
  WIN_KEY_RIGHT=-2,
  WIN_KEY_UP=-3,
  WIN_KEY_DOWN=-4
};


/*****************************************************************************
 *  types
 ****************************************************************************/

/** a Window IMproved (compared to the older tWin)
 */
typedef struct sWin tWin;

/** a handler for mouseclicks */
typedef int (*Win_tClickHand)(void *pCtx, tWin *pWin, int X, int Y);

/** a handler for keyboard inputs */
typedef int (*Win_tKeyHand)(void *pCtx, tWin *pWin, int X, int Y, int Key);

/** a Window IMproved (compared to the older tWin)
 */
struct sWin {
  /** the name, read only */
  const char		*Name;
  /** dimensions, read only */
  int			Dx,Dy;

  /** a handler for mouseclicks, may be modified */
  struct {
    Win_tClickHand	pFunc;
    void		*pCtx;
  }			Click;

  /** a handler for clicks, may be modified */
  struct {
    Win_tKeyHand	pFunc;
    void		*pCtx;
  }			Key;

  struct Win_sX		*pX;
};


/*****************************************************************************
 *  macros
 ****************************************************************************/

/** set a handler and its context pointer together
 *  \param h the handler structure in the Win
 *  \param f the function of the handler
 *  \param c the context pointer of the handler (often a tPic)
 */
#define WIN_SETH(h,f,c)	({ (h).pFunc=(Win_tClickHand)(f); (h).pCtx=(c); })


/*****************************************************************************
 *  exported functions
 ****************************************************************************/

EXTERN_C_BEGIN

void Win_OpenXY(tWin *pThat, const char *Name, int Dx, int Dy);
tWin * Win_New(const char *Name, const tPic *pPic);
tWin * Win_NewXY(const char *name, int Dx, int Dy);
tWin * Win_NewZ(const char *name, const tPic *pPic, int Zoom);
tWin * Win_NewAZ(const char *name, const tPic *pPic);

void Win_TitleSet(tWin *pThat, const char *name);

void Win_Zoom(tWin *pThat, int Z, int X, int Y);
void Win_AutoZoom(tWin *pThat, bool On);

void * Win_Rect(tWin *pThat, int X, int Y, int Dx, int Dy, int Col);
void * Win_RectA(tWin *pThat, int X0, int Y0, int X1, int Y1, int Col);
void * Win_Line(tWin *pThat, int X0, int Y0, int X1, int Y1, int Col);
void * Win_Text(tWin *pThat, int X, int Y, int Col, const char *Text, ...);
void * Win_BigText(tWin *pThat, int X, int Y, int Col, const char *Text, ...);

void Win_Render(tWin *pThat);

void Win_Clear(tWin *pThat);
void Win_AutoClear(tWin *pThat, bool On);
void Win_ClearGfx(tWin *pThat);

void Win_Dump(tWin *pThat, tPic *pPic);

void Win_ShowU8(tWin *pThat, const tPic *pPic);
void Win_ShowS8(tWin *pThat, const tPic *pPic);
void Win_ShowU8Shl(tWin *pThat, const tPic *pPic, int Shift);
void Win_ShowU16(tWin *pThat, const tPic *pPic, int Shift);
void Win_ShowS16(tWin *pThat, const tPic *pPic, int Shift);
void Win_ShowA3(tWin *pThat, const tPic *pPic);
void Win_ShowU32(tWin *pThat, const tPic *pPic, int Shift);
void Win_ShowS32(tWin *pThat, const tPic *pPic, int Shift);
void Win_ShowXRGB(tWin *pThat, const tPic *pPic);
void Win_ShowRGBX(tWin *pThat, const tPic *pPic);
void Win_ShowBGRX(tWin *pThat, const tPic *pPic);
void Win_ShowBGR(tWin *pThat, const tPic *pPic);
void Win_ShowU16wDisp(tWin *pThat, const tPic *pPic, const tPic *pD, int DBits);
void Win_ShowRGB888(tWin *pThat, const tPic *pPic);
void Win_ShowRGB565(tWin *pThat, const tPic *pPic);
void Win_ShowRGB555(tWin *pThat, const tPic *pPic);

tWin * Win_ShowMemU8(const char *Name, void *pDat, int S, int Dx, int Dy, int Zoom);
tWin * Win_ShowMemRGBX(const char *Name, void *pDat, int S, int Dx, int Dy);

int  Win_Wait(void);
void Win_Refresh(void);

void Win_RectDel(tWin *pThat, void *pRect, int N);
void Win_LineDel(tWin *pThat, void *pLine, int N);

EXTERN_C_END

#endif /* WIN_H */
