/* -*- tab-width: 4 -*- */
/** 
 *  template for windows functions that is instantiated several times	
 *
 *  \file      win_tpl.c
 *  \author    Norbert Stoeffler
 *  \date      2001-2011
 *
 */
#ifdef SHOW
{
#ifndef NO_X11
	int		x,y,dx,dy;
	int		r,g,b;
	u32		*out32;
	u16		*out16;
	u8		*pp;

	;   MUST(pThat); MUST(pPic);

	if(pThat->pX->FreeGfx)
		FreeGfx(pThat);

	r=g=b=0;
	dx=MIN(pThat->Dx,pPic->Dx);
	dy=MIN(pThat->Dy,pPic->Dy);

	memset(pThat->pX->Buf,0,pThat->Dx*pThat->Dy*lBpl);

	switch(lDepth){
	case 32:
	case 24:
		out32=(u32*)pThat->pX->Buf;
		pp=pPic->Pel;
		for(y=0;y<dy;y++){
			for(x=0;x<dx;x++){
				{SHOW}
				r=CLIP(r,0,255); g=CLIP(g,0,255); b=CLIP(b,0,255);
				out32[y*pThat->Dx+x]=(r<<16)|(g<<8)|(b<<0);
			}
			pp+=pPic->S;
		}
		break;
	case 16:
		out16=(u16*)pThat->pX->Buf;
		pp=pPic->Pel;
		for(y=0;y<dy;y++){
			for(x=0;x<dx;x++){
				{SHOW}
				r=CLIP(r>>3,0,31); g=CLIP(g>>2,0,63); b=CLIP(b>>3,0,31);
				out16[y*pThat->Dx+x]=(r<<11)|(g<<5)|(b<<0);
			}
			pp+=pPic->S;
		}
		break;
	default:
		MUST_UNDEF(lDepth);
	}

	if(pThat->pX->Z>1){
		Backup(pThat);
		Zoom(pThat);
	}

	Redraw(pThat);
#endif
}
#endif
