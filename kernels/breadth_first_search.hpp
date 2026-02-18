/// @file breadth_first_search.hpp
/// @brief Different BFS algorithms implementations headers

#ifndef BREADTH_FIRST_SEARCH_H
#define BREADTH_FIRST_SEARCH_H

#include "gen_graph.hpp"
#include <cstdint>

/// @brief Result of a BFS execution, containing the parent array, the time taken in milliseconds.
/// `teps` is not calculated.
typedef struct bfs_result {
	int64_t* parent_array; ///< parent_array[i] is the parent of node i in the BFS tree, or -1 if unreachable.
						   ///< The source's parent is itself.
	double teps;		   ///< UNUSED
	double time_ms;		   ///< Time taken to compute the BFS, in milliseconds.

	/// @brief Free the memory allocated for the parent array in the BFS result.
	~bfs_result() {
		free(parent_array);
	}
} bfs_result;

//////////////////////////////////////////////////////////////////////////////////
//////// ALL BFS IMPLEMENTATIONS, ON GRAPH `g` AND SOURCE NODE `source` //////////
//////////////////////////////////////////////////////////////////////////////////

/// @brief "Formal" BFS implementation using a queue and a visited array.
bfs_result bfs_formal(graph& g, int64_t source);

/// @brief BFS implementation using unordered_set as frontier representation, and only top-down steps.
bfs_result bfs_full_top_down(graph& g, int64_t source);

/// @brief BFS implementation using unordered_set as frontier representation, and only bottom-up steps.
bfs_result bfs_full_bottom_up(graph& g, int64_t source);

/// @brief BFS implementation using bitset as frontier representation, and only top-down steps.
bfs_result bfs_full_top_down_bitset(graph& g, int64_t source);

/// @brief BFS implementation using bitset as frontier representation, and only bottom-up steps.
bfs_result bfs_full_bottom_up_bitset(graph& g, int64_t source);

/// @brief BFS implementation using atomic_bitset as frontier representation, and only top-down steps, and parallel.
bfs_result bfs_full_top_down_parallel_bitset(graph& g, int64_t source);

/// @brief BFS implementation using atomic_bitset as frontier representation, and only bottom-up steps, and parallel.
bfs_result bfs_full_bottom_up_parallel_bitset(graph& g, int64_t source);

/// @brief Hybrid direction-optimizing BFS using an unordered_set for top-down steps and a bitset for bottom-up steps, like advised in the
/// paper. Switch heuristic taken from it too.
bfs_result bfs_hybrid_paper(graph& g, int64_t source);

/// @brief Hybrid direction-optimizing BFS using a unique atomic_bitset for both top-down and bottom-up steps.
/// No more switching of data structures needed.
bfs_result bfs_hybrid(graph& g, int64_t source);

/// @brief Run all BFS implementations on the given graph and source node, verify their correctness, and print their execution times.
bfs_result all_bfs(graph& g, int64_t source);

#endif // BREADTH_FIRST_SEARCH_H