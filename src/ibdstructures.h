#ifndef IBDSTRUCTURES_H
#define IBDSTRUCTURES_H

#include "hashobject.h"
#include "hashtable.h"
#include "sequences.h"

/********** IBDGraph **********/

typedef struct {
    HASHOBJECT_ITEMS;
    long id;
    HashTable *nodes, *edges, *graph_hashes;
    HashObject *current_hash;
    bool dirty;
}IBDGraph;

DECLARE_OBJECT(IBDGraph);

/********** IBDGraphNode **********/
typedef struct {
    HASHOBJECT_ITEMS;
    HashTable *edges;
} IBDGraphNode;

DECLARE_OBJECT(IBDGraphNode);

/********** IBDGraphEdge **********/

typedef struct {
    HASHOBJECT_ITEMS;
    HashTable *nodes;
}IBDGraphEdge;

DECLARE_OBJECT(IBDGraphEdge);

/********** _IBDGraphNodeReference **********/

typedef struct {
    HASHOBJECT_ITEMS;
    IBDGraphNode *node;
} _IBDGraphNodeReference;

DECLARE_OBJECT(_IBDGraphNodeReference);

/********** _IBDGraphEdgeReference **********/

typedef struct {
    HASHOBJECT_ITEMS;
    IBDGraphEdge *edge;
} _IBDGraphEdgeReference;

DECLARE_OBJECT(_IBDGraphEdgeReference);


/********************************************************************************
 *
 *  Interface routines. 
 *
 ********************************************************************************/

/* Creates and returns a new, empty IBD graph structure.  */
IBDGraph* NewIBDGraph(long id);

/* Retrieves or creates nodes in the ibd graphs. */
IBDGraphNode* IBDGraphNodeByName(IBDGraph *g, const char *name);
IBDGraphNode* IBDGraphNodeByNumber(IBDGraph *g, signed long number);
IBDGraphNode* IBDGraphNodeByHashKey(IBDGraph *g, HashKey key);

/* Retrieves or creates edges in the ibd graph */
IBDGraphEdge* IBDGraphEdgeByName(IBDGraph *g, const char *name);
IBDGraphEdge* IBDGraphEdgeByNumber(IBDGraph *g, signed long number);
IBDGraphEdge* IBDGraphEdgeByHashKey(IBDGraph *g, HashKey key);

/* Attaches an edge to a given node for a specified range of time. Set
 * the markers to MARKER_PLUS_INFTY or MARKER_MINUS_INFTY to denote
 * non-closed intervals.
 */
void IBDGraph_Connect(IBDGraph *g, IBDGraphEdge *e, IBDGraphNode *n, 
		      markertype range_start, markertype range_end);

/************************************************************
 *
 *  The only query routines written right now. :-(  More coming soon.
 *
 *************************************************************/

bool IBDGraphEqualAtMarker(IBDGraph *g1, IBDGraph *g2, markertype m);
bool IBDGraphEqual(IBDGraph *g1, IBDGraph *g2);

/* Returns the pointer to the current hash of the graph. */
HashObject* IBDGraphViewHash(IBDGraph *g);
HashObject* IBDGraphGetHashAtMarker(IBDGraph *g, markertype m);
HashObject* IBDGraphGetHashOfMarkerRange(IBDGraph *g, markertype start, markertype end);

void IBDGraphInvariantRegion(markertype *start, markertype *end, IBDGraph *g1, markertype m);

/* These slower than IBDGraphInvariantRegion. */
markertype IBDGraphInvariantRegionLower(IBDGraph *g1, markertype m);
markertype IBDGraphInvariantRegionUpper(IBDGraph *g1, markertype m);

/* Returns a markerinfo object giving all the locations where the
   ibdgraph structure is equivalent to the structure at marker point
   m. The caller owns a reference to the returned markerinfo object.*/ 
mi_ptr IBDGraphInvariantSet(IBDGraph *g, markertype m);

/* These functions can be used for testing whether a (possibly
 * external) edge or node is also present in this graph. */ 
bool IBDGraphContainsEdge(IBDGraph *g, IBDGraphEdge *e);
bool IBDGraphContainsEdgeWithHashKey(IBDGraph *g, HashKey key);
bool IBDGraphContainsNode(IBDGraph *g, IBDGraphNode *n);
bool IBDGraphContainsNodeWithHashKey(IBDGraph *g, HashKey key);

/********************************************************************************
 *
 *   Routines for doing processing over collections of graph objects.
 *   The primary class here is a collection class, which holds an
 *   ordered collection of graphs.  Operations can be done on this
 *   graph class.
 *
 ********************************************************************************/

DECLARE_NEW_SEQUENCE_OBJECT(IBDGraphList, HashObject, HASHOBJECT_ITEMS, 
			    IBDGraph*, Igl, 8, true);

/* Creates a new, empty graph list.*/
IBDGraphList* NewIBDGraphList(); 

/* Gives an IBDGraph, with reference count, to the list. */
void Igl_Give(IBDGraphList* gl, IBDGraph* g);

/* Adds an IBDGraph to the list; the IBDGraphList adds a reference.*/ 
void Igl_Add(IBDGraphList* gl, IBDGraph* g);

/* Returns the size of the IBDGraphList collection. */
static inline size_t Igl_Size(const IBDGraphList* gl);

/* Returns the graph at a particular index. */
IBDGraph* Igl_ViewItem(IBDGraphList* gl, size_t index);

/************************************************************
 *
 *   Routines for obtaining collections of hashes.
 *
 ************************************************************/

/* Define a class that allows for accumulating groups of classes. */

typedef struct {
    IBDGraph **graphs;
    size_t n_graphs;
}_IBDGraphEquivalenceClass;

typedef struct {
    OBJECT_ITEMS;
    size_t n_graphs;
    size_t n_classes;
    _IBDGraphEquivalenceClass *classes;
    IBDGraph **graphs;
} IBDGraphEquivalences;

DECLARE_OBJECT(IBDGraphEquivalences);

/* Returns collections of IBD graphs in which the graphs are
 * identical at a given marker value.. */

IBDGraphEquivalences* IBDGraphEquivalenceClassesAtMarker(IBDGraphList *gl, markertype m);

/* Returns collections of IBD graphs in which the graphs are the same
 * throughout the given marker range. */

IBDGraphEquivalences* IBDGraphEquivalenceClassesOfMarkerRange(
    IBDGraphList *gl, markertype start, markertype end);

/* Returns collections of IBD graphs in which the graphs are
 * completely identical. */

IBDGraphEquivalences* IBDGraphEquivalenceClasses(IBDGraphList *gl);

/* A convenience iterator for iterating through equivalence
 * classes. */

typedef struct {
    MEMORY_POOL_ITEMS;
    IBDGraphEquivalences *ige;
    size_t next_graph_index;
    size_t next_class_index;
    size_t n_left_in_class;
}IGEIterator;


/* Creates a new graph equivalences deal. */
IGEIterator *Igei_New(IBDGraphEquivalences *ige);

/* Fills the values pointed to by ibd_graph and equivalence_class with
 * the next ones in the sequence.  Returns false when there are no
 * more (no values filled then). */

bool Igei_Next(IBDGraph **ibd_graph, size_t* equivalence_class, IGEIterator *igei);

/* Call when things are done to clean up the iterator. */
void Igei_Finish(IGEIterator *igei);


/********************************************************************************
 *
 *  Convenience functions to sort and print equivalence classes; works
 *  on id's and class sizes.
 *
 ********************************************************************************/

/* call first */
void IBDGraphEquivalences_InplaceSort(IBDGraphEquivalences* ige);

/* prints output */
void IBDGraphEquivalences_Print(IBDGraphEquivalences* ige);

/************************************************************
 *
 *  Debug routines.
 *
 ************************************************************/

void IBDGraph_debug_Print(IBDGraph *g);
void IBDGraph_Print(IBDGraph *g);


#endif





