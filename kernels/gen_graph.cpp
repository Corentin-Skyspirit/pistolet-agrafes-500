#include "gen_graph.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <vector>

void print_slicing_idx(const graph& g) {
	std::cout << "slicing_idx: ";
	for (int64_t i = 0; i <= g.nb_nodes; ++i) {
		std::cout << g.slicing_idx[i] << " ";
	}
	std::cout << std::endl;
}

void print_neighbors(const graph& g) {
	for (int64_t node = 0; node < g.nb_nodes; ++node) {
		std::cout << "Node " << node << ": ";
		for (int64_t i = g.slicing_idx[node]; i < g.slicing_idx[node + 1]; ++i) {
			std::cout << "(" << g.neighbors[i] << ", " << g.weights[i] << ") ";
		}
		std::cout << std::endl;
	}
}

void print_neighbors_array(const graph& g) {
	std::cout << "neighbors array: ";
	for (int64_t i = 0; i < g.length; ++i) {
		std::cout << g.neighbors[i] << " ";
	}
	std::cout << std::endl;
}

static void print_neighbors_of_node(const graph& graph_obj, int64_t node) {
	std::vector<int64_t> neighbors;
	for (int64_t i = graph_obj.slicing_idx[node]; i < graph_obj.slicing_idx[node + 1]; ++i) {
		neighbors.push_back(graph_obj.neighbors[i]);
	}
	std::sort(neighbors.begin(), neighbors.end());
	std::cout << "Neighbors of node " << node << ": ";
	for (auto n : neighbors) {
		std::cout << n << " ";
	}
	std::cout << '\n';
}

graph _from_edge_list(edge_list input_list) {
	auto start = std::chrono::high_resolution_clock::now();
	int64_t max_node = 0;
	for (int64_t i = 0; i < input_list.length; i++) {
		max_node = std::max(input_list.edges[i].v0, max_node);
		max_node = std::max(input_list.edges[i].v1, max_node);
	}
	graph g;
	g.neighbors = (int64_t*)malloc(2 * input_list.length * sizeof(int64_t));
	g.weights = (float*)malloc(2 * input_list.length * sizeof(float));
	g.slicing_idx = (int64_t*)malloc((max_node + 2) * sizeof(int64_t));
	int64_t cpt = 0;
	int64_t cpt_idx = 1;
	g.slicing_idx[0] = cpt;
	for (int64_t node = 0; node <= max_node; node++) {
		for (int64_t i = 0; i < input_list.length; i++) {
			if (input_list.edges[i].v0 != input_list.edges[i].v1) {

				int64_t neighbor = -1;

				if (input_list.edges[i].v0 == node) {
					neighbor = input_list.edges[i].v1;
				}
				else if (input_list.edges[i].v1 == node) {
					neighbor = input_list.edges[i].v0;
				}

				if (neighbor != -1) {
					bool found = false;
					for (int64_t j = g.slicing_idx[cpt_idx - 1]; j < cpt; j++) {
						if (g.neighbors[j] == neighbor) {
							g.weights[j] = input_list.weights[i];
							found = true;
							break;
						}
					}

					if (!found) {
						g.neighbors[cpt] = neighbor;
						g.weights[cpt] = input_list.weights[i];
						cpt++;
					}
				}
			}
		}
		g.slicing_idx[cpt_idx] = cpt;
		cpt_idx++;
	}
	g.length = cpt;
	g.nb_nodes = max_node + 1;
	g.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
	return g;
}

#include <cmath>

typedef struct {
	int64_t u, v;
	float weight;
} edge;

bool are_same_edge(const edge& e1, const edge& e2) {
	return e1.u == e2.u && e1.v == e2.v;
}

bool is_loop(const edge& e) {
	return e.u == e.v;
}

graph from_edge_list(edge_list input_list) {
	auto start = std::chrono::high_resolution_clock::now();
	graph g;

	edge* edges = (edge*)malloc(sizeof(edge) * 2 * input_list.length);

	// Duplicate the edges
	for (size_t i = 0; i < input_list.length; i++) {
		edges[2 * i] = (edge){
			.u = input_list.edges[i].v1,
			.v = input_list.edges[i].v0,
			.weight = input_list.weights[i],
		};
		edges[(2 * i) + 1] = (edge){
			.u = input_list.edges[i].v0,
			.v = input_list.edges[i].v1,
			.weight = input_list.weights[i],
		};
	}

	// Sort them lexicographically
	std::sort(edges, edges + (input_list.length * 2), [&](const edge& e1, const edge& e2) {
		if (e1.u != e2.u) {
			return e1.u < e2.u;
		}
		return e1.v < e2.v;
	});

	int64_t nb_nodes = edges[(input_list.length * 2) - 1].u + 1;
	g.slicing_idx = (int64_t*)malloc(nb_nodes * sizeof(int64_t));
	g.neighbors = (int64_t*)malloc(2 * input_list.length * sizeof(int64_t));
	g.weights = (float*)malloc(2 * input_list.length * sizeof(float));

	// If there are loops at the beginning
	int nb_skipped = 0;
	while (is_loop(edges[0])) {
		edges++;
		nb_skipped++;
	}

	int64_t nb_nodes_so_far = 0;
	auto first_node = edges[0].u;
	g.slicing_idx[first_node] = 0;
	g.neighbors[0] = edges[0].v;
	g.weights[0] = edges[0].weight;
	int64_t nb_neighbors_so_far = 1;
	for (int64_t i = 1; i < (2 * input_list.length) - nb_skipped; i++) {
		if (are_same_edge(edges[i], edges[i - 1])) {
			continue;
		}

		if (is_loop(edges[i])) {
			continue;
		}

		if (edges[i].u != edges[i - 1].u) {
			// Fill slicing_idx for intermediary nodes with no neighbors
			for (int64_t missing = edges[i - 1].u + 1; missing < edges[i].u; ++missing) {
				nb_nodes_so_far++;
				g.slicing_idx[nb_nodes_so_far] = nb_neighbors_so_far;
			}
			// Different node -> mark a slice
			nb_nodes_so_far++;
			g.slicing_idx[nb_nodes_so_far] = nb_neighbors_so_far;
		}

		// Push v as a neighbor of u
		g.neighbors[nb_neighbors_so_far] = edges[i].v;
		g.weights[nb_neighbors_so_far] = edges[i].weight;
		nb_neighbors_so_far++;
	}
	// Close off the slices
	nb_nodes_so_far++;
	g.slicing_idx[nb_nodes_so_far] = nb_neighbors_so_far;
	g.length = nb_neighbors_so_far;
	g.nb_nodes = nb_nodes;

	g.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();

	// Verification avec vieux algorithme
	// graph g2 = _from_edge_list(input_list);
	// print_slicing_idx(g);
	// print_slicing_idx(g2);
	// for (int64_t node = 0; node < g.nb_nodes; ++node) {
	// 	print_neighbors_of_node(g, node);
	// 	print_neighbors_of_node(g2, node);
	// }

	return g;
}