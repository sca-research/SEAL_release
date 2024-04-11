#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "chain.h"

//Add a new node into the chain. Use lasthint to cut traversal. 
//New node is added at the end of the chain.
ChainNode *AddNode(ChainNode ** chain, ChainNode * lasthint, void *content)
{
    ChainNode *last;
    ChainNode *newnode = NULL;

    if (NULL == *chain)		//Create a new chain. lasthint ignored.
	{
	    *chain = (ChainNode *) malloc(sizeof(ChainNode));
	    (*chain)->prev = NULL;
	    (*chain)->next = NULL;
	    (*chain)->content = content;
	    return *chain;
	}

    //Move to the last node. Search from lasthint, or the start if no lasthint given.
    for (last = ((NULL == lasthint) ? *chain : lasthint);
	 NULL != last->next; last = last->next) ;

    //Insert a new node after the last.
    newnode = (ChainNode *) malloc(sizeof(ChainNode));
    newnode->content = content;
    newnode->next = NULL;
    newnode->prev = last;
    last->next = newnode;
    return newnode;
}

//Delete a node from a chain. Returns the previous node of the deleted.
//FF is called the node to free its content. A NULL FF keeps the content.
//Returns 1 if the head of chain is changed, or 0 otherwise.
int DelNode(ChainNode ** chain, ChainNode * node, FreeFunc FF)
{
    ChainNode *current = node, *prev = node->prev, *next = node->next;

    //Do nothing if current is NULL.
    if (NULL == current)
	{
	    return 0;
	}

    //Free the current node.
    if (NULL != FF)
	{
	    FF(current->content);
	}
    free(current);

    //Rearrange the chain.
    if (NULL == prev && NULL == next)	//current is the only node. 
	{
	    *chain = NULL;	//Reset the chain.
	    return 1;
	}
    else if (NULL == prev)	//current is the first node.
	{
	    next->prev = NULL;
	    *chain = next;
	    return 1;
	}
    else if (NULL == next)	//current is the last node.
	{
	    prev->next = NULL;
	    return 0;
	}
    else			//current is a trivial node.
	{
	    prev->next = next;
	    next->prev = prev;
	    return 0;
	}
}

//Destroy a chain. FF is called on each node to free the content. A NULL FF keeps contents.
void DelChain(ChainNode ** chain, FreeFunc FF)
{
    ChainNode *current, *next;

    if (NULL == *chain)
	{
	    //chain is empty already.
	    return;
	}

    current = *chain;
    next = current->next;

    while (true)
	{
	    //Free current node.
	    if (NULL != FF)
		{
		    FF(current->content);
		}
	    free(current);

	    //Move to next.
	    current = next;

	    if (NULL == current)
		{
		    break;
		}
	    next = current->next;
	}

    *chain = NULL;

    return;
}

//Search a chain for a node. MF is called on contents of each node against pattern.
//Returns the first node where MF returns true, or NULL if no node is matched.
ChainNode *SearchNode(ChainNode * chain, MatchFunc MF, const void *pattern)
{
    ChainNode *current = chain;

    for (current = chain; NULL != current; current = current->next)
	{
	    if (MF(current->content, pattern))
		{
		    return current;
		}
	}

    return NULL;
}

//Set enumnode to be an enumerator of chain.
void GetEnum(Enumerator * enumnode, ChainNode * chain)
{
    if (NULL == chain)
	{
	    *enumnode = NULL;
	    return;
	}

    //Move enumnode to the head.
    for (*enumnode = chain; NULL != (*enumnode)->prev;
	 *enumnode = (*enumnode)->prev) ;
    return;
}

//Enumerate over enumnode. A NULL enumnode indicates EOF.
void *EnumChain(Enumerator * enumnode)
{
    //Save current content.
    void *retcontent = (*enumnode)->content;

    //No more to enumerate.
    if (NULL == *enumnode)
	return NULL;

    //Move enumnode to the next.
    *enumnode = (*enumnode)->next;

    //Return current content.
    return retcontent;
}

//Generate an array of the contents in the chain terminated by NULL. Returns the number of elements in the array.
size_t __ListChain(void ***list, ChainNode * chain, int nnode)
{
    int i;
    ChainNode *node = chain;

    //Count number of nodes if nnode not given (i.e. 0).
    if (0 == nnode)
	{
	    for (nnode = 0; node != NULL; nnode++, node = node->next) ;
	}

    //Allocate an array of nnode elements.
    *list = (void **)malloc(sizeof(void *) * (nnode + 1));

    //Copy contents in the list.
    for (i = 0, node = chain; i < nnode; i++, node = node->next)
	{
	    (*list)[i] = node->content;
	}
    //The List terminates with a NULL.
    list[nnode] = NULL;

    return nnode;
}
