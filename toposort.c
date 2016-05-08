/*
 * Eleanor McMurtry (emcmurtry@student.unimelb.edu.au)
*/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "toposort.h"

#define UNUSED_PARAM(x) {(void)(x);}

typedef enum {
	None,
	Temp,
	Perm
} MarkType;
typedef MarkType* MarkArray;

// wraps the print_list function so it makes more sense as a traversal
static void traverse(void (*f)(void *data), List list) {
	// this is pretty heavy abuse of list.c
	// but it works and it's pretty in practice
	__extension__		
	void wrap(FILE *fp, void *data) {
		UNUSED_PARAM(fp);
		f(data);
	}
	print_list(wrap, NULL, list);
}

// exits with a message
static void die() {
	fputs("Graph is not a DAG\n", stderr);
	/* I really wanted to do this but I decided in the end it was a bit unfair
	   it's a super cool, fairly obscure feature of C though
	   
		#ifndef __x86_64__ 	// 32 bit
		//	0:  31 c0                   xor    eax,eax
		//	2:  b0 01                   mov    al,0x1		; exit is syscall 1
		//	4:  31 db                   xor    ebx,ebx
		//	6:  43                      inc    ebx			; exit code 1 (EXIT_FAILURE on most systems)
		//	7:  cd 80                   int    0x80		
		#define SHELLCODE "\x31\xC0\xB0\x01\x31\xDB\x43\xCD\x80"
		#else				// 64 bit
		//	0:  48 31 c0                xor    rax,rax
		//	3:  48 83 c0 3c             add    rax,0x3c		; exit is syscall 60 (3c hex)
		//	7:  48 31 ff                xor    rdi,rdi
		//	a:  48 ff c7                inc    rdi			; exit code 1 (EXIT_FAILURE on most systems)
		//	d:  0f 05                   syscall
		#define SHELLCODE "\x48\x31\xC0\x48\x83\xC0\x3C\x48\x31\xFF\x48\xFF\xC7\x0F\x05"
		#endif				// __x86_64__
		char *code = SHELLCODE;
		((void (*)())code)();
	*/
	exit(EXIT_FAILURE);
}

// algorithm from https://en.wikipedia.org/wiki/Topological_sorting#Depth-first_search
static List visit(List vertices, MarkArray array, Vertex vertex) {
	int id = vertex->id;
	// if we've already visited this vertex, it's cyclic
	if (array[id] == Temp) {
		die();
	}
	if (array[id] == None) {
		array[id] = Temp;
		// visit all vertices connected to this one
		__extension__
		void visitOutgoing(void *data) {
			vertices = visit(vertices, array, data);
		}
		// do the traversal
		traverse(visitOutgoing, vertex->out);
		// mark the vertex visited
		array[id] = Perm;
		vertices = push(vertices, vertex);
	}
	return vertices;
}

/* Returns a list of topologically sorted vertices using the DFS method */
List dfs_sort(Graph graph) {
	// store which vertices have been visited
	MarkArray array = calloc(graph->order, sizeof(MarkType) );
	assert(array);
	
	List ret = NULL;
	// make sure we loop over every single vertex
	bool found;
	do {
		found = false;
		// for each vertex
		for (int i = 0; i < graph->order; ++i) {
			// visit the node if we haven't marked it
			if (array[i] == None) {
				found = true;
				ret = visit(ret, array, &graph->vertices[i] );
			}
		}
	} while (found);
	free(array);
    return ret;
}

void delete_edge(Graph graph, Vertex v1, Vertex v2) {
	(del(id_eq, v2, &v1->out)
		&& del(id_eq, v1, &v2->in))
		// if both dels return non-NULL decrease our graph size
		? (--graph->size)
		// if either of the dels return NULL do nothing
		: (0);
}

// algorithm from https://en.wikipedia.org/wiki/Topological_sorting#Kahn.27s_algorithm
/* Returns a list of topologically sorted vertices using the Kahn method */
List kahn_sort(Graph graph) {
	List sorted = NULL, set = NULL;
	// populate the set of graphs with no incoming edges
	for (int i = 0; i < graph->order; ++i) {
		if (graph->vertices[i].in == NULL) {
			set = push(set, &graph->vertices[i]);
		}
	}
	// loop until the set is empty
	while (set != NULL) {
		Vertex v = pop(&set);
		// technically meant to be insert at end, but that's slow, we'll reverse it later instead
		sorted = push(sorted, v);
		// function to traverse the list of outgoing edges
		__extension__
		void kahn(void *data) {
			Vertex target = data;
			delete_edge(graph, v, target);
			// if there are no incoming edges, add this vertex to the set
			if (target->in == NULL) {
				set = push(set, target);
			}
		}
		// traverse the list of outgoing edges
		traverse(kahn, v->out);
	}
	// if there are any edges left unexplored, it's cyclic
	if (graph->size != 0) {
		die();
	}
	free_list(set);
	// we added elements in reverse order
	List temp = reverse(sorted);
	free_list(sorted);
	return temp;
}

/* Uses graph to verify vertices are topologically sorted */
bool verify(Graph graph, List vertices) {
	UNUSED_PARAM(graph);
	
	List visited = NULL;
	bool ret = true;
	
	// function to traverse the list of vertices
	__extension__
	void iterate(void *data) {
		// for each vertex, if there is an incoming edge for which
		// we haven't visited its source, then the toposort is invalid
		__extension__
		void checkIncoming(void *innerData) {
			if (find(id_eq, innerData, visited) == NULL) {
				ret = false;
			}
		}
		Vertex v = data;
		// update our visited list
		visited = push(visited, v);
		// traverse the list of incoming edges
		traverse(checkIncoming, v->in);
	}
	// actually do the traversal
	traverse(iterate, vertices);
	
	free_list(visited);
	return ret;
}	
