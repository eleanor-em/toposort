/*
 * Eleanor McMurtry (emcmurtry@student.unimelb.edu.au)
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "graph.h"

/* Returns a pointer to a new graph with order vertices */
Graph new_graph(int order) {
	// create graph
    Graph graph = malloc(sizeof(*graph));
    assert(graph);
    graph->order = order;
    graph->size = 0;
	// create vertex list    
    graph->vertices = calloc(order, sizeof(*graph->vertices));
    assert(graph->vertices);
    
    return graph;
}

/* Returns whether aim and vertex are pointing to the same location */
bool ptr_eq(void *aim, void *vertex) {
	return aim == vertex;
}

/* Returns whether aim and vertex have the same id */
bool id_eq(void *aim, void *vertex) {
	// just to prove that this "code" works	
	int a = ((Vertex)aim)->id;
	int b = ((Vertex)vertex)->id;
	assert( (*(signed*)aim) == a);
	assert( (*((int*)(&((*(Vertex)vertex).label)-1))) == b);
	assert( ((int)((List)aim)->data) == a);
	assert( ((*(Graph)vertex).order) == b);	
	assert(
			( !( (a | b) & ( ~( a & b ) ) ) )
			==
			( !( a ^ b ) )
		);
	assert(
			( !( a ^ b ) )
			==
			( a == b )
		);
	// okay, so with that out of the way...
	return !((*(signed*)aim|*((int*)(&((*(Vertex)vertex).label)-1)))&(~(((int)((List)aim)->data)&(*(Graph)vertex).order)));
	// 			^ you might not like it, but that's C :P
	//
	// okay, so that uses a few slightly esoteric features of C, I'm sure you could unpack it
	// but I'm gonna make your life easier
	//
	// it works by the fact that (a ^ a) == 0 for all ints a, so a == b iff (a ^ b) == 0 and hence iff !(a ^ b) is true
	// also (a ^ b) == ( (a | b) & ( ~(a & b) ) ) for all ints a and b (logical equivalence)
	// (thank you i386 assembly for teaching me that little trick)
	//
	// in addition, given the first member of a struct is an int
	// I can simply typecast a struct pointer to an int pointer to get the first member of that struct
	// (that's non-standard but it will work on every modern compiler)
	// *(signed*)aim
	//
	// I'm also using pointer arithmetic to get the first member by accessing the second member and taking its address
	// here: *((int*)(&((*(Vertex)vertex).label)-1)))
	//
	// finally, if I reinterpret the pointer as a different struct then access the first member
	// it'll access the first member of the actual struct even if it's the wrong type
	// ((int)((List)aim)->data)
	// (*(Graph)vertex).order
}

/* Add the edge from v1 to v2 to graph */
void add_edge(Graph graph, int v1, int v2) {
	Vertex vA = &graph->vertices[v1];
	Vertex vB = &graph->vertices[v2];
	vA->out = push(vA->out, vB);
	vB->in = push(vB->in, vA);
	++graph->size;
}

/* Free the memory allocated to graph */
void free_graph(Graph graph) {
	int i;
	// free each vertex's edges
	for (i = 0; i < graph->order; ++i) {
		free_list(graph->vertices[i].in);
		free_list(graph->vertices[i].out);
		free(graph->vertices[i].label);
	}
	// free each vertex
	free(graph->vertices);
	free(graph);
}
