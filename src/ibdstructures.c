
/********************************************************************************
 *
 *  The main structures for dealing with the ibd graph structures.
 *
 ********************************************************************************/

#include "ibdstructures.h"
#include "hashtable.h"
#include "hashobject.h"
#include "optimizations.h"

/************************************************************
 *
 *  All the constructors / destructors for the hash table items.
 *
 ************************************************************/

/********** IBDGraph **********/

void IBDGraph_Construct(IBDGraph *g)
{
    g->nodes = NewHashTable();
    g->edges = NewHashTable();
    g->dirty = true;
}

IBDGraph* NewIBDGraph()
{
    IBDGraph *g = ALLOCATEIBDGraph();
    IBDGraph_Construct(g);
    return g;
}

void IBDGraph_Destroy(IBDGraph *g)
{
    O_DECREF(g->nodes);
    O_DECREF(g->edges);
}

/********** IBDGraphNode **********/
void IBDGraphNode_Construct(IBDGraphNode *n)
{
    n->edges = NewHashTable();
}

void IBDGraphNode_Destroy(IBDGraphNode *n)
{
    O_DECREF(n->edges);
}

/********** IBDGraphEdge **********/
void IBDGraphEdge_Construct(IBDGraphEdge *e)
{
    e->nodes = NewHashTable();
}

void IBDGraphEdge_Destroy(IBDGraphEdge *e)
{
    O_DECREF(e->nodes);
}

/********** _IBDGraphNodeReference **********/

_IBDGraphNodeReference* NewIBDGraphNodeReference(IBDGraphNode *node)
{
    _IBDGraphNodeReference* nr = ALLOCATE_IBDGraphNodeReference();
    nr->node = node;
    O_INCREF(node);
    return nr;
}


void _IBDGraphNodeReference_Destroy(_IBDGraphNodeReference *nr)
{
    O_DECREF(nr->node);
}

/********** _IBDGraphEdgeReference **********/
_IBDGraphEdgeReference* NewIBDGraphEdgeReference(IBDGraphEdge *edge)
{
    _IBDGraphEdgeReference* er = ALLOCATE_IBDGraphEdgeReference();
    er->edge = edge;
    O_INCREF(edge);
    return er;
}

void _IBDGraphEdgeReference_Destroy(_IBDGraphEdgeReference *er)
{
    O_DECREF(er->edge);
}

DEFINE_OBJECT(
    /* Name. */     IBDGraph,
    /* BaseType */  HashObject,
    /* construct */ IBDGraph_Construct,
    /* delete */    IBDGraph_Destroy,
    /* Duplicate */ NULL);

DEFINE_OBJECT(
    /* Name. */     IBDGraphNode,
    /* BaseType */  HashObject,
    /* construct */ IBDGraphNode_Construct,
    /* delete */    IBDGraphNode_Destroy,
    /* Duplicate */ NULL);

DEFINE_OBJECT(
    /* Name. */     IBDGraphEdge,
    /* BaseType */  HashObject,
    /* construct */ IBDGraphEdge_Construct,
    /* delete */    IBDGraphEdge_Destroy,
    /* Duplicate */ NULL);

DEFINE_OBJECT(
    /* Name. */     _IBDGraphNodeReference,
    /* BaseType */  HashObject,
    /* construct */ NULL,
    /* delete */    _IBDGraphNodeReference_Destroy,
    /* Duplicate */ NULL);

DEFINE_OBJECT(
    /* Name. */     _IBDGraphEdgeReference,
    /* BaseType */  HashObject,
    /* construct */ NULL,
    /* delete */    _IBDGraphEdgeReference_Destroy,
    /* Duplicate */ NULL);


/************************************************************
 *
 *  Now the main routines for constructing such graphs.
 *
 ************************************************************/

/********  Node Retrieval / Construction ********/

static inline IBDGraphNode* _IBDGraphNodeByKey(IBDGraph *g, HashKey key)
{
    /* See if anything is in there. */
    HashObject *retrieved_n = Ht_ViewByKey(g->nodes, key);

    Ht_ClearMarkerCache(g->nodes);

    if(retrieved_n == NULL)
    {
	IBDGraphNode *n = ConstructIBDGraphNode();
	assert(n->edges != NULL);

	Hf_COPY_FROM_KEY(n, &key);
	H_SET_MARKER_INFO(n, Mi_NEW_INVALID());

	Ht_Give(g->nodes, (HashObject*)n);

	assert(Ht_Contains(g->nodes, (HashObject*)n));

	assert(O_REF_COUNT(n) >= 1);

	return n;
    }
    else
    {
	assert(O_REF_COUNT(retrieved_n) >= 1);
	return O_Cast(IBDGraphNode, retrieved_n);
    }
}

/* Retrieves or creates nodes in the ibd graphs. */
IBDGraphNode* IBDGraphNodeByName(IBDGraph *g, const char *name)
{
    assert(g != NULL);
    IBDGraphNode* n = _IBDGraphNodeByKey(g, Hk_FromString(name));
    assert(Ht_ContainsByKey(g->nodes, Hk_FromString(name)));
    return n;
}

/* Retrieves or creates nodes in the ibd graphs. */
IBDGraphNode* IBDGraphNodeByNumber(IBDGraph *g, signed long number)
{
    assert(g != NULL);
    IBDGraphNode* n =  _IBDGraphNodeByKey(g, Hk_FromInt(number));
    assert(Ht_ContainsByKey(g->nodes, Hk_FromInt(number)));
    return n;
}

/********  Edge Retrieval / Construction ********/

static inline IBDGraphEdge* _IBDGraphEdgeByKey(IBDGraph *g, HashKey key)
{
    assert(g != NULL);

    Ht_ClearMarkerCache(g->edges);

    /* See if anything is in there. */
    HashObject *retrieved_e = Ht_ViewByKey(g->edges, key);

    if(retrieved_e == NULL)
    {
	IBDGraphEdge *e = ConstructIBDGraphEdge();
	assert(e->nodes != NULL);

	Hf_COPY_FROM_KEY(e, &key);
	H_SET_MARKER_INFO(e, Mi_NEW_INVALID());
	Ht_Give(g->edges, (HashObject*)e);

	assert(Ht_Contains(g->edges, (HashObject*)e));

	assert(O_REF_COUNT(e) >= 1);

	return e;
    }
    else
    {
	assert(O_REF_COUNT(retrieved_e) >= 1);
	return (IBDGraphEdge*) retrieved_e;
    }
}

/* Retrieves or creates edges in the ibd graphs. */
IBDGraphEdge* IBDGraphEdgeByName(IBDGraph *g, const char *name)
{
    assert(g != NULL);
    return _IBDGraphEdgeByKey(g, Hk_FromString(name));
}

/* Retrieves or creates edges in the ibd graphs. */
IBDGraphEdge* IBDGraphEdgeByNumber(IBDGraph *g, signed long number)
{
    assert(g != NULL);
    return _IBDGraphEdgeByKey(g, Hk_FromInt(number));
}


/**************************************************
 *
 *  Construction routines for the graphs.  
 *
 **************************************************/

void IBDGraph_Connect(IBDGraph *g, IBDGraphEdge *e, IBDGraphNode *n, 
		      markertype valid_start, markertype valid_end)
{
    assert(g != NULL);
    assert(e != NULL);
    assert(n != NULL);

    assert(g->edges != NULL);
    assert(g->nodes != NULL);

    assert(n->edges != NULL);
    assert(e->nodes != NULL);

    /* Clear out caches if need be. */
    Ht_ClearMarkerCache(g->edges);
    Ht_ClearMarkerCache(g->nodes);
    Ht_ClearMarkerCache(n->edges);
    Ht_ClearMarkerCache(e->nodes);

    HashObject *nh = O_Cast(HashObject, n);
    HashObject *eh = O_Cast(HashObject, e);

    assert(Ht_Contains(g->nodes, nh));
    assert(Ht_Contains(g->edges, eh));

    /* Now add in the proper marker info to each of the edges and
     * nodes. */

    HashObject *nr = Ht_View(e->nodes, nh);
    
    if(nr == NULL)
    {
	nr = (HashObject*)NewIBDGraphNodeReference(n);
	H_COPY_AS_UNMARKED(nr, nh);
	H_GIVE_MARKER_INFO(nr, Mi_New(valid_start, valid_end));
	O_Cast(_IBDGraphNodeReference, nr)->node = n;
	Ht_Give(e->nodes, nr);
    }
    else
    {
	H_ADD_MARKER_VALID_RANGE(nr, valid_start, valid_end);
	assert(O_REF_COUNT(nr) == 1);
	assert(O_IsType(_IBDGraphNodeReference, nr));
	assert( ((_IBDGraphNodeReference*)nr)->node == n);
    }

    HashObject *er = Ht_View(n->edges, eh);
    
    if(er == NULL)
    {
	er = (HashObject*)NewIBDGraphEdgeReference(e);
	H_COPY_AS_UNMARKED(er, eh);
	H_GIVE_MARKER_INFO(er, Mi_New(valid_start, valid_end));
	O_Cast(_IBDGraphEdgeReference, er)->edge = e;
	Ht_Give(n->edges, er);
    }
    else
    {
	H_ADD_MARKER_VALID_RANGE(er, valid_start, valid_end);
	assert(O_REF_COUNT(er) == 1);
	assert(O_IsType(_IBDGraphEdgeReference, er));
	assert( ((_IBDGraphEdgeReference*)er)->edge == e);
    }

    g->dirty = true;
}

static inline void IBDGraph_Refresh(IBDGraph *g)
{
    /* Build the marker skip functionality. */

    if(g->dirty == false)
	return;

    if(g->graph_hashes != NULL)
	O_DECREF(g->graph_hashes);

    if(g->current_hash != NULL)
	O_DECREF(g->current_hash);

    _HashTableInternalIterator hti;
    _Hti_INIT(g->nodes, &hti);
    IBDGraphNode *n;
    HashSequence *accumulator = NULL;

    while(_Hti_NEXT( (HashObject**)(&n), &hti))
	accumulator = Ht_Summarize_Update(accumulator, n->edges);

    g->graph_hashes = Ht_Summarize_Finish(accumulator);
    g->current_hash = Ht_HashOfEverything(NULL, g->graph_hashes);
    g->dirty = false;
}

bool IBDGraphEqualAtMarker(IBDGraph *g1, IBDGraph *g2, markertype m)
{
    if(g1->dirty)
	IBDGraph_Refresh(g1);
    
    if(g2->dirty)
	IBDGraph_Refresh(g2);

    HashObject *h1 = Ht_HashAtMarkerPoint(NULL, g1->graph_hashes, m);
    HashObject *h2 = Ht_HashAtMarkerPoint(NULL, g2->graph_hashes, m);

    bool is_equal = H_Equal(h1, h2);
    
    O_DECREF(h1);
    O_DECREF(h2);
    
    return is_equal;
}

void IBDGraphInvariantRegion(markertype *start, markertype *end, IBDGraph *g, markertype m) 
{
    if(g->dirty)
	IBDGraph_Refresh(g);
    
    HashTableMarkerIterator htmi;
    HashValidityItem hvi;

    Htmi_INIT(g->graph_hashes, &htmi);

    while(Htmi_NEXT(&hvi, &htmi) && hvi.end <= m); 

    assert(hvi.start <= m);
    assert(hvi.end > m);

    *start = hvi.start;
    *end = hvi.end;
}

/* Makes the python debugging easier; slower than previous */
markertype IBDGraphInvariantRegionLower(IBDGraph *g, markertype m)
{
    markertype start, end;

    IBDGraphInvariantRegion(&start, &end, g, m);
    
    return start;
}

/* Makes the python debugging easier; slower than previous */
markertype IBDGraphInvariantRegionUpper(IBDGraph *g, markertype m)
{
    markertype start, end;

    IBDGraphInvariantRegion(&start, &end, g, m);
    
    return end;
}

HashObject* IBDGraphViewHash(IBDGraph *g)
{
    if(g->dirty)
	IBDGraph_Refresh(g);
    
    return g->current_hash;
}

HashObject* IBDGraphGetHashAtMarker(IBDGraph *g, markertype m)
{
    if(g->dirty)
	IBDGraph_Refresh(g);

    return Ht_HashAtMarkerPoint(NULL, g->graph_hashes, m);
}

HashObject* IBDGraphGetHashOfMarkerRange(IBDGraph *g, markertype start, markertype end)
{
    if(g->dirty)
	IBDGraph_Refresh(g);

    return Ht_HashOfMarkerRange(NULL, g->graph_hashes, start, end);
}

bool IBDGraphEqual(IBDGraph *g1, IBDGraph *g2)
{
    HashObject* h1 = IBDGraphViewHash(g1);
    HashObject* h2 = IBDGraphViewHash(g2);

    return H_Equal(h1, h2);
}
    
/************************************************************
 *
 *  Debug routines.
 *
 ************************************************************/

void IBDGraph_Print(IBDGraph *g) 
{
    if(g->dirty)
	IBDGraph_Refresh(g);

    printf("IBD Graph ");
    Hk_Print(H_Hash_RO(g->current_hash));
    printf(":\n ");
    Ht_Print(g->graph_hashes);
}

void IBDGraph_debug_Print(IBDGraph *g)
{
    printf("##>>>> EDGES <<<< ######################\n\n");
    Ht_debug_Print(g->edges);

    printf("##>>>> NODES <<<< ######################\n\n");
    Ht_debug_Print(g->nodes);

    printf("##>>>> NODE Edge tables <<<< ######################\n\n");

    HashTableIterator *hti = Hti_New(g->nodes);

    IBDGraphNode *n;

    while(Hti_Next( (HashObject**)(&n), hti))
    {
	printf("\n>> Node ");
	Hk_debug_PrintHash(H_Hash_RO(n));
	
	/* Ht_debug_print(n->edges); */
	Ht_debug_Print(n->edges);
    }

    printf("##>>>> Hash List <<<< ######################\n\n");

    IBDGraph_Refresh(g);
    
    /* Ht_debug_print(g->graph_hashes); */
    Ht_debug_Print(g->graph_hashes);
}

