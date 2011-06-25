#ifndef IBDSTRUCTURES_H
#define IBDSTRUCTURES_H

#include "hashobject.h"
#include "hashtable.h"

/********** IBDGraph **********/

typedef struct {
    HASHOBJECT_ITEMS;
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
IBDGraph* NewIBDGraph();

/* Retrieves or creates nodes in the ibd graphs. */
IBDGraphNode* IBDGraphNodeByName(IBDGraph *g, const char *name);
IBDGraphNode* IBDGraphNodeByNumber(IBDGraph *g, signed long number);

/* Retrieves or creates edges in the ibd graph */
IBDGraphEdge* IBDGraphEdgeByName(IBDGraph *g, const char *name);
IBDGraphEdge* IBDGraphEdgeByNumber(IBDGraph *g, signed long number);

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

/************************************************************
 *
 *  Debug routines.
 *
 ************************************************************/

void IBDGraph_debug_Print(IBDGraph *g);

#endif





