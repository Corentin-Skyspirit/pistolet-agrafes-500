#include "gen_graph.hpp"
#include <algorithm>
#include <iostream>

graph from_edge_list(edge_list input_list) {
	int64_t max_node = 0;
	for (int64_t i = 0; i < input_list.length; i++) {
		max_node = std::max(input_list.edges[i].v0, max_node);
		max_node = std::max(input_list.edges[i].v1, max_node);
	}
	graph g;
	g.neighbors = (int64_t*)malloc(2 * input_list.length * sizeof(int64_t));
	g.weights = (float*)malloc(2 * input_list.length * sizeof(float));
	g.slicing_idx = (int64_t*)malloc((max_node + 1) * sizeof(int64_t));
	int64_t cpt = 0;
	int64_t cpt_idx = 1;
	g.slicing_idx[0] = 0;
	for (int64_t node = 0; node <= max_node; node++) {
		for (int64_t i = 0; i < input_list.length; i++) {
			if (input_list.edges[i].v0 != input_list.edges[i].v1) {

				if (input_list.edges[i].v0 == node) {
					g.neighbors[cpt] = input_list.edges[i].v1;
					g.weights[cpt] = input_list.weights[i];
					cpt++;
				}
				if (input_list.edges[i].v1 == node) {
					g.neighbors[cpt] = input_list.edges[i].v0;
					g.weights[cpt] = input_list.weights[i];
					cpt++;
				}
			}
		}
		g.slicing_idx[cpt_idx] = cpt;
		cpt_idx++;
	}
	g.length = cpt;
	return g;
}