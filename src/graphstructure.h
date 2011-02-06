/*


#ifndef GRAPHSTRUCTURE_H
#define GRAPHSTRUCTURE_H



#include 

typedef 




typedef struct {
    unsigned long keys[NUM_HASH_KEYS];
    unsigned long n_nodes;
} PGKey;



/*****************************************
 *
 * Graph components and graph edges.
 *
 ****************************************/

typedef struct {
    char *label;
    size_t label_size;

    unsigned long hash;

    PGEdgeLabel label;

    size_t n_changes[2];
    size_t *markers[2];
    PGNode *nodes[2];

} PGEdge;


typedef struct {
    
    
    

};


typedef struct {
    size_t n_edges;
    size_t n_edges_allocated;

    PGEdge
    


    
    /* Flags for use within graph algorithms. */

} PGNode;


typedef struct {
    
    size_t n_nodes;
    PGNode* nodes;

} PGraph;


#endif
