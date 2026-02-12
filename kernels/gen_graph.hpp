#ifndef GEN_GRAPH_H
#define GEN_GRAPH_H

#include "../generator/generator.hpp"
#include <cstdint>

typedef struct {
	int64_t* slicing_idx;
	int64_t* neighbors;
	float* weights;
	double time_s;
	int64_t length;
} graph;

graph from_edge_list(edge_list input_list);

#endif