#include "gen_graph.hpp"
#include <algorithm>

graph graph::from_edge_list(edge_list input_list) {
	int64_t node = 0;
	int64_t max_node = 0;
	for (uint64_t i = 0; i < input_list.length; i++) {
		max_node = std::max(input_list.edges[i].v0, max_node);
		max_node = std::max(input_list.edges[i].v1, max_node);
	}
	for (uint64_t i = 0; i < input_list.length; i++) {}

	return;
}