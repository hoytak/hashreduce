
/********************************************************************************
 *
 *  The main structures for dealing with the dgl graph structures.
 *
 ********************************************************************************/

#include "dglstructures.h"
#include "hashtable.h"
#include "hashobject.h"
#include "optimizations.h"

/************************************************************
 *
 *  All the constructors / destructors for the hash table items.
 *
 ************************************************************/

/********** DGLGraph **********/

void DGLGraph_Construct(DGLGraph *g)
{
    g->nodes = NewHashTable();
    g->edges = NewHashTable();
    g->dirty = true;
}

DGLGraph* NewDGLGraph()
{
    DGLGraph *g = ALLOCATEDGLGraph();
    DGLGraph_Construct(g);
    return g;
}

void DGLGraph_Destroy(DGLGraph *g)
{
    O_DECREF(g->nodes);
    O_DECREF(g->edges);
}

/********** DGLGraphNode **********/
void DGLGraphNode_Construct(DGLGraphNode *n)
{
    n->edges = NewHashTable();
}

void DGLGraphNode_Destroy(DGLGraphNode *n)
{
    O_DECREF(n->edges);
}

/********** DGLGraphEdge **********/
void DGLGraphEdge_Construct(DGLGraphEdge *e)
{
    e->nodes = NewHashTable();
}

void DGLGraphEdge_Destroy(DGLGraphEdge *e)
{
    O_DECREF(e->nodes);
}

/********** _DGLGraphNodeReference **********/

_DGLGraphNodeReference* NewDGLGraphNodeReference(DGLGraphNode *node)
{
    _DGLGraphNodeReference* nr = ALLOCATE_DGLGraphNodeReference();
    nr->node = node;
    O_INCREF(node);
    return nr;
}


void _DGLGraphNodeReference_Destroy(_DGLGraphNodeReference *nr)
{
    O_DECREF(nr->node);
}

/********** _DGLGraphEdgeReference **********/
_DGLGraphEdgeReference* NewDGLGraphEdgeReference(DGLGraphEdge *edge)
{
    _DGLGraphEdgeReference* er = ALLOCATE_DGLGraphEdgeReference();
    er->edge = edge;
    O_INCREF(edge);
    return er;
}

void _DGLGraphEdgeReference_Destroy(_DGLGraphEdgeReference *er)
{
    O_DECREF(er->edge);
}

DEFINE_OBJECT(
    /* Name. */     DGLGraph,
    /* BaseType */  HashObject,
    /* construct */ DGLGraph_Construct,
    /* delete */    DGLGraph_Destroy,
    /* Duplicate */ NULL);

DEFINE_OBJECT(
    /* Name. */     DGLGraphNode,
    /* BaseType */  HashObject,
    /* construct */ DGLGraphNode_Construct,
    /* delete */    DGLGraphNode_Destroy,
    /* Duplicate */ NULL);

DEFINE_OBJECT(
    /* Name. */     DGLGraphEdge,
    /* BaseType */  HashObject,
    /* construct */ DGLGraphEdge_Construct,
    /* delete */    DGLGraphEdge_Destroy,
    /* Duplicate */ NULL);

DEFINE_OBJECT(
    /* Name. */     _DGLGraphNodeReference,
    /* BaseType */  HashObject,
    /* construct */ NULL,
    /* delete */    _DGLGraphNodeReference_Destroy,
    /* Duplicate */ NULL);

DEFINE_OBJECT(
    /* Name. */     _DGLGraphEdgeReference,
    /* BaseType */  HashObject,
    /* construct */ NULL,
    /* delete */    _DGLGraphEdgeReference_Destroy,
    /* Duplicate */ NULL);


/************************************************************
 *
 *  Now the main routines for constructing such graphs.
 *
 ************************************************************/

/********  Node Retrieval / Construction ********/

inline DGLGraphNode* _DGLGraphNodeByKey(DGLGraph *g, HashObject *key)
{
    /* See if anything is in there. */
    HashObject *retrieved_n = Ht_Get(g->nodes, key);

    if(retrieved_n == NULL)
    {
	DGLGraphNode *n = ConstructDGLGraphNode();
	assert(n->edges != NULL);
	H_Copy(n, key);
	Ht_Give(g->nodes, (HashObject*)n);

	O_DECREF(key);

	assert(O_REF_COUNT(n) >= 1);

	return n;
    }
    else
    {
	O_DECREF(key);

	assert(O_REF_COUNT(retrieved_n) >= 1);

	return (DGLGraphNode*) retrieved_n;
    }
}

/* Retrieves or creates nodes in the dgl graphs. */
DGLGraphNode* DGLGraphNodeByName(DGLGraph *g, const char *name)
{
    assert(g != NULL);
    return _DGLGraphNodeByKey(g, Hf_FromString(NULL, name));
}

/* Retrieves or creates nodes in the dgl graphs. */
DGLGraphNode* DGLGraphNodeByNumber(DGLGraph *g, signed long number)
{
    assert(g != NULL);
    return _DGLGraphNodeByKey(g, Hf_FromInt(NULL, number));
}

/********  Edge Retrieval / Construction ********/

inline DGLGraphEdge* _DGLGraphEdgeByKey(DGLGraph *g, HashObject *key)
{
    assert(g != NULL);
    assert(key != NULL);

    /* See if anything is in there. */
    HashObject *retrieved_e = Ht_Get(g->edges, key);

    if(retrieved_e == NULL)
    {
	DGLGraphEdge *e = ConstructDGLGraphEdge();
	assert(e->nodes != NULL);

	H_Copy(e, key);
	Ht_Give(g->edges, (HashObject*)e);

	O_DECREF(key);
	assert(O_REF_COUNT(e) >= 1);

	return e;
    }
    else
    {
	O_DECREF(key);
	assert(O_REF_COUNT(retrieved_e) >= 1);

	return (DGLGraphEdge*) retrieved_e;
    }
}

/* Retrieves or creates edges in the dgl graphs. */
DGLGraphEdge* DGLGraphEdgeByName(DGLGraph *g, const char *name)
{
    assert(g != NULL);
    return _DGLGraphEdgeByKey(g, Hf_FromString(NULL, name));
}

/* Retrieves or creates edges in the dgl graphs. */
DGLGraphEdge* DGLGraphEdgeByNumber(DGLGraph *g, signed long number)
{
    assert(g != NULL);
    return _DGLGraphEdgeByKey(g, Hf_FromInt(NULL, number));
}


/**************************************************
 *
 *  Construction routines for the graphs.  
 *
 **************************************************/

static inline void DGLGraph_Changed(DGLGraph *g);

void DGLGraph_Connect(DGLGraph *g, DGLGraphEdge *e, DGLGraphNode *n, 
		      markertype valid_start, markertype valid_end)
{

    /* Function / data structures could be optimized QUITE a bit. */

    assert(g != NULL);
    assert(e != NULL);
    assert(n != NULL);

    assert(g->edges != NULL);
    assert(g->nodes != NULL);

    assert(n->edges != NULL);
    assert(e->nodes != NULL);

    assert(Ht_Contains(g->edges, (HashObject*)e));
    assert(Ht_Contains(g->nodes, (HashObject*)n));

    /* Create a node reference for it. */
    HashObject *node_hash = (HashObject*) n;
    HashObject *edge_hash = (HashObject*) e;

    HashObject *nr = Ht_Pop(e->nodes, node_hash);
    
    if(nr == NULL)
    {
	nr = (HashObject*)NewDGLGraphNodeReference(n);
	H_Copy(nr, node_hash);
	((_DGLGraphNodeReference*)nr)->node = n;
    }
    else
    {
	assert(O_REF_COUNT(nr) == 1);
	assert(!H_IsLocked(nr));
	assert(O_IsType(_DGLGraphNodeReference, nr));
	assert( ((_DGLGraphNodeReference*)nr)->node == n);
    }

    H_AddMarkerValidRange(nr, valid_start, valid_end);

    assert(O_IsType(Object, nr));
    assert(O_IsType(HashObject, nr));

    /* Replaces the previous key. */
    Ht_Give(e->nodes, nr);


    /****************************************/
    /* Create a reference in the edge table to the node. */
    HashObject *er = Ht_Pop(n->edges, edge_hash);

    if(er == NULL)
    {
	er = (HashObject *)NewDGLGraphEdgeReference(e);
	H_Copy(er, edge_hash);
    }
    else
    {
	assert(O_REF_COUNT(er) == 1);
	assert(!H_IsLocked(er));
	assert(O_IsType(_DGLGraphEdgeReference, er));
	assert( ((_DGLGraphEdgeReference*)er)->edge == e);
    }

    H_AddMarkerValidRange(er, valid_start, valid_end);

    /* Replaces the previous key. */
    Ht_Give(n->edges, er);
    g->dirty = true;
}


static inline void DGLGraph_Changed(DGLGraph *g)
{
    /* Build the marker skip functionality. */

    if(g->dirty == false)
	return;

    if(g->graph_hashes != NULL)
	O_DECREF(g->graph_hashes);

    if(g->current_hash != NULL)
	O_DECREF(g->current_hash);

    HashTableIterator *hti = Hti_New(g->nodes);
    obj_ptr n;
    HashTable* summary = NULL;
   
    while( (n = Hti_Next(hti)) != NULL)
	summary = Ht_Summarize_Add(summary, O_Cast(DGLGraphNode, n)->edges);

    Hti_Delete(hti);

    g->graph_hashes = Ht_Summarize_Finish(summary);

    g->current_hash = Ht_HashOfEverything(NULL, g->graph_hashes);

    g->dirty = false;
}

bool DGLGraphEqualAtMarker(DGLGraph *g1, DGLGraph *g2, markertype m)
{
    if(g1->dirty)
	DGLGraph_Changed(g1);
    
    if(g2->dirty)
	DGLGraph_Changed(g2);

    HashObject *h1 = Ht_HashAtMarkerPoint(NULL, g1->graph_hashes, m);
    HashObject *h2 = Ht_HashAtMarkerPoint(NULL, g2->graph_hashes, m);

    bool is_equal = H_Equal(h1, h2);
    
    O_DECREF(h1);
    O_DECREF(h2);
    
    return is_equal;
}

HashObject* DGLGraphViewHash(DGLGraph *g)
{
    if(g->dirty)
	DGLGraph_Changed(g);
    
    return g->current_hash;
}

HashObject* DGLGraphGetHashAtMarker(DGLGraph *g, markertype m)
{
    if(g->dirty)
	DGLGraph_Changed(g);

    return Ht_HashAtMarkerPoint(NULL, g->graph_hashes, m);
}

bool DGLGraphEqual(DGLGraph *g1, DGLGraph *g2)
{
    return H_Equal(DGLGraphViewHash(g1), DGLGraphViewHash(g2));
}
    
/************************************************************
 *
 *  Debug routines.
 *
 ************************************************************/

void DGLGraph_debug_Print(DGLGraph *g)
{
    printf("##>>>> EDGES <<<< ######################\n\n");
    Ht_debug_print(g->edges);
    Ht_MSL_debug_Print(g->edges);

    printf("##>>>> NODES <<<< ######################\n\n");
    Ht_debug_print(g->nodes);
    Ht_MSL_debug_Print(g->nodes);

    printf("##>>>> NODE Edge tables <<<< ######################\n\n");

    HashTableIterator *hti = Hti_New(g->nodes);

    DGLGraphNode *n;

    while( (n = (DGLGraphNode*)Hti_Next(hti)) != NULL)
    {
	printf("\n>> Node ");
	Hk_debug_PrintHash(H_Hash_RO(n));

	Ht_debug_print(n->edges);
	Ht_MSL_debug_Print(n->edges);
    }

    printf("##>>>> Hash List <<<< ######################\n\n");

    DGLGraph_Changed(g);
    
    Ht_debug_print(g->graph_hashes);
    Ht_MSL_debug_Print(g->graph_hashes);
}

