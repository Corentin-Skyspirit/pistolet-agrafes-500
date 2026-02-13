#ifndef SHORTEST_PATH_H
#define SHORTEST_PATH_H

#include "gen_graph.hpp"
#include <cstdint>

typedef struct {
	int64_t* parent_array;
	float* distance_array;
	double teps;
	double time_ms;

} shortest_path;

shortest_path sssp(graph source_graph, int64_t root);

#endif