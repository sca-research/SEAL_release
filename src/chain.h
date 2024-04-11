#ifndef _CHAIN_H
#define _CHAIN_H
#ifdef __cplusplus
extern "C" {
#endif

    typedef void (*FreeFunc)(void *);
    typedef int (*MatchFunc)(const void *chainop, const void *targetop);

    //General structure for chains.
    typedef struct _ChainNode {
	struct _ChainNode *prev;
	struct _ChainNode *next;
	void *content;
    } ChainNode;
    typedef ChainNode *Enumerator;

    //Add a new node into the chain. Use lasthint to cut traversal.
    //New node is added at the end of the chain.
    ChainNode *AddNode(ChainNode ** chain, ChainNode * hint, void *content);
    //Delete note from chain.
    int DelNode(ChainNode ** chain, ChainNode * node, FreeFunc FF);
    //Delete the whole chain.
    void DelChain(ChainNode ** node, FreeFunc FF);
    //Search a node with pattern using MF in chain.
    ChainNode *SearchNode(ChainNode * chain, MatchFunc MF, const void *pattern);
    //Set enumnode to be an enumerator of chain.
    void GetEnum(Enumerator * enumnode, ChainNode * chain);
    //Enumerate over enumnode. A NULL enumnode indicates EOF.
    void *EnumChain(Enumerator * enumnode);
    //Generate an array of the contents in the chain terminated by NULL. Returns the number of elements in the array.
    size_t __ListChain(void ***list, ChainNode * chain, int nnode);
#define ListChain(x,y,z) __ListChain((void***)x,y,z)

#ifdef __cplusplus
}
#endif
#endif
