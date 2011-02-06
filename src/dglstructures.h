#ifndef DGLSTRUCTURES_H
#define DGLSTRUCTURES_H

#include "hashobject.h"
#include "hashtable.h"

/********** DGLGraph **********/

typedef struct {
    HASHOBJECT_ITEMS;
    HashTable *nodes, *edges, *graph_hashes;
    HashObject *current_hash;
    bool dirty;
}DGLGraph;

DECLARE_OBJECT(DGLGraph);

/********** DGLGraphNode **********/
typedef struct {
    HASHOBJECT_ITEMS;
    HashTable *edges;
} DGLGraphNode;

DECLARE_OBJECT(DGLGraphNode);

/********** DGLGraphEdge **********/

typedef struct {
    HASHOBJECT_ITEMS;
    HashTable *nodes;
}DGLGraphEdge;

DECLARE_OBJECT(DGLGraphEdge);

/********** _DGLGraphNodeReference **********/

typedef struct {
    HASHOBJECT_ITEMS;
    DGLGraphNode *node;
} _DGLGraphNodeReference;

DECLARE_OBJECT(_DGLGraphNodeReference);

/********** _DGLGraphEdgeReference **********/

typedef struct {
    HASHOBJECT_ITEMS;
    DGLGraphEdge *edge;
} _DGLGraphEdgeReference;

DECLARE_OBJECT(_DGLGraphEdgeReference);


/********************************************************************************
 *
 *  Interface routines. 
 *
 ********************************************************************************/

/* Creates and returns a new, empty DGL graph structure.  */
DGLGraph* NewDGLGraph();

/* Retrieves or creates nodes in the dgl graphs. */
DGLGraphNode* DGLGraphNodeByName(DGLGraph *g, const char *name);
DGLGraphNode* DGLGraphNodeByNumber(DGLGraph *g, signed long number);

/* Retrieves or creates edges in the dgl graph */
DGLGraphEdge* DGLGraphEdgeByName(DGLGraph *g, const char *name);
DGLGraphEdge* DGLGraphEdgeByNumber(DGLGraph *g, signed long number);

/* Attaches an edge to a given node for a specified range of time. Set
 * the markers to MARKER_PLUS_INFTY or MARKER_MINUS_INFTY to denote
 * non-closed intervals.
 */
void DGLGraph_Connect(DGLGraph *g, DGLGraphEdge *e, DGLGraphNode *n, 
		      markertype range_start, markertype range_end);

/************************************************************
 *
 *  The only query routines written right now. :-(  More coming soon.
 *
 *************************************************************/

bool DGLGraphEqualAtMarker(DGLGraph *g1, DGLGraph *g2, markertype m);
bool DGLGraphEqual(DGLGraph *g1, DGLGraph *g2);

/* Returns the pointer to the current hash of the graph. */
HashObject* DGLGraphViewHash(DGLGraph *g);
HashObject* DGLGraphGetHashAtMarker(DGLGraph *g, markertype m);

/************************************************************
 *
 *  Debug routines.
 *
 ************************************************************/

void DGLGraph_debug_Print(DGLGraph *g);

#endif





