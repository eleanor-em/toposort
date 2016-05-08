/*
 * COMP20007 Design of Algorithms
 * Semester 1 2016
 *
 * Clement Poh (cpoh@unimelb.edu.au)
 *
 * This module provides all the IO functionality related to graphs.
 *
 * Eleanor McMurtry (emcmurtry@student.unimelb.edu.au)
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "graphio.h"

#define MAX_LINE_LEN 256

/* Loads the graph from input */
Graph load_graph(char *input) {
	FILE *fp = fopen(input, "r");
	int order;
	
	// read in the number of vertices
	char line[MAX_LINE_LEN];
	assert( fgets(line, MAX_LINE_LEN - 1, fp) );
	sscanf(line, "%d", &order);
	
	// read in each vertex
	Graph graph = new_graph(order);
	int i;
	for (i = 0; i < order; ++i) {
		assert( fgets(line, MAX_LINE_LEN - 1, fp) );
		
		Vertex v = &graph->vertices[i];
		v->id = i;
		// null out the lists
		v->in = NULL;
		v->out = NULL;
		
		int len = strlen(line);
		// there's a superfluous \n that we don't want in the line
		// line[strlen(line)] is the null terminator, so the character before
		// (line[strlen(line) - 1] is the newline
		line[len - 1] = 0;
		v->label = calloc(len, sizeof(char) );
		assert(v->label);
		// now we're safe to copy it over
		strcpy(v->label, line);
	}
	
	// read in the edges
	while ( fgets(line, MAX_LINE_LEN - 1, fp) ) {
		int home, target;
		sscanf(line, "%d %d", &home, &target);
		add_edge(graph, home, target);
	}
	
	fclose(fp);
	
    return graph;
}

// prints the label of a vertex followed by a space
void print_vertex_label(FILE *fp, void *vertex) {
	fprintf(fp, "%s ", ((Vertex)vertex)->label);
}

// prints a single vertex in the .dot format
void print_vertex(FILE *fp, Vertex vertex) {
	fprintf(fp, "\t%s -> { ", vertex->label);
	// the edges are inserted in reverse order
	List reversed = reverse(vertex->out);
	print_list(print_vertex_label, fp,  reversed);
	free_list(reversed);
	fputs("}\n", fp);
}

/* Prints the graph */
void print_graph(char *output, Graph graph) {
	FILE *fp = fopen(output, "w");
	fputs("digraph {\n", fp);
	for (int i = 0; i < graph->order; ++i) {
		print_vertex(fp, &graph->vertices[i]);
	}
	fputs("}\n", fp);
	fclose(fp);
}

/* Prints the id of a vertex then a newline */
void print_vertex_id(FILE *file, void *vertex) {
    if (vertex)
        fprintf(file, "%d\n", ((Vertex)vertex)->id);
}

/* Returns a sequence of vertices read from file */
List load_vertex_sequence(FILE *file, Graph graph) {
    const char *err_duplicate = "Error: duplicate vertex %d %s\n";
    const char *err_order = "Error: graph order %d, loaded %d vertices\n";
    List list = NULL;
    int id;

    while(fscanf(file, "%d\n", &id) == 1) {
        assert(id >= 0);
        assert(id < graph->order);

        if (!insert_if(id_eq, graph->vertices + id, &list)) {
            fprintf(stderr, err_duplicate, id, graph->vertices[id].label);
            exit(EXIT_FAILURE);
        }
    }

    if (len(list) != graph->order) {
        fprintf(stderr, err_order, graph->order, len(list));
        exit(EXIT_FAILURE);
    }

    return list;
}
