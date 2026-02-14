#ifndef BREADTH_SEARCH_H
#define BREADTH_SEARCH_H

#include "gen_graph.hpp"
#include <cstdint>

typedef struct {
	int64_t* parent_array;
	double teps;
	double time_ms;
} bfs_result;

bfs_result bfs(graph& g, int64_t source);

#endif