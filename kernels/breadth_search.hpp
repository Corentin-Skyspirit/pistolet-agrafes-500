#ifndef BREADTH_SEARCH_H
#define BREADTH_SEARCH_H

#include "gen_graph.hpp"
#include <cstdint>

typedef struct bfs_result {
	int64_t* parent_array;
	double teps;
	double time_ms;

	~bfs_result() {
		free(parent_array);
	}
} bfs_result;

bfs_result bfs(graph& g, int64_t source);
bfs_result bfs_formal(graph& g, int64_t source);
bfs_result bfs_full_top_down(graph& g, int64_t source);
bfs_result bfs_full_bottom_up(graph& g, int64_t source);
bfs_result bfs_full_top_down_bitset(graph& g, int64_t source);
bfs_result bfs_full_bottom_up_bitset(graph& g, int64_t source);
bfs_result bfs_full_top_down_parallel_bitset(graph& g, int64_t source);
bfs_result bfs_full_bottom_up_parallel_bitset(graph& g, int64_t source);
bfs_result bfs_hybrid(graph& g, int64_t source);

#endif