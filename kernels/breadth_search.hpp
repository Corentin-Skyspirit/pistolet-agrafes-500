#ifndef BREADTH_SEARCH_H
#define BREADTH_SEARCH_H

#include "gen_graph.hpp"
#include <cstdint>

typedef struct {
	uint64_t* parent_array;
	double teps;
} bfs_result;

bfs_result bfs(graph& g);

#endif