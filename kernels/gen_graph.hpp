#ifndef GEN_GRAPH_H
#define GEN_GRAPH_H

#include "../generator/generator.hpp"
#include <cstdint>

typedef struct {
	int64_t* slicing_idx;
	int64_t* neighbors;
	float* weights;
	double time_ms;
	int64_t length;
	int64_t nb_nodes;
} graph;

graph from_edge_list(edge_list input_list);

#endif