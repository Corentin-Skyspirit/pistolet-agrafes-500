/// @file gen_graph.hpp
/// @brief Functions to generate a graph from an edge list, and utilities to manipulate the graph.

#ifndef GEN_GRAPH_H
#define GEN_GRAPH_H

#include "../generator/generator.hpp"
#include <cstdint>

/// @brief Graph structure in compressed sparse row format.
typedef struct graph {
	int64_t* slicing_idx; ///< In the flattened arrays, where the neighbors of each node start and end.
						  ///< The neighbors (and weights) of node i are in the range [slicing_idx[i], slicing_idx[i+1]).
	int64_t* neighbors;	  ///< Flattened array of all neighbors of all nodes.
	float* weights;		  ///< Flattened array of all weights of all edges, parallel to `neighbors`.
	double time_ms;		  ///< Time taken to build the graph, in milliseconds.
	int64_t length;		  ///< Length of the `neighbors` and `weights` arrays
	int64_t nb_nodes;	  ///< Number of nodes in the graph. The nodes are assumed to be indexed from 0 to nb_nodes-1.
} graph;

/// @brief Free the memory allocated for a graph.
/// @param g The graph to destroy.
void graph_destroy(graph& g);

/// @brief Build a graph from an edge list.
/// @param input_list The list of edges to build the graph from.
graph from_edge_list(edge_list& input_list);

/// @brief Try all different implementations of from_edge_list we did, and print their execution times.
/// Used for benchmarking and comparing them.
void from_edge_list_try_all(edge_list input_list);

///////////////////////////////////////////////////
//////////////////// Utilities ////////////////////
///////////////////////////////////////////////////

/// @brief Iterate over the neighbors of a node and apply a function on them.
/// @param g The graph on which to iterate.
/// @param node The node whose neighbors to iterate on.
/// @param f The function to apply on each neighbor. It takes the neighbor's id and the weight of the edge as parameters.
template <typename F> static inline void for_each_neighbor(const graph& g, int64_t node, F f) {
	int64_t start = g.slicing_idx[node];
	int64_t end = g.slicing_idx[node + 1];
	for (int64_t i = start; i < end; i++) {
		f(g.neighbors[i], g.weights[i]);
	}
}

/// @brief Parallel version of for_each_neighbor. Iterate over the neighbors of a node and apply a function on them, in parallel.
/// @param g The graph on which to iterate.
/// @param node The node whose neighbors to iterate on.
/// @param f The function to apply on each neighbor. It takes the neighbor's id and the weight of the edge as parameters.
template <typename F> static inline void parallel_for_each_neighbor(const graph& g, int64_t node, F f) {
	int64_t start = g.slicing_idx[node];
	int64_t end = g.slicing_idx[node + 1];
#pragma omp parallel for
	for (int64_t i = start; i < end; i++) {
		f(g.neighbors[i], g.weights[i]);
	}
}

/// @brief Get the degree of a node in the graph.
/// @param g The graph on which to get the degree.
/// @param node The node whose degree to get.
static uint64_t degree_of_node(const graph& g, int64_t node) {
	return g.slicing_idx[node + 1] - g.slicing_idx[node];
}

/// @brief Compare two graphs for structural equality. Returns true if identical.
bool compare_graphs(const graph& a, const graph& b, double weight_eps = 1e-6);

#endif