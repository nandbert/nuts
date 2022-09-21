/* -*- tab-width: 8 -*- */
/**
 *  double linked lists in C
 *
 *  \file      list.c
 *  \author    Norbert Stoeffler
 *  \date      2001-2011
 *
 */

#include	"list.h"
#include	"debug.h"

/*****************************************************************************
 *  exported functions
 ****************************************************************************/

/****************************************************************************/
/** initialize a linked list
 *
 *  \param pList pointer to the list
 */
void LnkList_Init(tLnkList *pList)
{
  pList->Head.pSucc=&pList->Tail;
  pList->Head.pPred=NULL;
  pList->Tail.pSucc=NULL;
  pList->Tail.pPred=&pList->Head;
  pList->Len=0;
}


/****************************************************************************/
/** add a new node at the end of a linked list
 *
 *  \param pList pointer to the list
 *  \param pNode pointer to the new node
 *  \return pNode
 */
void * LnkList_Add(tLnkList *pList, void *pNode)
{
  tNode		*pn=pNode;

  MUST_MSG(pList->Head.pSucc && pList->Tail.pPred &&
	   !pList->Head.pPred && !pList->Tail.pSucc,"list not initialized");

  ((tNode*)pList->Tail.pPred)->pSucc=pn;
  pn->pPred=pList->Tail.pPred;
  pn->pSucc=&pList->Tail;
  pList->Tail.pPred=pn;

  pList->Len++;

  return pn;
}


/****************************************************************************/
/** remove a node from a list
 *
 *  \param pList pointer to the list
 *  \param pNode pointer to the node
 *  \return pNode
 */
void * LnkList_Remove(tLnkList *pList, void *pNode)
{
  tNode		*pn=pNode;

  MUST_MSG(pList->Head.pSucc && pList->Tail.pPred &&
	   !pList->Head.pPred && !pList->Tail.pSucc,"list not initialized");

  ((tNode*)pn->pPred)->pSucc=pn->pSucc;
  ((tNode*)pn->pSucc)->pPred=pn->pPred;
  pn->pSucc=pn->pPred=NULL;

  pList->Len--;

  return pn;
}

#ifdef UNIX_GNU

/****************************************************************************/
/** free all nodes of a list
 *
 *  \param pList pointer to the list
 */
void LnkList_Free(tLnkList *pList)
{
  tNode		*pr,*ps;

  MUST_MSG(pList->Head.pSucc && pList->Tail.pPred &&
	   !pList->Head.pPred && !pList->Tail.pSucc,"list not initialized");

  for(pr=pList->Head.pSucc;pr->pSucc;){
    ps=pr->pSucc;
    free(pr);
    pr=ps;
  }

  LnkList_Init(pList);
}


/****************************************************************************/
/** free a node and its predecessors
 *
 *  \param pList pointer to the list
 *  \param pNode pointer to the node
 *  \param N number of nodes to free
 */
void LnkList_FreeNodes(tLnkList *pList, void *pNode, int N)
{
  tNode		*pp=NULL,*pc,*ps;

  MUST_MSG(pList->Head.pSucc && pList->Tail.pPred &&
	   !pList->Head.pPred && !pList->Tail.pSucc,"list not initialized");
  MUST_Gt(N,0);

  pc=pNode;
  ps=pc->pSucc;

  for(;N;pc=pp,N--){
    pp=pc->pPred; MUST(pc);
    free(pc);
    pList->Len--;
  }

  pp->pSucc=ps;
  ps->pPred=pp;
}

#endif
