/* -*- tab-width: 8 -*- */
/** 
 *  types and functions to manipulate various types of images on a very low
 *  level. pel pointers and strides are always in byte.
 *
 *  \file      pic.c
 *  \author    Norbert Stoeffler
 *  \date      2001-2011
 *
 */

//#define DLOGGING

#include		"flavor.h"

#if defined UNIX_GNU || defined ANDROID
#include		<stdio.h>
#include		<string.h>
#include		<malloc.h>
#elif defined		LINUX_KERNEL
#include		<linux/module.h>
#include		<linux/kernel.h>
#else
#include		<stdio.h>
#include		<string.h>
#define free(x)		(MUST_MSG_(0,"don't have this on PPC")(void)(x))
#define calloc(s,d)	(MUST_MSG_(0,"don't have this on PPC")(void*)0)
#define	fread(a,b,c,d)	(MUST_MSG(0,"don't have this in kernel"))
#define	fwrite(a,b,c,d)	(MUST_MSG(0,"don't have this in kernel"))
#define	fgets(a,b,c)	(MUST_MSG(0,"don't have this in kernel"))
#define	fgetc(a)	(MUST_MSG(0,"don't have this in kernel"))
#define fclose(a)	(MUST_MSG(0,"don't have this in kernel"))
#define fprintf(...)	(MUST_MSG(0,"don't have this in kernel"))
#define fscanf(...)	(MUST_MSG(0,"don't have this in kernel"))
#define FILE		void
#endif

#include		"debug.h"
#include		"pic.h"
#include		"bits.h"


/*****************************************************************************
 *  local macros
 ****************************************************************************/

#define MUST_PLAUS(dx,dy) do{MUST_In(dx,1,8192); MUST_In(dx,1,8192);}while(0)

#define CERU32(a)	(*((u32*)(a)))
#define CERU16(a)	(*((u16*)(a)))
#define CEW32(a,v)	(*((u32*)(a))=(v))
#define CEW16(a,v)	(*((u16*)(a))=(v))


/*****************************************************************************
 *  local macros: kernel dummies
 ****************************************************************************/

#ifdef LINUX_KERNEL
#define free		kfree
#define calloc(s,d)	kmalloc(s,GFP_KERNEL)
#define	fopen(a,b)	(MUST_MSG(0,"don't have this in kernel"),NULL)
#define	fread(a,b,c,d)	MUST_MSG(0,"don't have this in kernel")
#define	fwrite(a,b,c,d)	MUST_MSG(0,"don't have this in kernel")
#define	fgets(a,b,c)	MUST_MSG(0,"don't have this in kernel")
#define	fgetc(a)	MUST_MSG(0,"don't have this in kernel")
#define fclose(a)	MUST_MSG(0,"don't have this in kernel")
#define fprintf(...)	MUST_MSG(0,"don't have this in kernel")
#define fscanf(...)	MUST_MSG(0,"don't have this in kernel")
#define FILE		void
#define stdin		NULL
#define EOF		0
#endif


/*****************************************************************************
 *  exported functions
 ****************************************************************************/

/****************************************************************************/
/** create a Pic with 8bit pels from already existing data (or as a window
 *  inside another Pic)
 *
 *  \param  pThat
 *  \param  S stride
 *  \param  dx width
 *  \param  dy height
 *  \param  dat picture data
 */
void Pic_Create(tPic *pThat, int S, int dx, int dy, void *dat)
{
    pThat->Dx=dx;
    pThat->Dy=dy;
    pThat->S=S;
    pThat->Pel=dat;
}


/****************************************************************************/
/** free a Pic
 *
 *  \param pThat
 */
void Pic_Free(tPic *pThat)
{
    free(pThat->Pel);
}


/****************************************************************************/
/** free a Pic that has been allocated with MallocWithPad()
 *
 *  \param  pThat
 *  \param  pad the padding value used in MallocWithPad()
 */
void Pic_FreeWithPad(tPic *pThat, int pad)
{
    int padleft=PAD(pad,sizeof(int));

    free(pThat->Pel-pad*pThat->S-padleft);
}


/****************************************************************************/
/** malloc a Pic with 8bit pels
 *
 *  \param  pThat the pic
 *  \param  dx    width
 *  \param  dy    height
 *  \param  Aln   alignment
 */
void Pic8_MallocAln(tPic *pThat, int dx, int dy, int Aln)
{
    MUST_PLAUS(dx,dy);

    pThat->Dx=dx;
    pThat->Dy=dy;
    pThat->S=PAD(dx,Aln);
    pThat->Pel=calloc(pThat->S*dy,1); MUST(pThat->Pel);
    MUST_Eq((pThat->Pel-(u8*)0)&0x3,0);
}


/****************************************************************************/
/** malloc a Pic with 8bit pels
 *
 *  \param  pThat
 *  \param  dx width
 *  \param  dy height
 */
void Pic8_Malloc(tPic *pThat, int dx, int dy)
{
    Pic8_MallocAln(pThat,dx,dy,sizeof(int));
}


/****************************************************************************/
/** malloc a Pic with 8bit pels and enough memory, to access up to "pad" pels
 *  _outside_ the image
 *
 *  \param  pThat
 *  \param  dx width
 *  \param  dy height
 *  \param  pad marginsize available for padding
 */
void Pic8_MallocWithPad(tPic *pThat, int dx, int dy, int pad)
{
    int padleft=PAD(pad,sizeof(int));

    MUST_PLAUS(dx,dy);

    pThat->Dx=dx;
    pThat->Dy=dy;
    pThat->S=PAD((dx+padleft+pad),sizeof(int));
    pThat->Pel=((u8*)calloc(pThat->S*(dy+2*pad),1))+pad*pThat->S+padleft;
    MUST(pThat->Pel);
}


/****************************************************************************/
/** malloc a Pic with 16bit pels
 *
 *  \param  pThat
 *  \param  dx width
 *  \param  dy height
 */
void Pic16_Malloc(tPic *pThat, int dx, int dy)
{
    Pic8_Malloc(pThat,2*dx,dy);
    pThat->Dx=dx;
}


/****************************************************************************/
/** malloc a Pic with 24bit pels
 *
 *  \param  pThat
 *  \param  dx width
 *  \param  dy height
 */
void Pic24_Malloc(tPic *pThat, int dx, int dy)
{
    Pic8_Malloc(pThat,3*dx,dy);
    pThat->Dx=dx;
}


/****************************************************************************/
/** malloc a Pic with 32bit pels
 *
 *  \param  pThat
 *  \param  dx width
 *  \param  dy height
 */
void Pic32_Malloc(tPic *pThat, int dx, int dy)
{
    Pic8_Malloc(pThat,4*dx,dy);
    pThat->Dx=dx;
}


/****************************************************************************/
/** malloc a Yuv as 420 (i.e Y dx*dy UV dx/2*dy/2)
 *
 *  \param  pThat
 *  \param  dx width
 *  \param  dy height
 */
void Yuv420_Malloc(tYuv *pThat, int dx, int dy)
{
    MUST(dx%2==0); MUST(!dy%2==0);

    pThat->Dx=dx;
    pThat->Dy=dy;

    Pic8_Malloc(&pThat->C[0],dx,dy);
    Pic8_Malloc(&pThat->C[1],dx/2,dy/2);
    Pic8_Malloc(&pThat->C[2],dx/2,dy/2);
}


/****************************************************************************/
/** malloc a Yc as 420 (i.e Y dx*dy UV dx/2*dy/2)
 *
 *  \param  pThat
 *  \param  dx width
 *  \param  dy height
 */
void Yc420_Malloc(tYc *pThat, int dx, int dy)
{
    MUST(dx%2==0); MUST(!dy%2==0);

    pThat->Dx=dx;
    pThat->Dy=dy;

    Pic8_Malloc(&pThat->Y,dx,dy);
    Pic8_Malloc(&pThat->C,dx,dy/2);
}


/****************************************************************************/
/** import a Yc image from a Yuv image
 *
 *  \param  pThat
 *  \param  pSrc
 */
void Yc_Import(tYc *pThat, tYuv *pSrc)
{
    u8    *pd,*pu,*pv;
    int   x,y;

    Pic8_Copy(&pThat->Y,&pSrc->C[0]);

    pd=pThat->C.Pel;
    pu=pSrc->C[1].Pel;
    pv=pSrc->C[2].Pel;
    for(y=0;y<pThat->C.Dy;y++){
	for(x=0;x<pThat->Dx/2;x++){
	    pd[2*x]=pu[x];
	    pd[2*x+1]=pv[x];
	}
	pd+=pThat->C.S;
	pu+=pSrc->C[1].S;
	pv+=pSrc->C[2].S;
    }
}


/****************************************************************************/
/** copy that pic from a source pic
 *
 *  \param  pThat the pic
 */
void Pic8_Clear(tPic *pThat)
{
    u8    *pd;
    int   y;

    pd=pThat->Pel;
    for(y=0;y<pThat->Dy;y++){
	memset(pd,0,pThat->Dx);
	pd+=pThat->S;
    }
}


/****************************************************************************/
/** copy that pic from a source pic
 *
 *  \param  pThat
 *  \param  pSrc
 */
void Pic8_Copy(tPic *pThat, tPic *pSrc)
{
    u8    *pd,*ps;
    int   y;

    MUST_Ge(pThat->Dx,pSrc->Dx);
    MUST_Ge(pThat->Dy,pSrc->Dy);

    pd=pThat->Pel;
    ps=pSrc->Pel;
    for(y=0;y<pSrc->Dy;y++){
	memcpy(pd,ps,pSrc->Dx);
	pd+=pThat->S;
	ps+=pSrc->S;
    }
}


/****************************************************************************/
/** copy that pic from a source pic
 *
 *  \param  pThat
 *  \param  pSrc
 */
void Pic16_Copy(tPic *pThat, tPic *pSrc)
{
    u8    *pd,*ps;
    int   y;

    MUST_Ge(pThat->Dx,pSrc->Dx);
    MUST_Ge(pThat->Dy,pSrc->Dy);

    pd=pThat->Pel;
    ps=pSrc->Pel;
    for(y=0;y<pSrc->Dy;y++){
	memcpy(pd,ps,pSrc->Dx*2);
	pd+=pThat->S;
	ps+=pSrc->S;
    }
}


/****************************************************************************/
/** copy that pic from a source pic
 *
 *  \param  pThat
 *  \param  pSrc
 */
void Pic24_Copy(tPic *pThat, tPic *pSrc)
{
    u8    *pd,*ps;
    int   y;

    MUST_Ge(pThat->Dx,pSrc->Dx);
    MUST_Ge(pThat->Dy,pSrc->Dy);

    pd=pThat->Pel;
    ps=pSrc->Pel;
    for(y=0;y<pSrc->Dy;y++){
	memcpy(pd,ps,pSrc->Dx*3);
	pd+=pThat->S;
	ps+=pSrc->S;
    }
}

/****************************************************************************/
/** copy that pic from a source pic
 *
 *  \param  pThat
 *  \param  pSrc
 */
void Pic32_Copy(tPic *pThat, tPic *pSrc)
{
    u8    *pd,*ps;
    int   y;

    MUST_Ge(pThat->Dx,pSrc->Dx);
    MUST_Ge(pThat->Dy,pSrc->Dy);

    pd=pThat->Pel;
    ps=pSrc->Pel;
    for(y=0;y<pSrc->Dy;y++){
	memcpy(pd,ps,pSrc->Dx*4);
	pd+=pThat->S;
	ps+=pSrc->S;
    }
}


/****************************************************************************/
/** copy that pic from a source pic
 *
 *  \param  pThat the pic
 *  \param  pSrc  a src pic
 *  \param  Shr   right shift for the src data
 */
void Pic16_CopyU32Shr(tPic *pThat, tPic *pSrc, int Shr)
{
    u8    *pd,*ps;
    int   y,x;
    u32	pel;

    MUST_Ge(pThat->Dx,pSrc->Dx);
    MUST_Ge(pThat->Dy,pSrc->Dy);

    pd=pThat->Pel;
    ps=pSrc->Pel;
    for(y=0;y<pSrc->Dy;y++){
	for(x=0;x<pSrc->Dx;x++){
	    pel=CERU32(ps+4*x);
	    pel>>=Shr;
	    CEW16(pd+2*x,pel);
	}
	pd+=pThat->S;
	ps+=pSrc->S;
    }
}


/****************************************************************************/
/** copy that pic from a source pic
 *
 *  \param  pThat the pic
 *  \param  pSrc  a src pic
 *  \param  Shr   right shift for the src data
 */
void Pic8_CopyU16Shr(tPic *pThat, tPic *pSrc, int Shr)
{
    u8    *pd,*ps;
    int   y,x;
    u32	pel;

    MUST_Ge(pThat->Dx,pSrc->Dx);
    MUST_Ge(pThat->Dy,pSrc->Dy);

    pd=pThat->Pel;
    ps=pSrc->Pel;
    for(y=0;y<pSrc->Dy;y++){
	for(x=0;x<pSrc->Dx;x++){
	    pel=CERU16(ps+2*x);
	    pel>>=Shr;
	    pd[x]=pel;
	}
	pd+=pThat->S;
	ps+=pSrc->S;
    }
}


/****************************************************************************/
/** copy that pic from a source pic
 *
 *  \param  pThat the pic
 *  \param  pSrc  a src pic
 *  \param  Shl   left shift of src data
 */
void Pic32_CopyU16Shl(tPic *pThat, tPic *pSrc, int Shl)
{
    u8    *pd,*ps;
    int   y,x;
    u32	pel;

    MUST_Ge(pThat->Dx,pSrc->Dx);
    MUST_Ge(pThat->Dy,pSrc->Dy);

    pd=pThat->Pel;
    ps=pSrc->Pel;
    for(y=0;y<pSrc->Dy;y++){
	for(x=0;x<pSrc->Dx;x++){
	    pel=CERU16(ps+2*x);
	    pel<<=Shl;
	    CEW32(pd+4*x,pel);
	}
	pd+=pThat->S;
	ps+=pSrc->S;
    }
}


/****************************************************************************/
/** Flip pic horizontally
 *
 *  \param  pPic  pointer to pic
 *  \return 0 on success, 1 on failure
 */
void Pic_HorFlip(tPic *pPic)
{
    int i;
    u8 *line;
    int len;
    u8 *bot;
    u8 *top;

    MUST(pPic);

    len = pPic->S;
    line = calloc(len,1);
    top = pPic->Pel;
    bot = pPic->Pel + (pPic->S*(pPic->Dy-1));
    for (i = 0; i<pPic->Dy>>1; i++) {
	memcpy(line,top,len);
	memcpy(top,bot,len);
	memcpy(bot,line,len);
	top += len;
	bot -= len;
    }
    free(line);
}


/****************************************************************************/
/** shift the bits of all U8 pels to the left (inplace) and clip
 *
 *  \param  pThat
 *  \param  Shift
 */
void Pic8_ShiftLeft(tPic *pThat, int Shift)
{
    int x,y,v;

    for(y=0;y<pThat->Dy;y++){
	for(x=0;x<pThat->Dx;x++){
	    v=*pPEL8(pThat,x,y);
	    v<<=Shift;
	    v=CLIP(v,0,MAX_U8);
	    *pPEL8(pThat,x,y)=v;
	}
    }
}


/****************************************************************************/
/** shift the bits of all U16 pels to the left (inplace) and clip
 *
 *  \param  pThat
 *  \param  Shift
 */
void Pic16_ShiftLeft(tPic *pThat, int Shift)
{
    int x,y,v;

    for(y=0;y<pThat->Dy;y++){
	for(x=0;x<pThat->Dx;x++){
	    v=CERU16(pPEL16(pThat,x,y));
	    v<<=Shift;
	    v=CLIP(v,0,MAX_U16);
	    CEW16(pPEL16(pThat,x,y),v);
	}
    }
}


/****************************************************************************/
/** set all pixels of that pic to val
 *
 *  \param  pThat
 *  \param  val pixel value
 */
void Pic8_Set(tPic *pThat, int val)
{
    u8    *pd;
    int   y;

    pd=pThat->Pel;
    for(y=0;y<pThat->Dy;y++){
	memset(pd,val,pThat->Dx);
	pd+=pThat->S;
    }
}


/****************************************************************************/
/** set all pixels of that pic to val
 *
 *  \param  pThat
 *  \param  val pixel value
 */
void Pic16_Set(tPic *pThat, int val)
{
    int   x,y;

    for(y=0;y<pThat->Dy;y++)
	for(x=0;x<pThat->Dx;x++)
	    CEW16(pPEL16(pThat,x,y),val);
}


/****************************************************************************/
/** set all pixels of that pic to val
 *
 *  \param  pThat
 *  \param  val pixel value
 */
void Pic32_Set(tPic *pThat, u32 val)
{
    int   x,y;

    for(y=0;y<pThat->Dy;y++)
	for(x=0;x<pThat->Dx;x++)
	    CEW32(pPEL32(pThat,x,y),val);
}


/****************************************************************************/
/** pack that pic as ARGB 4:4:4:4 from a source pic in ARGB 8:8:8:8
 *
 *  \param  pThat
 *  \param  pSrc
 */
void Pic16_Pack4444(tPic *pThat, tPic *pSrc)
{
    u8    *pd,*ps;
    int   y,x;

    MUST_Eq(pThat->Dx,pSrc->Dx);
    MUST_Eq(pThat->Dy,pSrc->Dy);

    pd=pThat->Pel;
    ps=pSrc->Pel;
    for(y=0;y<pThat->Dy;y++){
	for(x=0;x<pThat->Dx;x++){
	    pd[2*x+0]
		=(ps[4*x+0]>>4)<<4
		|(ps[4*x+1]>>4);
	    pd[2*x+1]
		=(ps[4*x+2]>>4)<<4
		|(ps[4*x+3]>>4);
	}
	pd+=pThat->S;
	ps+=pSrc->S;
    }
}


/****************************************************************************/
/** pack that pic as reg RGB 5:6:5 from a source pic in mem ARGB 8:8:8:8
 *
 *  \param  pThat
 *  \param  pSrc
 */
void Pic16_Pack565_XRGB(tPic *pThat, tPic *pSrc)
{
    u8    *pd,*ps;
    int   y,x,dx,dy,r,g,b;

    dx=MIN(pThat->Dx,pSrc->Dx);  dy=MIN(pThat->Dy,pSrc->Dy);
    pd=pThat->Pel;  ps=pSrc->Pel;

    for(y=0;y<dy;y++){
	for(x=0;x<dx;x++){
	    r=ps[4*x+1]; g=ps[4*x+2]; b=ps[4*x+3];
	    r>>=3; g>>=2; b>>=3;
	    CEW16(pd+2*x,r<<11|g<<5|b);
	}
	pd+=pThat->S;
	ps+=pSrc->S;
    }
}


/****************************************************************************/
/** pack that pic as reg RGB 5:6:5 from a source pic in mem RGBX 8:8:8:8
 *
 *  \param  pThat
 *  \param  pSrc
 */
void Pic16_Pack565_RGBX(tPic *pThat, tPic *pSrc)
{
    u8    *pd,*ps;
    int   y,x,dx,dy,r,g,b;

    dx=MIN(pThat->Dx,pSrc->Dx);  dy=MIN(pThat->Dy,pSrc->Dy);
    pd=pThat->Pel;  ps=pSrc->Pel;

    for(y=0;y<dy;y++){
	for(x=0;x<dx;x++){
	    r=ps[4*x+0]; g=ps[4*x+1]; b=ps[4*x+2];
	    r>>=3; g>>=2; b>>=3;
	    CEW16(pd+2*x,r<<11|g<<5|b);
	}
	pd+=pThat->S;
	ps+=pSrc->S;
    }
}


/****************************************************************************/
/** pack that pic as RGB 5:6:5 from a source pic in ARGB 8:8:8:8
 *
 *  \param  pThat
 *  \param  pSrc
 */
void Pic32_UnPack565(tPic *pThat, tPic *pSrc)
{
    u8    *pd,*ps;
    int   y,x,dx,dy,v;
    u32	argb;

    dx=MIN(pThat->Dx,pSrc->Dx);  dy=MIN(pThat->Dy,pSrc->Dy);
    pd=pThat->Pel;  ps=pSrc->Pel;

    for(y=0;y<dy;y++){
	for(x=0;x<dx;x++){
	    v=CERU16(ps+2*x);
	    argb=BITS(v,15,11)<<(16+3)|BITS(v,10,5)<<(8+2)|BITS(v,4,0)<<3;
	    argb|=BITS(v,15,13)<<16|BITS(v,10,9)<<8|BITS(v,4,2);
	    BEW32(pd+4*x,argb);
	}
	pd+=pThat->S;
	ps+=pSrc->S;
    }
}


/****************************************************************************/
/** 
 *
 *  \param  pThat
 *  \param  pSrc
 */
void Pic32_RGBXfromU8(tPic *pThat, tPic *pSrc)
{
    u8    	*pd,*ps;
    int   	y,x,dx,dy,v;

    dx=MIN(pThat->Dx,pSrc->Dx);  dy=MIN(pThat->Dy,pSrc->Dy);

    pd=pThat->Pel;  ps=pSrc->Pel;

    for(y=0;y<dy;y++)
    {
	for(x=0;x<dx;x++)
	{
	    v=ps[x];
	    pd[4*x+0]=v;
	    pd[4*x+1]=v;
	    pd[4*x+2]=v;
	    pd[4*x+3]=v;
	}
	pd+=pThat->S;
	ps+=pSrc->S;
    }
}


/****************************************************************************/
/** 
 *
 *  \param  pThat
 *  \param  pSrc
 */
void Pic32_RGBXfromRGB(tPic *pThat, tPic *pSrc)
{
    u8    	*pd,*ps;
    int   	y,x,dx,dy;

    dx=MIN(pThat->Dx,pSrc->Dx);  dy=MIN(pThat->Dy,pSrc->Dy);

    pd=pThat->Pel;  ps=pSrc->Pel;

    for(y=0;y<dy;y++)
    {
	for(x=0;x<dx;x++)
	{
	    pd[4*x+0]=ps[3*x+0];
	    pd[4*x+1]=ps[3*x+1];
	    pd[4*x+2]=ps[3*x+2];
	    pd[4*x+3]=0;
	}
	pd+=pThat->S;
	ps+=pSrc->S;
    }
}


/****************************************************************************/
/** set alpha to transparent if a certain color is matched, opaque otherwise
 *
 *  \param  pThat
 *  \param  r,g,b color to become transparent
 */
void Pic32_GenAlpha(tPic *pThat, int r, int g, int b)
{
    u8    *pd;
    int   y,x;

    pd=pThat->Pel;
    for(y=0;y<pThat->Dy;y++){
	for(x=0;x<pThat->Dx;x++){
	    pd[4*x+0]=(pd[4*x+1]==r&&pd[4*x+2]==g&&pd[4*x+3]==b)?0:0xff;
	}
	pd+=pThat->S;
    }
}


/****************************************************************************/
/** 
 *
 *  \param  pThat
 *  \param  pSrc
 */
void Pic32_RGBXfromBGR(tPic *pThat, tPic *pSrc)
{
    u8    	*pd,*ps;
    int   	y,x,dx,dy;

    dx=MIN(pThat->Dx,pSrc->Dx);  dy=MIN(pThat->Dy,pSrc->Dy);

    pd=pThat->Pel;  ps=pSrc->Pel;

    for(y=0;y<dy;y++)
    {
	for(x=0;x<dx;x++)
	{
	    pd[4*x+0]=ps[3*x+2];
	    pd[4*x+1]=ps[3*x+1];
	    pd[4*x+2]=ps[3*x+0];
	    pd[4*x+3]=0;
	}
	pd+=pThat->S;
	ps+=pSrc->S;
    }
}


/****************************************************************************/
/** pack a 3 byte RGB image from a 4 byte RGBX
 *
 *  \param  pThat
 *  \param  pSrc
 */
void Pic24_RGBfromRGBX(tPic *pThat, tPic *pSrc)
{
    u8    	*pd,*ps;
    int   	y,x,dx,dy;

    dx=MIN(pThat->Dx,pSrc->Dx);  dy=MIN(pThat->Dy,pSrc->Dy);

    pd=pThat->Pel;  ps=pSrc->Pel;

    for(y=0;y<dy;y++)
    {
	for(x=0;x<dx;x++)
	{
	    pd[3*x+0]=ps[4*x+0];
	    pd[3*x+1]=ps[4*x+1];
	    pd[3*x+2]=ps[4*x+2];
	}
	pd+=pThat->S;
	ps+=pSrc->S;
    }
}


/****************************************************************************/
/** pack a 3 byte BGR image from a 4 byte RGBX
 *
 *  \param  pThat
 *  \param  pSrc
 */
void Pic24_BGRfromRGBX(tPic *pThat, tPic *pSrc)
{
    u8    	*pd,*ps;
    int   	y,x,dx,dy;

    dx=MIN(pThat->Dx,pSrc->Dx);  dy=MIN(pThat->Dy,pSrc->Dy);

    pd=pThat->Pel;  ps=pSrc->Pel;

    for(y=0;y<dy;y++)
    {
	for(x=0;x<dx;x++)
	{
	    pd[3*x+0]=ps[4*x+2];
	    pd[3*x+1]=ps[4*x+1];
	    pd[3*x+2]=ps[4*x+0];
	}
	pd+=pThat->S;
	ps+=pSrc->S;
    }
}


/****************************************************************************/
/** load planar YUV file (3 consecutive images) from file
 *
 *  \param  pThat
 *  \param  Name filename
 *  \return success (always TRUE at the moment)
 */
bool Yuv_Load(tYuv *pThat, const char *Name)
{
    FILE  *file;
    u8    *pp;
    int   c,y,r;

    if(strcmp(Name,"-")==0)
	file=stdin;
    else file=fopen(Name,"r");     MUST(file);

    for(c=0;c<LEN(pThat->C);c++){
	pp=pThat->C[c].Pel;
	for(y=0;y<pThat->C[c].Dy;y++){
	    r=fread(pp,pThat->C[c].Dx,1,file);
	    if(r!=1) ERROR("cannot read");
	    pp+=pThat->C[c].S;
	}
    }

    fclose(file);

    return TRUE;

}


/****************************************************************************/
/** load planar YUV 420 file (3 consecutive images) from file to packed image
 *  in memory
 *
 *  \param  pThat
 *  \param  Name filename
 *  \return success (always TRUE at the moment)
 */
bool Yc_Load(tYc *pThat, const char *Name)
{
    FILE  *file;
    u8    *pp;
    int   x,y,r;
    u8	buff[1024];

    MUST_Le(pThat->Dx/2,(int)sizeof(buff));

    if(strcmp(Name,"-")==0)
	file=stdin;
    else if(!(file=fopen(Name,"r"))){
	pThat->Dx=0;
	pThat->Dy=0;
	pThat->Y.Dx=0;
	pThat->Y.Dy=0;
	pThat->Y.S=0;
	pThat->Y.Pel=0;
	pThat->C.Dx=0;
	pThat->C.Dy=0;
	pThat->C.S=0;
	pThat->C.Pel=0;
	return FALSE;
    }

    pp=pThat->Y.Pel;
    for(y=0;y<pThat->Dy;y++){
	r=fread(pp,pThat->Dx,1,file);
	if(r!=1) ERROR("cannot read");
	pp+=pThat->Y.S;
    }

    pp=pThat->C.Pel;
    for(y=0;y<pThat->Dy/2;y++){
	r=fread(buff,pThat->Dx/2,1,file);
	if(r!=1) ERROR("cannot read");
	for(x=0;x<pThat->Dx/2;x++)
	    pp[2*x+0]=buff[x];
	pp+=pThat->Y.S;
    }

    pp=pThat->C.Pel;
    for(y=0;y<pThat->Dy/2;y++){
	r=fread(buff,pThat->Dx/2,1,file);
	if(r!=1) ERROR("cannot read");
	for(x=0;x<pThat->Dx/2;x++)
	    pp[2*x+1]=buff[x];
	pp+=pThat->Y.S;
    }


    fclose(file);

    return TRUE;
}


/****************************************************************************/
/** free a packed yuv Pi
 *
 *  \param  pThat
 */
void Yc_Free(tYc *pThat)
{
    Pic_Free(&pThat->Y);
    Pic_Free(&pThat->C);
}


/****************************************************************************/
/** calculate a SAD image between to 32bit RGB images
 *
 *  \param  pThat
 *  \param  pA,pB the 2 images to compare
 *  \param  factor is multiplied with the output pixels, to enhance small
 *          differences
 *  \return sad over all pels
 */
int Pic8_Sad32RGB(tPic *pThat, const tPic *pA, const tPic *pB, int factor)
{
    int               x,y,r,g,b,s,ss;

    MUST(pThat->Dx<=pA->Dx);
    MUST(pThat->Dy<=pA->Dy);
    MUST(pThat->Dx<=pB->Dx);
    MUST(pThat->Dy<=pB->Dy);

    ss=0;

    for(y=0;y<pThat->Dy;y++){
	for(x=0;x<pThat->Dx;x++){
	    r=pA->Pel[pA->S*y+4*x+1]-pB->Pel[pB->S*y+4*x+1];
	    g=pA->Pel[pA->S*y+4*x+2]-pB->Pel[pB->S*y+4*x+2];
	    b=pA->Pel[pA->S*y+4*x+3]-pB->Pel[pB->S*y+4*x+3];
	    s=ABS(r)+ABS(g)+ABS(b);
	    pThat->Pel[pThat->S*y+x]=MIN(255,s*factor);
	    ss+=s;
	}
    }

    return ss;
}


/****************************************************************************/
/** calculate a SAD image between to 32bit grey value images
 *
 *  \param  pThat
 *  \param  pA,pB the 2 images to compare
 *  \param  factor is multiplied with the output pixels, to enhance small
 *          differences
 *  \return sad over all pels
 */
int Pic8_Sad32(tPic *pThat, const tPic *pA, const tPic *pB, int factor)
{
    int		x,y;
    u64		s,ss;

    MUST_Le(pThat->Dx,pA->Dx);
    MUST_Le(pThat->Dy,pA->Dy);
    MUST_Le(pThat->Dx,pB->Dx);
    MUST_Le(pThat->Dy,pB->Dy);

    ss=0;

    for(y=0;y<pThat->Dy;y++){
	for(x=0;x<pThat->Dx;x++){
	    s=ABS(((s64)CERU32(pA->Pel+pA->S*y+4*x))-((s64)CERU32(pB->Pel+pB->S*y+4*x)));
	    pThat->Pel[pThat->S*y+x]=(u8)MIN(255,s*factor);
	    ss+=s;
	}
    }

    return (int)ss;
}


/****************************************************************************/
/** calculate a SAD image between to 8bit images
 *
 *  \param  pThat
 *  \param  pA,pB the 2 images to compare
 *  \param  factor is multiplied with the output pixels, to enhance small
 *          differences
 *  \return sad over all pels
 */
int Pic8_Sad8(tPic *pThat, const tPic *pA, const tPic *pB, int factor)
{
    int               x,y,s,ss;

    MUST_Le(pThat->Dx,pA->Dx);
    MUST_Le(pThat->Dy,pA->Dy);
    MUST_Le(pThat->Dx,pB->Dx);
    MUST_Le(pThat->Dy,pB->Dy);

    ss=0;

    for(y=0;y<pThat->Dy;y++){
	for(x=0;x<pThat->Dx;x++){
	    s=ABS(pA->Pel[pA->S*y+x]-pB->Pel[pB->S*y+x]);
	    pThat->Pel[pThat->S*y+x]=MIN(255,s*factor);
	    ss+=s;
	}
    }

    return ss;
}


/****************************************************************************/
/** calculate a SAD image between to 8bit images
 *
 *  \param  pThat
 *  \param  pA,pB the 2 images to compare
 *  \param  factor is multiplied with the output pixels, to enhance small
 *          differences
 *  \return sad over all pels
 */
int Pic8_Sad16(tPic *pThat, const tPic *pA, const tPic *pB, int factor)
{
    int               x,y,s,ss;

    MUST_Le(pThat->Dx,pA->Dx);
    MUST_Le(pThat->Dy,pA->Dy);
    MUST_Le(pThat->Dx,pB->Dx);
    MUST_Le(pThat->Dy,pB->Dy);

    ss=0;

    for(y=0;y<pThat->Dy;y++){
	for(x=0;x<pThat->Dx;x++){
	    s=ABS(CERU16(pA->Pel+pA->S*y+2*x)-CERU16(pB->Pel+pB->S*y+2*x));
	    pThat->Pel[pThat->S*y+x]=MIN(255,s*factor);
	    ss+=s;
	}
    }

    return ss;
}


/****************************************************************************/
/** copy border pixels to pad area
 *
 *  \param  pThat
 *  \param  pad size of pad area
 */
void Pic8_Pad(tPic *pThat, int pad)
{
    int		x,y;

    /* left */
    for(y=0;y<pThat->Dy;y++)
	for(x=-pad;x<0;x++)
	    *pPEL8(pThat,x,y)=*pPEL8(pThat,0,y);

    /* right */
    for(y=0;y<pThat->Dy;y++)
	for(x=pThat->Dx;x<pThat->Dx+pad;x++)
	    *pPEL8(pThat,x,y)=*pPEL8(pThat,pThat->Dx-1,y);

    /* top */
    for(x=-pad;x<pThat->Dx+pad;x++)
	for(y=-pad;y<0;y++)
	    *pPEL8(pThat,x,y)=*pPEL8(pThat,x,0);

    /* bottom */
    for(x=-pad;x<pThat->Dx+pad;x++)
	for(y=pThat->Dy;y<pThat->Dy+pad;y++)
	    *pPEL8(pThat,x,y)=*pPEL8(pThat,x,pThat->Dy-1);
}


/*****************************************************************************
 *  exported functions: save
 ****************************************************************************/

/****************************************************************************/
/** save 8bit grey image as raw pgm
 *
 *  \param  pThat
 *  \param  Name filename
 *  \return success (always TRUE at the moment)
 */
bool Pic8_Save(const tPic *pThat, const char *Name)
{
    FILE  *file;
    u8    *pp;
    int   y;

    if(!(file=fopen(Name,"w"))) ERROR("cannot create: %s",Name);
    fprintf(file,"P5\n# created by nuts\n%d %d\n255\n",pThat->Dx,pThat->Dy);

    pp=pThat->Pel;
    for(y=0;y<pThat->Dy;y++){
	if(fwrite(pp,pThat->Dx,1,file)!=1) ERROR("writing: %s,Name");
	pp+=pThat->S;
    }

    fclose(file);

    return TRUE;
}


/****************************************************************************/
/** save 8bit grey image as ascii pgm
 *
 *  \param  pThat
 *  \param  Name filename
 *  \return success (always TRUE at the moment)
 */
bool Pic8_SaveA(const tPic *pThat, const char *Name)
{
    FILE  *file;
    u8    *pp;
    int   y,x;

    if(!(file=fopen(Name,"w"))) ERROR("cannot create: %s",Name);
    fprintf(file,"P2\n# created by nuts\n%d %d\n255\n",pThat->Dx,pThat->Dy);

    pp=pThat->Pel;
    for(y=0;y<pThat->Dy;y++){
	for(x=0;x<pThat->Dx;x++){
	    fprintf(file,"%3d ",pp[x]);
	}
	fprintf(file,"\n");
	pp+=pThat->S;
    }

    fclose(file);

    return TRUE;
}


/****************************************************************************/
/** save 16bit grey image as raw pgm
 *
 *  \param  pThat
 *  \param  Name filename
 *  \return success (always TRUE at the moment)
 */
bool Pic16_Save(const tPic *pThat, const char *Name)
{
    FILE  *file;
    u8    *pp;
    int   y;

    if(!(file=fopen(Name,"w"))) ERROR("cannot create: %s",Name);
    fprintf(file,"P5\n# created by nuts\n%d %d\n65535\n",pThat->Dx,pThat->Dy);

    pp=pThat->Pel;
    for(y=0;y<pThat->Dy;y++){
	if(fwrite(pp,pThat->Dx*2,1,file)!=1) ERROR("writing: %s,Name");
	pp+=pThat->S;
    }

    fclose(file);

    return TRUE;
}


/****************************************************************************/
/** save 32bit grey image as raw pgm
 *
 *  \param  pThat
 *  \param  Name filename
 *  \return success (always TRUE at the moment)
 */
bool Pic32_Save(const tPic *pThat, const char *Name)
{
    FILE  *file;
    u8    *pp;
    int   y;

    if(!(file=fopen(Name,"w"))) ERROR("cannot create: %s",Name);
    fprintf(file,"P5\n# created by nuts\n%d %d\n4294967295\n",pThat->Dx,pThat->Dy);

    pp=pThat->Pel;
    for(y=0;y<pThat->Dy;y++){
	if(fwrite(pp,pThat->Dx*4,1,file)!=1) ERROR("writing: %s,Name");
	pp+=pThat->S;
    }

    fclose(file);

    return TRUE;
}


/****************************************************************************/
/** save 16bit grey image as ascii pgm. should normally not be used, as raw
 *  images can always be converted for display with "pnmnoraw".
 *
 *  \param  pThat
 *  \param  Name filename
 *  \return success (always TRUE at the moment)
 */
bool Pic16_SaveA(const tPic *pThat, const char *Name)
{
    FILE  *file;
    u8    *pp;
    int   x,y;

    if(!(file=fopen(Name,"w"))) ERROR("cannot create: %s",Name);
    fprintf(file,"P2\n# created by nuts\n%d %d\n65535\n",pThat->Dx,pThat->Dy);

    pp=pThat->Pel;
    for(y=0;y<pThat->Dy;y++){
	for(x=0;x<pThat->Dx;x++){
	    fprintf(file,"%d ",CERU16(pp+2*x));
	}
	fprintf(file,"\n");
	pp+=pThat->S;
    }

    fclose(file);

    return TRUE;
}


/****************************************************************************/
/** save 16bit grey image as ascii pgm. should normally not be used, as raw
 *  images can always be converted for display with "pnmnoraw".
 *
 *  \param  pThat
 *  \param  Name filename
 *  \return success (always TRUE at the moment)
 */
bool Pic32_SaveA(const tPic *pThat, const char *Name)
{
    FILE  *file;
    u8    *pp;
    int   x,y;

    if(!(file=fopen(Name,"w"))) ERROR("cannot create: %s",Name);
    fprintf(file,"P2\n# created by nuts\n%d %d\n4294967295\n",pThat->Dx,pThat->Dy);

    pp=pThat->Pel;
    for(y=0;y<pThat->Dy;y++){
	for(x=0;x<pThat->Dx;x++){
	    fprintf(file,"%d ",CERU32(pp+4*x));
	}
	fprintf(file,"\n");
	pp+=pThat->S;
    }

    fclose(file);

    return TRUE;
}

/****************************************************************************/
/** save pic to ppm file interpreting the bytes in memory as XRGB
 *
 *  \param  pThat
 *  \param  Name filename
 *  \return success (always TRUE at the moment)
 */
bool Pic32_SaveXRGB(const tPic *pThat, const char *Name)
{
    FILE  *file;
    u8    *pp;
    int   x,y,r;

    if(!(file=fopen(Name,"w"))) ERROR("cannot create: %s",Name);
    fprintf(file,"P6\n# created by nuts\n%d %d\n255\n",pThat->Dx,pThat->Dy);

    pp=pThat->Pel;
    for(y=0;y<pThat->Dy;y++){
	for(x=0;x<pThat->Dx;x++){
	    r=fwrite(&pp[4*x+1],3,1,file);
	    if(r!=1) ERROR("cannot write");
	}
	pp+=pThat->S;
    }

    fclose(file);

    return TRUE;
}

/****************************************************************************/
/** save pic to ppm file interpreting the bytes in memory as RGBX
 *
 *  \param  pThat
 *  \param  Name filename
 *  \return success (always TRUE at the moment)
 */
bool Pic32_SaveRGBX(const tPic *pThat, const char *Name)
{
    FILE  *file;
    u8    *pp;
    int   x,y,r;

    if(!(file=fopen(Name,"w"))) ERROR("cannot create: %s",Name);
    fprintf(file,"P6\n# created by nuts\n%d %d\n255\n",pThat->Dx,pThat->Dy);

    pp=pThat->Pel;
    for(y=0;y<pThat->Dy;y++){
	for(x=0;x<pThat->Dx;x++){
	    r=fwrite(&pp[4*x],3,1,file);
	    if(r!=1) ERROR("cannot read");
	}
	pp+=pThat->S;
    }

    fclose(file);

    return TRUE;
}

/****************************************************************************/
/** save pic to ppm file interpreting the bytes in memory as BGRX
 *  which is XRGB on a LE system
 *
 *  \param  pThat
 *  \param  Name filename
 *  \return success (always TRUE at the moment)
 */
bool Pic32_SaveBGRX(const tPic *pThat, const char *Name)
{
    FILE  	*file;
    u8    	*pp;
    int   	x,y;
    u32		pel;

    if(!(file=fopen(Name,"w"))) ERROR("cannot create: %s",Name);
    fprintf(file,"P6\n# created by nuts\n%d %d\n255\n",pThat->Dx,pThat->Dy);

    pp=pThat->Pel;
    for(y=0;y<pThat->Dy;y++){
	for(x=0;x<pThat->Dx;x++){
	    pel=((u32*)pp)[x];
	    fputc(BITS(pel,23,16),file);
	    fputc(BITS(pel,15, 8),file);
	    fputc(BITS(pel, 7, 0),file);
	}
	pp+=pThat->S;
    }

    fclose(file);

    return TRUE;
}

/****************************************************************************/
/** save 3 colorbytes from XRGB 32 bit words to ppm file
 *
 *  \param  pThat
 *  \param  Name filename
 *  \return success (always TRUE at the moment)
 */
bool Pic32_SaveRgbA(const tPic *pThat, const char *Name)
{
    FILE  *file;
    u8    *pp;
    int   x,y;

    if(!(file=fopen(Name,"w"))) ERROR("cannot create: %s",Name);
    fprintf(file,"P3\n# created by nuts\n%d %d\n255\n",pThat->Dx,pThat->Dy);

    pp=pThat->Pel;
    for(y=0;y<pThat->Dy;y++){
	for(x=0;x<pThat->Dx;x++){
	    fprintf(file,"%3d %3d %3d  ",pp[4*x+1],pp[4*x+2],pp[4*x+3]);
	}
	fprintf(file,"\n");
	pp+=pThat->S;
    }

    fclose(file);

    return TRUE;
}


/*****************************************************************************
 *  exported functions: load
 ****************************************************************************/

/****************************************************************************/
/** load 8bit image from pgm file with alignment
 *
 *  \param  pThat
 *  \param  Name filename
 *  \param  Aln  alignment (4,8,16,32)
 *  \return TRUE if file exists
 */
bool Pic8_LoadAln(tPic *pThat, const char *Name, int Aln)
{
    FILE		*file;
    u8		*pp;
    int		x,y,dx,dy,v;
    bool		raw=TRUE;
    char		buffer[256];

    if(strcmp(Name,"-")==0)
	file=stdin;
    else if(!(file=fopen(Name,"r"))){
	pThat->Dx=0;
	pThat->Dy=0;
	pThat->S=0;
	pThat->Pel=NULL;
	return FALSE;
    }

    fgets(buffer,sizeof(buffer),file);
    if(strncmp(buffer,"P5",2)!=0){
	if(strncmp(buffer,"P2",2)!=0)
	    ERROR("pgm header (P2/P5) not found in %s",Name);
	raw=FALSE;
    }
    do fgets(buffer,sizeof(buffer),file); while(buffer[0]=='#');

    sscanf(buffer,"%d %d",&dx,&dy);
    Pic8_MallocAln(pThat,dx,dy,Aln);

    do fgets(buffer,sizeof(buffer),file); while(buffer[0]=='#');
    sscanf(buffer,"%d",&v);
    if(v>0xff)
	ERROR("%s has wrong range %d",Name,v);

    pp=pThat->Pel;

    if(raw){
	for(y=0;y<pThat->Dy;y++){
	    if(fread(pp,pThat->Dx,1,file)!=1)
		ERROR("read error in %s, line %d",Name,y);
	    pp+=pThat->S;
	}
    }
    else{
	for(y=0;y<dy;y++){
	    for(x=0;x<dx;x++){
		if(fscanf(file,"%d",&v)!=1)
		    ERROR("read error in %s at (%d,%d)",Name,x,y);
		pp[x]=v;
	    }
	    pp+=pThat->S;
	}
    }

    fclose(file);

    return TRUE;
}


/****************************************************************************/
/** load 8bit image from pgm file, old version
 *
 *  \param  pThat
 *  \param  Name filename
 *  \return TRUE if file exists
 */
bool Pic8_Load(tPic *pThat, const char *Name)
{
    return Pic8_LoadAln(pThat,Name,sizeof(int));
}


/****************************************************************************/
/** load 16bit image from pgm file with alignment
 *
 *  \param  pThat
 *  \param  Name filename
 *  \param  Aln  alignment (4,8,16,32)
 *  \return TRUE if file exists
 */
bool Pic16_LoadAln(tPic *pThat, const char *Name, int Aln)
{
    FILE		*file;
    u8		*pp;
    int		x,y,dx,dy,v,res;
    bool		raw=TRUE;
    char		buffer[256];

    if(strcmp(Name,"-")==0)
	file=stdin;
    else if(!(file=fopen(Name,"r"))){
	pThat->Dx=0;
	pThat->Dy=0;
	pThat->S=0;
	pThat->Pel=NULL;
	return FALSE;
    }

    fgets(buffer,sizeof(buffer),file);
    if(strncmp(buffer,"P5",2)!=0){
	if(strncmp(buffer,"P2",2)!=0)
	    ERROR("pgm header (P2/P5) not found in %s",Name);
	raw=FALSE;
    }
    do fgets(buffer,sizeof(buffer),file); while(buffer[0]=='#');

    res=sscanf(buffer,"%d %d",&dx,&dy); DLOGd(res);
    if(!res) ERROR("cannot read size of %s");
    else if(res==1){
	do fgets(buffer,sizeof(buffer),file); while(buffer[0]=='#');
	sscanf(buffer,"%d",&dy);
    }

    DLOGd(dx); DLOGd(dy);

    Pic8_MallocAln(pThat,dx*2,dy,Aln);  pThat->Dx=dx;

    do fgets(buffer,sizeof(buffer),file); while(buffer[0]=='#');
    sscanf(buffer,"%d",&v);

    if(v>0xffff)
	ERROR("%s has unsupported range %d",Name,v);

    pp=pThat->Pel;

    if(raw){
	for(y=0;y<pThat->Dy;y++){
	    if(fread(pp,pThat->Dx*2,1,file)!=1)
		ERROR("read error in %s line %d",Name,y);
	    pp+=pThat->S;
	}
    }
    else{
	for(y=0;y<dy;y++){
	    for(x=0;x<dx;x++){
		if(fscanf(file,"%d",&v)!=1)
		    ERROR("read error in %s at (%d,%d)",Name,x,y);
		CEW16(pp+2*x,v);
	    }
	    pp+=pThat->S;
	}
    }

    fclose(file);

    return TRUE;
}


/****************************************************************************/
/** load 16bit image from pgm file, old version
 *
 *  \param  pThat
 *  \param  Name filename
 *  \return TRUE if file exists
 */
bool Pic16_Load(tPic *pThat, const char *Name)
{
    return Pic16_LoadAln(pThat,Name,sizeof(int));
}


/****************************************************************************/
/** load 16bit image from pgm file, old version
 *
 *  \param  pThat
 *  \param  Name filename
 *  \return TRUE if file exists
 */
bool Pic16_UniLoad(tPic *pThat, const char *Name)
{
    switch(Pic_FileType(Name)){
    case PIC_G8:	return Pic16_LoadShl(pThat,Name,8);
    case PIC_G16:	return Pic16_Load(pThat,Name);
    default:
	pThat->Dx=0;
	pThat->Dy=0;
	pThat->S=0;
	pThat->Pel=NULL;
	return FALSE;
    }
}



/****************************************************************************/
/** load 32bit image from pgm file
 *
 *  \param  pThat
 *  \param  Name filename
 *  \return TRUE if file exists
 */
bool Pic32_Load(tPic *pThat, const char *Name)
{
    FILE		*file;
    u8		*pp;
    int		x,y,dx,dy,res;
    bool		raw=TRUE;
    char		buffer[256];
    u32		v;

    if(strcmp(Name,"-")==0)
	file=stdin;
    else if(!(file=fopen(Name,"r"))){
	pThat->Dx=0;
	pThat->Dy=0;
	pThat->S=0;
	pThat->Pel=NULL;
	return FALSE;
    }

    fgets(buffer,sizeof(buffer),file);
    if(strncmp(buffer,"P5",2)!=0){
	if(strncmp(buffer,"P2",2)!=0)
	    ERROR("pgm header (P2/P5) not found in %s",Name);
	raw=FALSE;
    }
    do fgets(buffer,sizeof(buffer),file); while(buffer[0]=='#');

    res=sscanf(buffer,"%d %d",&dx,&dy); DLOGd(res);
    if(!res) ERROR("cannot read size of %s");
    else if(res==1){
	do fgets(buffer,sizeof(buffer),file); while(buffer[0]=='#');
	sscanf(buffer,"%d",&dy);
    }

    DLOGd(dx); DLOGd(dy);

    Pic32_Malloc(pThat,dx,dy);

    do fgets(buffer,sizeof(buffer),file); while(buffer[0]=='#');
    sscanf(buffer,"%u",&v);
    if(v!=0xffffffff)
	ERROR("%s has wrong range %u",Name,v);

    pp=pThat->Pel;

    if(raw){
	for(y=0;y<pThat->Dy;y++){
	    if(fread(pp,pThat->Dx*4,1,file)!=1)
		ERROR("read error in %s line %d",Name,y);
	    pp+=pThat->S;
	}
    }
    else{
	for(y=0;y<dy;y++){
	    for(x=0;x<dx;x++){
		if(fscanf(file,"%u",&v)!=1)
		    ERROR("read error in %s at (%d,%d)",Name,x,y);
		CEW32(pp+4*x,v);
	    }
	    pp+=pThat->S;
	}
    }

    fclose(file);

    return TRUE;
}


/****************************************************************************/
/** load 16bit image from pgm file
 *
 *  \param  pThat the pic
 *  \param  Name  filename
 *  \param  Shift left shift of 8bit input data
 *  \return TRUE if file exists
 */
bool Pic16_LoadShl(tPic *pThat, const char *Name, int Shift)
{
    FILE		*file;
    u8		*pp;
    int		x,y,dx,dy,v;
    bool		raw=TRUE,r8=FALSE;
    char		buffer[256];

    if(strcmp(Name,"-")==0)
	file=stdin;
    else if(!(file=fopen(Name,"r"))){
	pThat->Dx=0;
	pThat->Dy=0;
	pThat->S=0;
	pThat->Pel=NULL;
	return FALSE;
    }

    fgets(buffer,sizeof(buffer),file);
    if(strncmp(buffer,"P5",2)!=0){
	if(strncmp(buffer,"P2",2)!=0)
	    ERROR("pgm header (P2/P5) not found in %s",Name);
	raw=FALSE;
    }
    do fgets(buffer,sizeof(buffer),file); while(buffer[0]=='#');

    sscanf(buffer,"%d %d",&dx,&dy);
    Pic16_Malloc(pThat,dx,dy);

    do fgets(buffer,sizeof(buffer),file); while(buffer[0]=='#');
    sscanf(buffer,"%d",&v);
    if(v!=0xffff){
	if(v!=0xff)
	    ERROR("%s has wrong range %d",v,Name);
	r8=TRUE;
    }

    pp=pThat->Pel;

    for(y=0;y<dy;y++){
	for(x=0;x<dx;x++){
	    if(raw){
		v=fgetc(file);
		if(v==EOF)  ERROR("read error in %s at (%d,%d)",Name,x,y);
		if(!r8){
		    v<<=8;
		    v|=fgetc(file);
		}
	    }
	    else{
		if(fscanf(file,"%d",&v)!=1)
		    ERROR("read error in %s at (%d,%d)",Name,x,y);
	    }
	    v<<=Shift;
	    CEW16(pp+2*x,v);
	}
	pp+=pThat->S;
    }

    fclose(file);

    return TRUE;
}


/****************************************************************************/
/** load 3 colorbytes to 32 bit image with XRGB order in memory
 *
 *  \param  pThat
 *  \param  Name filename
 *  \return success (always TRUE at the moment)
 */
bool Pic32_LoadXRGB(tPic *pThat, const char *Name)
{
    FILE  *file;
    u8    *pp;
    int   x,y,dx,dy,a=0,b=0,c=0;
    char  buffer[256];
    bool	raw=TRUE;

    if(strcmp(Name,"-")==0)
	file=stdin;
    else if(!(file=fopen(Name,"r"))){
	pThat->Dx=0;
	pThat->Dy=0;
	pThat->S=0;
	pThat->Pel=NULL;
	return FALSE;
    }

    fgets(buffer,sizeof(buffer),file);
    if(strncmp(buffer,"P6",2)!=0){
	if(strncmp(buffer,"P3",2)!=0)
	    ERROR("pgm header (P3/P6) not found in %s",Name);
	raw=FALSE;
    }
    do fgets(buffer,sizeof(buffer),file); while(buffer[0]=='#');

    sscanf(buffer,"%d %d",&dx,&dy);
    Pic32_Malloc(pThat,dx,dy);

    do fgets(buffer,sizeof(buffer),file); while(buffer[0]=='#');
    sscanf(buffer,"%d",&c);
    if(c!=0xff)
	ERROR("%s has wrong range %d",Name,c);

    pp=pThat->Pel;

    if(raw){
	for(y=0;y<pThat->Dy;y++){
	    for(x=0;x<pThat->Dx;x++){
		pp[4*x]=0;
		if(fread(&pp[4*x+1],3,1,file)!=1)
		    ERROR("read error in %s  at (%d,%d)",Name,x,y);
	    }
	    pp+=pThat->S;
	}
    }
    else{
	for(y=0;y<dy;y++){
	    for(x=0;x<dx;x++){
		if(fscanf(file,"%d %d %d",&a,&b,&c)!=3)
		    ERROR("read error in %s at (%d,%d)",Name,x,y);
		pp[4*x+0]=0;
		pp[4*x+1]=a;
		pp[4*x+2]=b;
		pp[4*x+3]=c;
	    }
	    pp+=pThat->S;
	}
    }

    fclose(file);

    return TRUE;
}


/****************************************************************************/
/** load 3 colorbytes to 32 bit image with RGBX order in memory
 *
 *  \param  pThat
 *  \param  Name filename
 *  \return success (always TRUE at the moment)
 */
bool Pic32_LoadRGBX(tPic *pThat, const char *Name)
{
    FILE  *file;
    u8    *pp;
    int   x,y,dx,dy,a=0,b=0,c=0;
    char  buffer[256];
    bool	raw=TRUE;

    if(strcmp(Name,"-")==0)
	file=stdin;
    else if(!(file=fopen(Name,"r"))){
	pThat->Dx=0;
	pThat->Dy=0;
	pThat->S=0;
	pThat->Pel=NULL;
	return FALSE;
    }

    fgets(buffer,sizeof(buffer),file);
    if(strncmp(buffer,"P6",2)!=0){
	if(strncmp(buffer,"P3",2)!=0)
	    ERROR("pgm header (P3/P6) not found in %s",Name);
	raw=FALSE;
    }
    do fgets(buffer,sizeof(buffer),file); while(buffer[0]=='#');

    sscanf(buffer,"%d %d",&dx,&dy);
    Pic32_Malloc(pThat,dx,dy);

    do fgets(buffer,sizeof(buffer),file); while(buffer[0]=='#');
    sscanf(buffer,"%d",&c);
    if(c!=0xff)
	ERROR("%s has wrong range %d",Name,c);

    pp=pThat->Pel;

    if(raw){
	for(y=0;y<pThat->Dy;y++){
	    for(x=0;x<pThat->Dx;x++){
		pp[4*x+3]=0;
		if(fread(&pp[4*x],3,1,file)!=1)
		    ERROR("read error in %s  at (%d,%d)",Name,x,y);
	    }
	    pp+=pThat->S;
	}
    }
    else{
	for(y=0;y<dy;y++){
	    for(x=0;x<dx;x++){
		if(fscanf(file,"%d %d %d",&a,&b,&c)!=3)
		    ERROR("read error in %s at (%d,%d)",Name,x,y);
		pp[4*x+0]=a;
		pp[4*x+1]=b;
		pp[4*x+2]=c;
		pp[4*x+3]=0;
	    }
	    pp+=pThat->S;
	}
    }

    fclose(file);

    return TRUE;
}


/****************************************************************************/
/** determine the type of picture contained in a file
 *
 *  \param Name the filename
 *  \return PIC_G[32|16|8] | NIL
 */
int Pic_FileType(const char *Name)
{
    FILE		*file;
    int		dx,dy;
    u32		v;
    int		t,res;
    char		buffer[256];

    if(!(file=fopen(Name,"r")))
	return PIC_NONE;

    fgets(buffer,sizeof(buffer),file);
    if(strncmp(buffer,"P3",2)==0||strncmp(buffer,"P6",2)==0)
	t=PIC_XRGB;
    else{
	if(strncmp(buffer,"P5",2)!=0){
	    if(strncmp(buffer,"P2",2)!=0)
		ERROR("pgm header (P2/P5) not found in %s",Name);
	}
	do fgets(buffer,sizeof(buffer),file); while(buffer[0]=='#');

	res=sscanf(buffer,"%d %d",&dx,&dy); DLOGd(res);
	if(!res) ERROR("cannot read size of %s");
	else if(res==1){
	    do fgets(buffer,sizeof(buffer),file); while(buffer[0]=='#');
	    sscanf(buffer,"%d",&dy);
	}

	DLOGd(dx); DLOGd(dy);

	do fgets(buffer,sizeof(buffer),file); while(buffer[0]=='#');
	DLOG("buf %s",buffer);
	sscanf(buffer,"%u",&v); DLOGx(v);
	if(v<=0xff)
	    t=PIC_G8;
	else if(v<=0xffff)
	    t=PIC_G16;
	else
	    t=PIC_G32;
    }
    ;	DLOGd(t);
    fclose(file);
    return t;
}


/***************************************
 *  
 **************************************/

void Pic16_BGRfromU8(tPic *pThat, tPic *pSrc)
{
    u16		*pd;
    u8    	*ps;
    int   	x,y,pel,r,g,b;

    MUST_Ge(pThat->Dx,pSrc->Dx);
    MUST_Ge(pThat->Dy,pSrc->Dy);

    pd=(u16*)pThat->Pel;
    ps=pSrc->Pel;
	
    for(y=0;y<pSrc->Dy;y++)
    {
	for(x=0;x<pSrc->Dx;x++)
	{
	    pel=ps[x];
	    r=pel>>3;		g=pel>>2;		b=pel>>3;
	    pd[x]=r<<11|g<<5|b;
	}
	pd+=pThat->S/2;
	ps+=pSrc->S;
    }
}


/***************************************
 *  
 **************************************/

void Pic32_XBGRfromU8(tPic *pThat, tPic *pSrc)
{
    u32		*pd;
    u8    	*ps;
    int   	x,y,pel;

    MUST_Ge(pThat->Dx,pSrc->Dx);
    MUST_Ge(pThat->Dy,pSrc->Dy);

    pd=(u32*)pThat->Pel;
    ps=pSrc->Pel;
	
    for(y=0;y<pSrc->Dy;y++)
    {
	for(x=0;x<pSrc->Dx;x++)
	{
	    pel=ps[x];
	    pd[x]=pel<<16|pel<<8|pel;
	}
	pd+=pThat->S/4;
	ps+=pSrc->S;
    }
}


/***************************************
 *  
 **************************************/

void Pic16_DrawCross(tPic *pThat, int x, int y, int color)
{
    *(u16*)pPEL16(pThat,x  ,y  )=color;

    *(u16*)pPEL16(pThat,x-1,y-1)=color;
    *(u16*)pPEL16(pThat,x+1,y-1)=color;
    *(u16*)pPEL16(pThat,x-1,y+1)=color;
    *(u16*)pPEL16(pThat,x+1,y+1)=color;
#if 1
    *(u16*)pPEL16(pThat,x-2,y-2)=color;
    *(u16*)pPEL16(pThat,x+2,y-2)=color;
    *(u16*)pPEL16(pThat,x-2,y+2)=color;
    *(u16*)pPEL16(pThat,x+2,y+2)=color;
#endif
}


/***************************************
 *  
 **************************************/

void Pic16_DrawRect(tPic *pThat, int x, int y, int color)
{
    *(u16*)pPEL16(pThat,x-1,y-1)=color;
    *(u16*)pPEL16(pThat,x+0,y-1)=color;
    *(u16*)pPEL16(pThat,x+1,y-1)=color;

    *(u16*)pPEL16(pThat,x-1,y+0)=color;
    *(u16*)pPEL16(pThat,x+1,y+0)=color;

    *(u16*)pPEL16(pThat,x-1,y+1)=color;
    *(u16*)pPEL16(pThat,x+0,y+1)=color;
    *(u16*)pPEL16(pThat,x+1,y+1)=color;
}


/***************************************
 *  
 **************************************/

void Pic32_DrawRect(tPic *pThat, int x, int y, int color)
{
    *(u32*)pPEL32(pThat,x-1,y-1)=color;
    *(u32*)pPEL32(pThat,x+0,y-1)=color;
    *(u32*)pPEL32(pThat,x+1,y-1)=color;

    *(u32*)pPEL32(pThat,x-1,y+0)=color;
    *(u32*)pPEL32(pThat,x+1,y+0)=color;

    *(u32*)pPEL32(pThat,x-1,y+1)=color;
    *(u32*)pPEL32(pThat,x+0,y+1)=color;
    *(u32*)pPEL32(pThat,x+1,y+1)=color;
}


/***************************************
 *  
 **************************************/

void Pic16_DrawLine(tPic *pThat, int ax, int ay,  int bx, int by, unsigned int color)
{
    // Bresenham
    int width = pThat->Dx;
    int height = pThat->Dy;
    //int stride = pThat->S;

    float x, y, initialX, initialY,  pdx, pdy, dx,dy, incx,incy;
    int xDiscr, yDiscr, n, t;

    if(ax==bx && ay==by)	return;

    dx = (float)(bx - ax);
    dy = (float)(by - ay);

    incx = (dx >= 0.0) ? 1.0f : -1.0f;
    incy = (dy >= 0.0) ? 1.0f : -1.0f;

    dx *= incx;
    dy *= incy;

    if (dx > dy)
    {
	pdx = incx;
	pdy = incy * dy / dx;
	n = (int)dx;
    }
    else
    {
	pdx = incx * dx / dy;
	pdy = incy;
	n = (int)dy;
    }

    if (dx > dy)
    {
	int discretisedX =  (int)(ax + 0.5);
	float deltaSubPxX = (float)discretisedX - ax;
	float deltaSubPxY = deltaSubPxX * incx * incy * dy / dx;
	initialX = (float)discretisedX;
	initialY = ay + deltaSubPxY;
    }
    else
    {
	int discretisedY =  (int)(ay + 0.5);
	float deltaSubPxY = (float)(discretisedY) - ay;
	float deltaSubPxX = deltaSubPxY * incy * incx * dx / dy;
	initialY = (float)discretisedY;
	initialX = ax + deltaSubPxX;
    }
    for (t=0; t <=n; t++)
    {
	x = initialX + t * pdx;
	y = initialY + t * pdy;
	xDiscr = (int)(x + 0.5);
	yDiscr = (int)(y + 0.5);
	if (xDiscr >= 0 && yDiscr >= 0 && xDiscr < width && yDiscr < height)
	    *(u16*)pPEL16(pThat,xDiscr,yDiscr)=color;
    }
}


/***************************************
 *  todo: solve this with an include kernel
 **************************************/

void Pic32_DrawLine(tPic *pThat, int ax, int ay,  int bx, int by, unsigned int color)
{
    // Bresenham
    int width = pThat->Dx;
    int height = pThat->Dy;
    //int stride = pThat->S;

    float x, y, initialX, initialY,  pdx, pdy, dx,dy, incx,incy;
    int xDiscr, yDiscr, n, t;

    if(ax==bx && ay==by)	return;

    dx = (float)(bx - ax);
    dy = (float)(by - ay);

    incx = (dx >= 0.0) ? 1.0f : -1.0f;
    incy = (dy >= 0.0) ? 1.0f : -1.0f;

    dx *= incx;
    dy *= incy;

    if (dx > dy)
    {
	pdx = incx;
	pdy = incy * dy / dx;
	n = (int)dx;
    }
    else
    {
	pdx = incx * dx / dy;
	pdy = incy;
	n = (int)dy;
    }

    if (dx > dy)
    {
	int discretisedX = (int)(ax + 0.5);
	float deltaSubPxX = (float)discretisedX - ax;
	float deltaSubPxY = deltaSubPxX * incx * incy * dy / dx;
	initialX = (float)discretisedX;
	initialY = ay + deltaSubPxY;
    }
    else
    {
	int discretisedY = (int)(ay + 0.5);
	float deltaSubPxY = (float)(discretisedY) - ay;
	float deltaSubPxX = deltaSubPxY * incy * incx * dx / dy;
	initialY = (float)discretisedY;
	initialX = ax + deltaSubPxX;
    }
    for (t=0; t <=n; t++)
    {
	x = initialX + t * pdx;
	y = initialY + t * pdy;
	xDiscr = (int)(x + 0.5);
	yDiscr = (int)(y + 0.5);
	if (xDiscr >= 0 && yDiscr >= 0 && xDiscr < width && yDiscr < height)
	    *(u32*)pPEL32(pThat,xDiscr,yDiscr)=color;
    }
}

