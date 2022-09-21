/* -*- tab-width: 8 -*- */
/** 
 *  types and functions to manipulate various types of images on a very low
 *  level. pel pointers and strides are always in byte.
 *
 *  \file      pic.h
 *  \author    Norbert Stoeffler
 *  \date      200X
 *
 */

#ifndef PIC_H
#define PIC_H

#include  "basic.h"

/*****************************************************************************
 *  defines
 ****************************************************************************/

/** calculate the address of an 8bit pel by coordinates x,y
 *  save but slow, use only in e.g. algorithmic reference code
 */
#define pPEL8(ppic,x,y)     (\
    MUST_In_(y,0,(ppic)->Dy-1)\
    MUST_In_(x,0,(ppic)->Dx-1)\
  &(ppic)->Pel[(y)*(ppic)->S+(x)])

/** calculate the address of an 16bit pel by coordinates x,y
 *  save but slow, use only in e.g. algorithmic reference code
 */
#define pPEL16(ppic,x,y)    (\
  MUST_In_(y,0,(ppic)->Dy-1)\
  MUST_In_(x,0,(ppic)->Dx-1)\
  &(ppic)->Pel[(y)*(ppic)->S+2*(x)])

/** calculate the address of an 32bit pel by coordinates x,y
 *  save but slow, use only in e.g. algorithmic reference code
 */
#define pPEL32(ppic,x,y)    (\
  MUST_In_(y,0,(ppic)->Dy-1)\
  MUST_In_(x,0,(ppic)->Dx-1)\
  &(ppic)->Pel[(y)*(ppic)->S+4*(x)])


#define pPELCto32(ppic,x,y)    ({\
  MUST_In(y,0,(ppic)->Dy-1);\
  MUST_In(x,0,(ppic)->Dx-1);\
  (int*) &(ppic)->Pel[(y)*(ppic)->S+4*(x)];})


enum {
  PIC_NONE=0,
  PIC_G8,
  PIC_G16,
  PIC_G32,
  PIC_GU8,
  PIC_GU16,
  PIC_GU32,
  PIC_GS8,
  PIC_GS16,
  PIC_GS32,
  PIC_XRGB,
  PIC_FLOAT,
  PIC_YUV420SP
};


/*****************************************************************************
 *  types
 ****************************************************************************/

/** basic type to reference all single-channel or packed multi-channel images
    and pictures. can be used to describe whole images or rectangular windows
    inside of other images.
*/
typedef struct {
  u8    *Pel;		/**< pointer to topleft pixel */
  int   Dx;		/**< dimension in x (aka line length aka image width) */
  int	Dy;		/**< dimension in y (aka image height) */
  int   S;		/**< stride, byte distance in memory between the
			   beginning of two vertically adjacent lines */
} tPic;

/** YUV image with 3 planar components
 */
typedef struct {
  int   Dx,Dy;
  tPic  C[3];
} tYuv;

/** YUV image with a planar Y and a packed chroma component
 */
typedef struct {
  int   Dx,Dy;
  tPic  Y,C;
} tYc;


/*****************************************************************************
 *  exported functions
 ****************************************************************************/

EXTERN_C_BEGIN

void Pic_Free(tPic *pThat);
void Pic_Create(tPic *pThat, int S, int dx, int dy, void *dat);

void Pic8_Malloc(tPic *pThat, int dx, int dy);
void Pic8_MallocWithPad(tPic *pThat, int dx, int dy, int pad);
int  Pic8_Sad8(tPic *pThat, const tPic *pA, const tPic *pB, int factor);
int  Pic8_Sad16(tPic *pThat, const tPic *pA, const tPic *pB, int factor);
int  Pic8_Sad32(tPic *pThat, const tPic *pA, const tPic *pB, int factor);
void Pic8_Copy(tPic *pThat, tPic *pSrc);
void Pic8_Set(tPic *pThat, int val);
void Pic8_Pad(tPic *pThat, int pad);
void Pic8_Clear(tPic *pThat);

void Pic16_Malloc(tPic *pThat, int dx, int dy);
void Pic16_Pack4444(tPic *pThat, tPic *pSrc);

void Pic16_Copy(tPic *pThat, tPic *pSrc);
void Pic16_Set(tPic *pThat, int val);
void Pic16_CopyU32Shr(tPic *pThat, tPic *pSrc, int Shr);
void Pic16_Pack565_XRGB(tPic *pThat, tPic *pSrc);
void Pic16_Pack565_RGBX(tPic *pThat, tPic *pSrc);
void Pic16_BGRfromU8(tPic *pThat, tPic *pSrc);

void Pic32_CopyU16Shl(tPic *pThat, tPic *pSrc, int Shl);
void Pic8_CopyU16Shr(tPic *pThat, tPic *pSrc, int Shr);

void Pic8_ShiftLeft(tPic *pThat, int Shift);
void Pic16_ShiftLeft(tPic *pThat, int Shift);

void Pic32_Malloc(tPic *pThat, int dx, int dy);
void Pic32_GenAlpha(tPic *pThat, int r, int g, int b);
void Pic32_Copy(tPic *pThat, tPic *pSrc);
void Pic32_Set(tPic *pThat, u32 val);
void Pic32_UnPack565(tPic *pThat, tPic *pSrc);
void Pic32_RGBXfromU8(tPic *pThat, tPic *pSrc);
void Pic32_RGBXfromRGB(tPic *pThat, tPic *pSrc);
void Pic32_RGBXfromBGR(tPic *pThat, tPic *pSrc);
void Pic32_XBGRfromU8(tPic *pThat, tPic *pSrc);

void Pic24_Malloc(tPic *pThat, int dx, int dy);
void Pic24_Copy(tPic *pThat, tPic *pSrc);

void Pic24_RGBfromRGBX(tPic *pThat, tPic *pSrc);
void Pic24_BGRfromRGBX(tPic *pThat, tPic *pSrc);

void Yuv420_Malloc(tYuv *pThat, int dx, int dy);
bool Yuv_Load(tYuv *pThat, const char *Name);

void Yc420_Malloc(tYc *pThat, int dx, int dy);
void Yc_Free(tYc *pThat);
bool Yc_Load(tYc *pThat, const char *Name);
void Yc_Import(tYc *pThat, tYuv *pSrc);

bool Pic8_Save(const tPic *pThat, const char *Name);
bool Pic8_SaveA(const tPic *pThat, const char *Name);
bool Pic16_Save(const tPic *pThat, const char *Name);
bool Pic16_SaveA(const tPic *pThat, const char *Name);
bool Pic32_Save(const tPic *pThat, const char *Name);
bool Pic32_SaveA(const tPic *pThat, const char *Name);
bool Pic32_SaveRGB(const tPic *pThat, const char *Name);
bool Pic32_SaveRGBX(const tPic *pThat, const char *Name);
bool Pic32_SaveXRGB(const tPic *pThat, const char *Name);
bool Pic32_SaveBGRX(const tPic *pThat, const char *Name);
bool Pic32_SaveRgbA(const tPic *pThat, const char *Name);

bool Pic8_Load(tPic *pThat, const char *Name);
bool Pic8_LoadAln(tPic *pThat, const char *Name, int Aln);

bool Pic16_Load(tPic *pThat, const char *Name);
bool Pic16_LoadAln(tPic *pThat, const char *Name, int Aln);
bool Pic16_LoadShl(tPic *pThat, const char *Name, int Shift);
bool Pic16_UniLoad(tPic *pThat, const char *Name);

bool Pic32_Load(tPic *pThat, const char *Name);
bool Pic32_LoadXRGB(tPic *pThat, const char *Name);
bool Pic32_LoadRGBX(tPic *pThat, const char *Name);

int  Pic_FileType(const char *Name);

void Pic_HorFlip(tPic *pPic);

void Pic16_DrawRect(tPic *pThat, int x, int y, int color);
void Pic16_DrawLine(tPic *pThat, int ax, int ay,  int bx, int by, unsigned int color);
void Pic16_DrawCross(tPic *pThat, int x, int y, int color);

void Pic32_DrawRect(tPic *pThat, int x, int y, int color);
void Pic32_DrawLine(tPic *pThat, int ax, int ay,  int bx, int by, unsigned int color);


EXTERN_C_END


#endif /* PIC_H */
