#ifndef GEN_GRAPH_H
#define GEN_GRAPH_H

#include "../generator/generator.hpp"
#include <cstdint>

typedef struct graph graph;
struct graph {
	uint64_t* slicing_idx;
	uint64_t* neighbors;
	float* weight;
	double time_s;

	graph from_edge_list(edge_list input_list);
};

#endif