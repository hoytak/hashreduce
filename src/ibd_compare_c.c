

// IBDgraph Main Program
// Elizabeth Thompson
// Lucas Koepke
// June 26, 2011

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "ibd_fatpack.c"

// include all necessary header files.

//#include "src/bitops.h"
//#include "src/brg_endian.h"
//#include "src/debugging.h"
//#include "src/dgl.h"
/* #include "src/ibdstructures.h" */
/* #include "src/errorhandling.h" */
/* //#include "src/fileparsing.h" */
/* //#include "src/generalstructures.h" */
/* //#include "src/graphmanagement.h" */
/* //#include "src/graphstructure.h" */
/* //#include "src/hashkey_inline.h" */
/* #include "src/hashkeys.h" */
/* #include "src/hashobject.h" */
/* //#include "src/hashobject_inline.h" */
/* #include "src/hashtable.h" */
/* #include "src/markerinfo.h" */
/* #include "src/memorypool.h" */
/* #include "src/randfunctions.h" */
/* #include "src/object.h" */
//#include "src/optimizations.h"
//#include "src/skiplist.h"
//#include "src/skiplists.h"


#define EDGE_NAME_BUFFER_SIZE 64


// function declarations
bool verifyKeyAndAdvanceStream(char* name, FILE* fp);
bool getKeyFromFileStream(HashKey* key, char* name, FILE* fp);
void createIBDGraphs(char* file, IBDGraphList *ibd_graphs);
int checkEdgeList(int edge, int NUM_EDGES, int edge_list[]);


int main(int argc, char **argv)
{

    double start = (double)clock();

    int marker;
    int lower = 0;
    int upper = 0;
    int graph = 0;

    int marker_loc[2];

    // char h[33];

    double elapsed1, elapsed2;

    if(argc == 4)
    {
	marker = atoi(argv[3]);
    }
    if(argc == 5)
    {
	lower = atoi(argv[3]);
	upper = atoi(argv[4]);
	graph = lower;
    }

    char help1[] = "-h";
    char help2[] = "--help";
    if(argc == 1 || strcmp(argv[1], help1) == 0 || strcmp(argv[1], help2) == 0)
    {
	// print usage
	printf("\nUsage:\t\t./ibdgraph <file> <option>\n\n");
	printf("\t\tnone\t\tPrints equivalence classes over all markers\n");
	printf("Options:\t-m <int>\tPrints equivalent graphs at marker <>\n");
	printf("\t\t-r <int> <int>\tPrints graphs same over range <> to <>\n");
	printf("\t\t-s <int> <int>\tPrints validity range of graph around marker\n");
	printf("\t\t-a <int>\tPrints entire validity range of graph\n\n");
	return 0;
    }

    IBDGraphList *ibd_graphs; // initializing graph list
    ibd_graphs = NewIBDGraphList();

    IBDGraphEquivalences *ibd_equivalences;

    // parsing the command line options
    if ((argc > 2) && (argv[2][0] == '-'))
    {
	switch (argv[2][1])
	{
	case 'h':
	    // print usage
	    printf("\nUsage:\t\t./ibdgraph <file> <NUM_GRAPHS> <NUM_EDGES> <option>\n");
	    printf("Options:\t-m <int>\tPrints equivalent graphs at marker <>\n");
	    printf("\t\t-r <int> <int>\tPrints graphs same over range <> to <>\n");
	    printf("\t\t-s <int> <int>\tPrints validity range of graph at marker\n");
	    printf("\t\t-v <int>\tPrints entire validity range of graph\n\n");
	    break;

	case 'm':
	    // get marker location
	    marker_loc[0] = atoi(argv[3]);
	    marker_loc[1] = -1;
	    if(argc != 4)
	    {
		printf("\nERROR: Incorrect number of arguments for the -m flag.\n");
		printf("Use the -h or --help flag to see options and usage.\n\n");
		break;
	    }
	    printf("\nTesting at specified marker location %d.\n\n", marker_loc[0]);
	    createIBDGraphs(argv[1], ibd_graphs);
	    elapsed1 = (double)clock() - start;
	    ibd_equivalences = IBDGraphEquivalenceClassesAtMarker(ibd_graphs, marker_loc[0]);
	    IBDGraphEquivalences_InplaceSort(ibd_equivalences);
	    IBDGraphEquivalences_Print(ibd_equivalences);

	    elapsed2 = (double)clock() - elapsed1;
	    printf("\nCreating the IBD graphs took %.2lf seconds.\n", elapsed1/CLOCKS_PER_SEC);
	    printf("Finding the equivalence classes took %.2lf seconds.", elapsed2/CLOCKS_PER_SEC);
	    break;
	    
	case 'r':
	    // get range
	    marker_loc[0] = atoi(argv[3]);
	    marker_loc[1] = atoi(argv[4]) + 1;	    
	    printf("\nTesting range from marker %d to %d\n\n", marker_loc[0], marker_loc[1]);
	    createIBDGraphs(argv[1], ibd_graphs);

	    elapsed1 = (double)clock() - start;
	    printf("\nTesting range from marker %d to %d\n\n", marker_loc[0], marker_loc[1]);

	    ibd_equivalences = IBDGraphEquivalenceClassesOfMarkerRange(ibd_graphs, marker_loc[0],
		marker_loc[1]);

	    IBDGraphEquivalences_InplaceSort(ibd_equivalences);
	    IBDGraphEquivalences_Print(ibd_equivalences);

	    elapsed2 = (double)clock() - elapsed1;
	    printf("\nCreating the IBD graphs took %.2lf seconds.\n", elapsed1/CLOCKS_PER_SEC);
	    printf("Finding the equivalence classes took %.2lf seconds.", elapsed2/CLOCKS_PER_SEC);
	    break;

	case 's':
	    // get graph number and specified marker
	    
	    printf("\nDisplaying range around marker %d for graph %d.\n\n", upper, graph);
	    createIBDGraphs(argv[1], ibd_graphs);
	    elapsed1 = (double)clock() - start;
	    printf("\nDisplaying range around marker %d for graph %d:\n\n", 
		   atoi(argv[5]), atoi(argv[6]));

	    // IBDPrintRange(ibd_graphs[atoi(argv[6]) - 1], atoi(argv[5]), h);
	    // IBDGraph_Print(ibd_graphs[atoi(argv[5]) - 1]);


	    elapsed2 = (double)clock() - elapsed1;
	    printf("\nCreating the IBD graphs took %.2lf seconds.\n", elapsed1/CLOCKS_PER_SEC);
	    printf("Printing the validity range took %.2lf seconds.", elapsed2/CLOCKS_PER_SEC);
	    break;

	case 'a':
	    // get graph number
	    createIBDGraphs(argv[1], ibd_graphs);
	    elapsed1 = (double)clock() - start;
	    printf("\nDisplaying marker validity intervals for graph %d:\n\n", atoi(argv[5]));
	    // IBDGraph_Print(ibd_graphs[atoi(argv[5]) - 1]);
	    elapsed2 = (double)clock() - elapsed1;
	    printf("\nCreating the IBD graphs took %.2lf seconds.\n", elapsed1/CLOCKS_PER_SEC);
	    printf("Printing the validity range took %.2lf seconds.", elapsed2/CLOCKS_PER_SEC);
	    break;

	default:
	    printf("\nImproper argument: %s, use option -h for usage.\n\n", argv[4]);
	    break;
	}
    }

    if(argc == 2)
    {
	marker_loc[0] = -1;
	marker_loc[1] = -1;
	createIBDGraphs(argv[1], ibd_graphs);
	elapsed1 = (double)clock() - start;
	ibd_equivalences = IBDGraphEquivalenceClasses(ibd_graphs);
	IBDGraphEquivalences_InplaceSort(ibd_equivalences);
	IBDGraphEquivalences_Print(ibd_equivalences);
	elapsed2 = (double)clock() - elapsed1;
	printf("\nCreating the IBD graphs took %.2lf seconds.\n", elapsed1/CLOCKS_PER_SEC);
	printf("Finding the equivalence classes took %.2lf seconds.", elapsed2/CLOCKS_PER_SEC);
    }
    
    O_DecRef(ibd_graphs);

    double elapsed = (double)clock() - start;
    printf("\nTotal elapsed CPU time is %.2lf seconds.\n\n", elapsed/CLOCKS_PER_SEC);

    return 0;
}



/*****************************************************/
// FUNCTION DEFINITIONS


// assume IBDGraph_ContainsNode(IBDGraph* g, int node)
// step through until node is contained in "active" graph, then start
// a new graph


void createIBDGraphs(char *file, IBDGraphList *ibd_graphs)
{

    int i, j, k;
    int ibd0;
    int ibd1;
    int changes;
    int ibd;
    int change_pos;
    int cur_range_min = 0;
    int mr_graph_min = 0;

    IBDGraph *ibd_graph;

    /* char s[33]; */
    /* s[32] = '\0'; */

    IBDGraphEdge *e;
    IBDGraphNode *cur_node;

    char edge[EDGE_NAME_BUFFER_SIZE];

    /* Using new functions IBDGraphContainsEdge and IBDGraphContainsNode
     * to build graphs by starting a new graph when a node is repeated */

    ///////////////////////////
    // Now go through NUM_GRAPHS of ibd graphs and build them

    // opening file
    FILE *fp = fopen(file, "r");

    /* printf("Preparing graphs.\n\n"); */

    // Creating IBD graphs until file is empty

    /* i keeps track of the number of graphs generated
     * j keeps track of the number of edges per graph,
     * useful for error checking */

    /* k keeps the while loop going until reach  EOF */

    i = 1;
    j = 1;
    k = 1;
	
    ibd_graph = NewIBDGraph(i); /* since first run through, no edges in graph */
	
    /* printf("\nCreated new IBDGraph %d\n\n", i); */
    
    HashKey hkey;
    HashKey key;
    
    // running through connections, start by reading a line of input
    while (getKeyFromFileStream(&key, edge, fp) 
	   && fscanf(fp, "%d %d %d", &ibd0, &ibd1, &changes) == 3)
    {
	/* printf("Building graph %d\n", i); */
	/* printf("Read %s %d %d %d\n", edge, ibd0, ibd1, changes); */
	cur_range_min = mr_graph_min; // check

	//key = Hk_FromInt(edge);
	hkey = key;
	
	// Checking if the edge just read is already in the graph
	// if so, start a new graph
	if(IBDGraphContainsEdgeWithHashKey(ibd_graph, key)) 
	{
	    Igl_Give(ibd_graphs, ibd_graph);
	    i++;
	    mr_graph_min = 0;
	    ibd_graph = NewIBDGraph(i);
	    /* printf("** ** **"); */
	}

	// Adds an edge, with one set of connections, to the graph
	e = IBDGraphEdgeByHashKey(ibd_graph, key); // edge is the edge_name
	cur_node = IBDGraphNodeByNumber(ibd_graph, ibd0); // ibd0 is the base_node

	/* H_ExtractHash(s, cur_node); */
	/* printf("  @@ Adding node %s at range [0, 1)\n", s); */
		
	// call function to add the connection
	IBDGraph_Connect(ibd_graph, e, cur_node, cur_range_min, 1);
	/* printf("  connecting initial: 0 1\n"); */
	cur_range_min = 1;
	cur_node = IBDGraphNodeByNumber(ibd_graph, ibd1);
	
	if(changes > 0) // checking if there are changes to manage
	{
	    for(k=0; k<changes; k++) // if there are changes, run through and update graph
	    {
		fscanf(fp, "%d %d", &change_pos, &ibd);
		/* printf("  Changes %d %d\n", change_pos, ibd); */

		/* H_ExtractHash(s, cur_node); */
		/* printf("  @@ Adding node %s at range [%d, %d)\n", s, change_pos, ibd); */

		// call function to add the connection
		IBDGraph_Connect(ibd_graph, e, cur_node, cur_range_min, change_pos);
		/* printf("  connecting changes: %d %d\n", cur_range_min, change_pos); */
		cur_range_min = change_pos;
		cur_node = IBDGraphNodeByNumber(ibd_graph, ibd);
	    }
	}

	// no changes in this line so just add connection
	IBDGraph_Connect(ibd_graph, e, cur_node, cur_range_min, Mr_Plus_Infinity());

	/* Reading paired line to above */
	
	if (verifyKeyAndAdvanceStream(edge, fp) && fscanf(fp, "%d %d %d", &ibd0, &ibd1, &changes) == 3)
	{
	    /* printf("Read %s %d %d %d\n", edge, ibd0, ibd1, changes); */

	    // Adds an edge, with one set of connections, to the graph
	    e = IBDGraphEdgeByHashKey(ibd_graph, key); // edge is the edge_name
	    cur_node = IBDGraphNodeByNumber(ibd_graph, ibd0); // ibd0 is the base_node

	    cur_range_min = mr_graph_min; // check

	    /* H_ExtractHash(s, cur_node); */
	    /* printf("  @@ Adding node %s at range [0, 1)\n", s); */
		
	    // call function to add the connection
	    IBDGraph_Connect(ibd_graph, e, cur_node, cur_range_min, 1);
	    /* printf("  connecting initial: 0 1\n"); */
	    cur_range_min = 1;
	    cur_node = IBDGraphNodeByNumber(ibd_graph, ibd1);

	    if(changes > 0) // checking if there are changes to manage
	    {
		for(k=0; k<changes; k++) // if there are changes, update graph
		{
		    fscanf(fp, "%d %d", &change_pos, &ibd);
		    /* printf("  Changes %d %d\n", change_pos, ibd); */
		    
		    /* H_ExtractHash(s, cur_node); */
		    /* printf("  @@ Adding node %s at range [%d, %d)\n", s, change_pos, ibd); */

		    // call function to add the connection
		    IBDGraph_Connect(ibd_graph, e, cur_node, cur_range_min, change_pos);
		    /* printf("  connecting changes: %d %d\n", cur_range_min, change_pos); */
		    cur_range_min = change_pos;
		    cur_node = IBDGraphNodeByNumber(ibd_graph, ibd);
		}
	    }

	    // no changes in this line so just add connection
	    IBDGraph_Connect(ibd_graph, e, cur_node, cur_range_min, Mr_Plus_Infinity());
	    /* printf("  connecting: %d %d\n", cur_range_min, (int)Mr_Plus_Infinity()); */
	}
	else
	{
	    printf("  ERROR: Parsing error.");
	}
		
    }
    

    fclose(fp);
   
    return;
}

bool verifyKeyAndAdvanceStream(char* name, FILE* fp)
{
    int pos, r;

    do{
	r = fgetc(fp);
	
	if(unlikely(r == EOF))
	    return false;

    }while(isspace(r));

    if(r != name[0])
	return false;

    for(pos = 1; pos < EDGE_NAME_BUFFER_SIZE; ++pos) 
    {

	r = fgetc(fp);

	if(unlikely(r == EOF))
	    return false;
	      
	if(isspace(r))
	    return true;

	if(r != name[pos])
	    return false;
	
    }

    do{
	r = fgetc(fp);

	if(unlikely(r == EOF))
	    return false;

    }while(!isspace(r));

    return true;
}

bool getKeyFromFileStream(HashKey* key, char* name, FILE* fp)
{
    // This version will handle anything!
    Hk_CLEAR(key);

    char buffer[EDGE_NAME_BUFFER_SIZE];
    bool starting = true, first = true;

    // First get rid of blank characters
    while(true)
    {
	bool done = false, is_int = true;
	int pos;

	for(pos = 0; pos < EDGE_NAME_BUFFER_SIZE; ++pos)
	{
	    int r;

	    do{
		r = fgetc(fp);

		if(unlikely(r == EOF))
		    return false;

	    }while(unlikely(starting && isspace(r)));

	    if(isspace(r))
	    {
		buffer[pos] = '\0';
		done = true;
		if(first && name != NULL)
		    name[pos] = '\0';
		break;
	    }
	    else
	    {
		buffer[pos] = (char)r;
		starting = false;
		if(first && name != NULL)
		    name[pos] = buffer[pos];

		is_int &= !!isdigit(r);
	    }
	}

	// Catch case of exactly a 64 character string; this is next;
	if(unlikely(pos == 0))
	    return true;

	if(likely(Hk_ISZERO(key)))
	{


	    if(likely(done))
	    {
		// See if we can use the cheaper integer hash function
		if(is_int)
		{
		    long n = atol(buffer);

		    if(likely(n != 0))
		    {
			*key = Hk_FromInt(n);
			return true;
		    }
		}

		*key = Hk_FromCharBuffer(buffer, pos);
		return true;

	    } else {
		assert(pos == EDGE_NAME_BUFFER_SIZE);
		*key = Hk_FromCharBuffer(buffer, EDGE_NAME_BUFFER_SIZE);
	    }
	} else {
	    HashKey new_key = Hk_FromCharBuffer(buffer, pos);
	    Hk_InplaceCombine(key, &new_key);

	    if(done)
		return true;
	}
    }
}

// compares edge to the list of edges initially generated
int checkEdgeList(int edge, int NUM_EDGES, int edge_list[])
{
    int i = 0, j;
    for(j = 0; j < NUM_EDGES; j++)
    {
	if(edge_list[j] == edge)
	{
	    i = 1;
	    return i;
	}
    }
    return i;
}


