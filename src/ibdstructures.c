
/********************************************************************************
 *
 *  The main structures for dealing with the ibd graph structures.
 *
 ********************************************************************************/

#include "ibdstructures.h"
#include "hashtable.h"
#include "hashobject.h"
#include "optimizations.h"
#include "ksort.h"

/************************************************************
 *
 *  All the constructors / destructors for the hash table items.
 *
 ************************************************************/

/********** IBDGraph **********/

void _IBDGraph_Construct(IBDGraph *g)
{
    g->nodes = NewHashTable();
    g->edges = NewHashTable();
    g->dirty = true;
}

IBDGraph* NewIBDGraph(long id)
{
    IBDGraph *g = ALLOCATEIBDGraph();
    g->id = id;
    _IBDGraph_Construct(g);
    return g;
}

void _IBDGraph_Destroy(IBDGraph *g)
{
    O_DECREF(g->nodes);
    O_DECREF(g->edges);
}

/********** IBDGraphNode **********/
void _IBDGraphNode_Construct(IBDGraphNode *n)
{
    n->edges = NewHashTable();
}

void _IBDGraphNode_Destroy(IBDGraphNode *n)
{
    O_DECREF(n->edges);
}

/********** IBDGraphEdge **********/
void _IBDGraphEdge_Construct(IBDGraphEdge *e)
{
    e->nodes = NewHashTable();
}

void _IBDGraphEdge_Destroy(IBDGraphEdge *e)
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
    /* construct */ _IBDGraph_Construct,
    /* delete */    _IBDGraph_Destroy);

DEFINE_OBJECT(
    /* Name. */     IBDGraphNode,
    /* BaseType */  HashObject,
    /* construct */ _IBDGraphNode_Construct,
    /* delete */    _IBDGraphNode_Destroy);

DEFINE_OBJECT(
    /* Name. */     IBDGraphEdge,
    /* BaseType */  HashObject,
    /* construct */ _IBDGraphEdge_Construct,
    /* delete */    _IBDGraphEdge_Destroy);

DEFINE_OBJECT(
    /* Name. */     _IBDGraphNodeReference,
    /* BaseType */  HashObject,
    /* construct */ NULL,
    /* delete */    _IBDGraphNodeReference_Destroy);

DEFINE_OBJECT(
    /* Name. */     _IBDGraphEdgeReference,
    /* BaseType */  HashObject,
    /* construct */ NULL,
    /* delete */    _IBDGraphEdgeReference_Destroy);


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

IBDGraphNode* IBDGraphNodeByHashKey(IBDGraph *g, HashKey key)
{
    assert(g != NULL);
    IBDGraphNode* n = _IBDGraphNodeByKey(g, key);
    assert(Ht_ContainsByKey(g->nodes, key));
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

IBDGraphEdge* IBDGraphEdgeByHashKey(IBDGraph *g, HashKey key)
{
    assert(g != NULL);
    return _IBDGraphEdgeByKey(g, key);
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
	H_GIVE_MARKER_INFO(nr, Mi_NEW(valid_start, valid_end));
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
    hvi.start = 0;
    hvi.end = 0;

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

mi_ptr IBDGraphInvariantSet(IBDGraph *g, markertype m)
{
    HashObject *h = IBDGraphGetHashAtMarker(g, m);
    
    HashKey hk = *H_Hash_RO(h);
    O_DECREF(h);

    return Ht_EqualToHash(g->graph_hashes, hk);
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

bool IBDGraphContainsEdge(IBDGraph *g, IBDGraphEdge *e)
{
    return Ht_Contains(g->edges, O_Cast(HashObject, e));
}

bool IBDGraphContainsEdgeWithHashKey(IBDGraph *g, HashKey key)
{
    return Ht_ContainsByKey(g->edges, key);
}

bool IBDGraphContainsNode(IBDGraph *g, IBDGraphNode *n)
{
    return Ht_Contains(g->nodes, O_Cast(HashObject, n));
}

bool IBDGraphContainsNodeWithHashKey(IBDGraph *g, HashKey key)
{
    return Ht_ContainsByKey(g->nodes, key);
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

/********************************************************************************
 *
 *   Now for higher level processing.
 *
 ********************************************************************************/

DEFINE_NEW_SEQUENCE_OBJECT(IBDGraphList, HashObject, HASHOBJECT_ITEMS, 
			   IBDGraph*, Igl, 8, true);


/* Gives an IBDGraph, with reference count, to the list. */
void Igl_Give(IBDGraphList* gl, IBDGraph* g)
{
    Igl_Append(gl, g);
    O_DECREF(g);
}

/* Adds an IBDGraph to the list; the IBDGraphList adds a reference.*/ 
void Igl_Add(IBDGraphList* gl, IBDGraph* g)
{
    Igl_Append(gl, g);
}

/* Returns a view of graph at a particular index. */
IBDGraph* Igl_ViewItem(IBDGraphList* gl, size_t index)
{
    return Igl_AtIndex(gl, index);
}

/* These functions add in the bins. */
static inline void _IGLBins_AddItem(HashTable *bc, const HashKey *key, IBDGraph* g)
{
    assert(bc != NULL);

    HashObject *h = Ht_ViewByKey(bc, *key);
    IBDGraphList *gl;

    if(h == NULL)
    {
	gl = NewIBDGraphList();
	Hf_COPY_FROM_KEY(gl, key);
	Ht_Give(bc, O_Cast(HashObject, gl));
    }
    else
    {
	gl = O_Cast(IBDGraphList, h);
    }

    Igl_Append(gl, g);
}

/************************************************************
 * Now for handling the graph equivalences. 
 ************************************************************/

void _IBDGraphEquivalences_Destroy(IBDGraphEquivalences* ige)
{
    free(ige->classes);

    size_t i;
    for(i = 0; i < ige->n_graphs; ++i)
	O_DECREF(ige->graphs[i]);

    free(ige->graphs);
}

DEFINE_OBJECT(
    /* Name. */     IBDGraphEquivalences,
    /* BaseType */  Object,
    /* construct */ NULL,
    /* delete */    _IBDGraphEquivalences_Destroy);


IBDGraphEquivalences* NewIBDGraphEquivalences(HashTable *ht, size_t n_graphs)
{
    IBDGraphEquivalences *ige = ALLOCATEIBDGraphEquivalences();

    ige->n_classes = Ht_SIZE(ht);
    ige->classes = (_IBDGraphEquivalenceClass*)malloc(
	sizeof(_IBDGraphEquivalenceClass) * ige->n_classes);
    ige->graphs = (IBDGraph**) malloc(sizeof(IBDGraph*) * n_graphs);
    ige->n_graphs = n_graphs;

    _HashTableInternalIterator hti;

    /* Hash Table stuff -- this one is always valid. */
    _Hti_INIT(ht, &hti);

    size_t cl_idx = 0, g_idx = 0;
    IBDGraphList *gl = NULL;
    
    while(_Hti_NEXT(O_CastPtr(HashObject, &gl), &hti))
    {
	ige->classes[cl_idx].n_graphs = Igl_Size(gl);
	ige->classes[cl_idx].graphs = &(ige->graphs[g_idx]);

	IBDGraphListIterator gli;
    	Igli_INIT(gl, &gli);

	size_t count = 0;
	IBDGraph *g;
	while(Igli_NEXT(&g, &gli)) {
	    O_INCREF(g);
	    ige->graphs[g_idx++] = g;
	    assert(ige->classes[cl_idx].graphs[count] == g);

	    ++count;
	}

	assert(count == Igl_Size(gl));
    
	++cl_idx;
    }

#ifdef RUN_CONSISTENCY_CHECKS

    IGEIterator *it = Igei_New(ige);

    IBDGraph *g;
    size_t equivalence_class = 0, prev_equivalence_class = -1;
    size_t n_graphs_tested = 0, n_classes_tested = 0;

    while(Igei_Next(&g, &equivalence_class, it)) 
    {
	if(prev_equivalence_class != equivalence_class)
	{
	    prev_equivalence_class = equivalence_class;
	    assert(ige->graphs[n_graphs_tested] == ige->classes[n_classes_tested].graphs[0]);
	    ++n_classes_tested;
	}

	++n_graphs_tested;

	assert(g != NULL);

	// printf("class = %ld, graph = %ld, id = %d\n", n_classes_tested, n_graphs_tested, g->id);
    }

    Igei_Finish(it);

    assert(n_graphs_tested == n_graphs);
    assert(ige->n_graphs = n_graphs);
    assert(n_classes_tested == ige->n_classes);

#endif

    assert(g_idx == n_graphs);

    return ige;
}

IBDGraphEquivalences* IBDGraphEquivalenceClassesAtMarker(IBDGraphList *gl, markertype m)
{
    HashTable *ht = NewHashTable();
    
    IBDGraphListIterator gli;
    IBDGraph* g;
    Igli_INIT(gl, &gli);

    HashObject *h = NewHashObject();

    while(Igli_NEXT(&g, &gli))
    {
	if(g->dirty)
	    IBDGraph_Refresh(g);

	Ht_HashAtMarkerPoint(h, g->graph_hashes, m);
	_IGLBins_AddItem(ht, H_Hash_RO(h), g);
    }

    O_DECREF(h);

    IBDGraphEquivalences* ige = NewIBDGraphEquivalences(ht, Igl_Size(gl));
    O_DECREF(ht);
    return ige;
}

IBDGraphEquivalences* IBDGraphEquivalenceClasses(IBDGraphList *gl)
{
    HashTable *ht = NewHashTable();
    
    IBDGraphListIterator gli;
    IBDGraph* g;
    Igli_INIT(gl, &gli);

    HashObject* h = NewHashObject();

    while(Igli_NEXT(&g, &gli))
    {
	if(g->dirty)
	    IBDGraph_Refresh(g);

	Ht_HashOfEverything(h, g->graph_hashes);
	_IGLBins_AddItem(ht, H_Hash_RO(h), g);
    }

    IBDGraphEquivalences* ige = NewIBDGraphEquivalences(ht, Igl_Size(gl));
    O_DECREF(ht);
    O_DECREF(h);
    return ige;
}

IBDGraphEquivalences* IBDGraphEquivalenceClassesOfMarkerRange(IBDGraphList *gl, markertype start, markertype end)
{
    HashTable *ht = NewHashTable();
    
    IBDGraphListIterator gli;
    IBDGraph* g;
    Igli_INIT(gl, &gli);

    HashObject* h = NewHashObject();

    while(Igli_NEXT(&g, &gli))
    {
	if(g->dirty)
	    IBDGraph_Refresh(g);

	Ht_HashOfMarkerRange(h, g->graph_hashes, start, end);
	_IGLBins_AddItem(ht, H_Hash_RO(h), g);
    }

    IBDGraphEquivalences* ige = NewIBDGraphEquivalences(ht, Igl_Size(gl));
    O_DECREF(ht);
    O_DECREF(h);
    return ige;
}

LOCAL_MEMORY_POOL(IGEIterator);

IGEIterator *Igei_New(IBDGraphEquivalences *ige)
{
    IGEIterator* igei = Mp_NewIGEIterator();
    igei->ige = ige;
    O_INCREF(ige);

    igei->next_graph_index = 0;
    igei->next_class_index = 0;
    igei->n_left_in_class = unlikely(ige->n_classes == 0) ? 0 : ige->classes[0].n_graphs;
    assert(igei->n_left_in_class != 0);

    return igei;
}

/* Fills the values pointed to by ibd_graph and equivalence_class with
 * the next ones in the sequence.  Returns false when there are no
 * more (no values filled then). */

bool Igei_Next(IBDGraph **ibd_graph, size_t* equivalence_class, IGEIterator *igei)
{
    if(unlikely(igei->next_graph_index == igei->ige->n_graphs))
	return false;

    (*equivalence_class) = igei->next_class_index;
    (*ibd_graph) = igei->ige->graphs[igei->next_graph_index];
    
    ++igei->next_graph_index;
    
    /* Can't set the next graph if there is no next graph. */
    if(likely(igei->next_graph_index != igei->ige->n_graphs))
    {
	if( (--(igei->n_left_in_class)) == 0)
	{
	    ++igei->next_class_index;
	    assert(igei->next_class_index != igei->ige->n_classes);
	    
	    igei->n_left_in_class = igei->ige->classes[igei->next_class_index].n_graphs;
	    assert(igei->n_left_in_class != 0);
	}
    }

    return true;
}

/* Call when things are done to clean up the iterator. */
void Igei_Finish(IGEIterator *igei)
{
    O_DECREF(igei->ige);
    Mp_FreeIGEIterator(igei);
}

/********************************************************************************
 *
 * A convenience function to sort all the inputs. 
 *
 ********************************************************************************/

typedef IBDGraph* ibdgraph_cptr;
typedef _IBDGraphEquivalenceClass* ibdgraph_ec_cptr;

static inline bool _IBDGraph_within_class_lt(ibdgraph_cptr g1, ibdgraph_cptr g2)
{
    return (g1->id < g2->id);
}

static inline bool _IBDGraph_between_class_lt(_IBDGraphEquivalenceClass gc1, 
					      _IBDGraphEquivalenceClass gc2)
{
    return  ((gc1.n_graphs == gc2.n_graphs) 
	     ? (gc1.graphs[0]->id < gc2.graphs[0]->id) 
	     : (gc1.n_graphs < gc2.n_graphs));
}

KSORT_INIT(within_class, ibdgraph_cptr, _IBDGraph_within_class_lt);
KSORT_INIT(between_class, _IBDGraphEquivalenceClass, _IBDGraph_between_class_lt);

void IBDGraphEquivalences_InplaceSort(IBDGraphEquivalences* ige)
{
    size_t i;
    for(i = 0; i < ige->n_classes; ++i) 
	ks_introsort_within_class(ige->classes[i].n_graphs, ige->classes[i].graphs);

    ks_introsort_between_class(ige->n_classes, ige->classes);

    /* Now have to rearrange the graphs in the list so they correspond
     * to the classes, in order. */

    IBDGraph** graph_buf = (IBDGraph**) malloc(sizeof(IBDGraph*) * ige->n_graphs);
    
    size_t j, pos = 0;
    for(i = 0; i < ige->n_classes; ++i) 
	for(j = 0; j < ige->classes[i].n_graphs; ++j) 
	    graph_buf[pos++] = ige->classes[i].graphs[j];

    free(ige->graphs);

    ige->graphs = graph_buf;

    /* And need to rebuild the class pointers too. */
    pos = 0;
    for(i = 0; i < ige->n_classes; ++i)
    {
	ige->classes[i].graphs = &(ige->graphs[pos]);
	pos += ige->classes[i].n_graphs;
    }
}
	

void IBDGraphEquivalences_Print(IBDGraphEquivalences* ige)
{
    size_t i, j;
    for(i = 0; i < ige->n_classes; ++i) 
    {
	printf("%ld\t : ", (unsigned long)ige->classes[i].n_graphs);

	for(j = 0; j < ige->classes[i].n_graphs - 1; ++j) 
	    printf("%ld, ", ige->classes[i].graphs[j]->id);

	printf("%ld\n", ige->classes[i].graphs[j]->id);
    }
}
