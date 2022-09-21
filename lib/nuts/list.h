/* -*- tab-width: 8 -*- */
/** 
 *  double linked lists in C
 *
 *  \file      list.h
 *  \author    Norbert Stoeffler
 *  \date      2001-2011
 *
 */

#ifndef LIST_H
#define LIST_H

#include	"basic.h"


/*****************************************************************************
 *  defines
 ****************************************************************************/

/** iterate a pointer through the nodes of a linked list
 *  \param l the list
 *  \param p pointer of suitable type
 */
#define LNKLIST_FOR(l,p)	for((p)=(void*)(l).Head.pSucc;\
				      ((tNode*)(p))->pSucc;\
				  (p)=(void*)((tNode*)(p))->pSucc)

/** is a linked list empty?
 */
#define LNKLIST_ISEMPTY(l)	((l).Head.pSucc==&(l).Tail)

/** get pointer to first node in list
 */
#define LNKLIST_FIRST(l)	((l).Head.pSucc)


/** get pointer to first node in list
 */
#define LNKLIST_LAST(l)		((l).Tail.pPred)

#ifdef UNIX_GNU

#include	<malloc.h>

/** allocate a new object of type t and return a pointer to it
 */
#define NEW(t)			({ void *p=calloc(sizeof(t),1); MUST(p); p; })

#endif

/*****************************************************************************
 *  types
 ****************************************************************************/

typedef struct sNode {
  void		*pSucc;
  void		*pPred;
} tNode;


typedef struct {
  tNode		Head;
  tNode		Tail;
  int		Len;
} tLnkList;


/*****************************************************************************
 *  exported functions
 ****************************************************************************/

void  LnkList_Init(tLnkList *pList);
void* LnkList_Add(tLnkList *pList, void *pNode);
void* LnkList_Remove(tLnkList *pList, void *pNode);
void  LnkList_Free(tLnkList *pList);
void  LnkList_FreeNodes(tLnkList *pList, void *pNode, int N);


#endif /* LIST_H */
