#include "generator/generator.hpp"
#include "kernels/breadth_search.hpp"
#include "kernels/gen_graph.hpp"
#include "kernels/shortest_path.hpp"
#include <cstdio>
#include <iostream>

int main(int argc, char const* argv[]) {
	printf("Hello world\n");
	edge_list list = generate_graph(12, 5);
	// print_edge_list(&list);
	graph g = from_edge_list(list);
	std::cout << "nb_nodes = " << g.nb_nodes << ", nb_neighbors = " << g.length << ", time = " << g.time_ms << "ms" << std::endl;

	if (argv[1]) {
		for (int64_t node = 0; node < g.nb_nodes; node++) {
			std::cout << "Neighbors of " << node << " :" << std::endl;

			for (int64_t i = g.slicing_idx[node]; i < g.slicing_idx[node + 1]; i++) {

				std::cout << "  (" << g.neighbors[i] << ", " << g.weights[i] << ")" << std::endl;
			}
		}
	}
	edge_list_destroy(list);
	return 0;
}