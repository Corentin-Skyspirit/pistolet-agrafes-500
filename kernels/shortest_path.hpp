/// @file shortest_path.hpp
/// @brief Different shortest path algorithms implementations headers

#ifndef SHORTEST_PATH_H
#define SHORTEST_PATH_H

#include "gen_graph.hpp"
#include <cstdint>

/// @brief Result of a shortest path execution.
typedef struct {
	int64_t* parent_array; ///< parent_array[i] is the parent of node i in the shortest path tree, or -1 if unreachable.
						   ///< The root's parent is itself.
	float* distance_array; ///< distance_array[i] is the shortest distance from the root to node i, or infinity if unreachable.
	double teps;		   ///< UNUSED
	double time_ms;		   ///< Time taken to compute the shortest paths, in milliseconds.
} shortest_path;

/// @brief Destroy the memory allocated for a shortest_path result.
void shortest_path_destroy(shortest_path& sp);

/// @brief Shortest path with Dijkstra's algorithm (sequential).
/// @param g Input graph
/// @param root Starting node for SSSP
/// @return List of shortest distances from root to each node, and parent nodes for path reconstruction.
shortest_path sssp_dj(graph& g, int64_t root);

/// @brief Shortest path with Bellman-Ford's algorithm (sequential).
/// @param g Input graph
/// @param root Starting node for SSSP
/// @return List of shortest distances from root to each node, and parent nodes for path reconstruction.
shortest_path sssp_bf(graph& g, int64_t root);

/// @brief Shortest path with Bellman-Ford's algorithm (parallel).
/// @param g Input graph
/// @param root Starting node for SSSP
/// @return List of shortest distances from root to each node, and parent nodes for path reconstruction.
shortest_path sssp_parallel(graph& g, int64_t root);

#endif