#include "generator/generator.hpp"
#include "kernels/breadth_search.hpp"
#include "kernels/gen_graph.hpp"
#include "kernels/shortest_path.hpp"
#include <cstdio>
#include <iostream>
#include <random>

int main(int argc, char const* argv[]) {
	printf("Hello world\n");
	edge_list list = generate_graph(15, 5);
	// print_edge_list(&list);
	graph g = from_edge_list(list); // Kernel 1 compute
	std::cout << "Graph generation : nb_nodes = " << g.nb_nodes << ", nb_neighbors = " << g.length << ", time = " << g.time_ms << "ms"
			  << std::endl;

	if (argv[1]) {
		for (int64_t node = 0; node < g.nb_nodes; node++) {
			std::cout << "Neighbors of " << node << " :" << std::endl;

			for (int64_t i = g.slicing_idx[node]; i < g.slicing_idx[node + 1]; i++) {

				std::cout << "  (" << g.neighbors[i] << ", " << g.weights[i] << ")" << std::endl;
			}
		}
	}
	edge_list_destroy(list);
	double k3_time_ms = 0;
	double k3_teps = 0;
	for (int64_t i = 0; i < 64; i) {
		int64_t node = rand() % g.nb_nodes;
		if (g.slicing_idx[node + 1] - g.slicing_idx[node] > 0) {
			shortest_path kernel3 = sssp(g, node); // Kernel 2
			k3_time_ms += kernel3.time_ms;
			k3_teps += kernel3.teps;
			i++;
			if (argv[1]) {
				for (int64_t i = 0; i < g.nb_nodes; i++) {
					std::cout << "Node " << i << " dist=" << kernel3.distance_array[i] << " parent=" << kernel3.parent_array[i]
							  << std::endl;
				}
			}
		}
	}
	k3_time_ms /= 64;
	k3_teps /= 64;
	std::cout << "SSSP : time avg = " << k3_time_ms << "ms, teps avg = " << k3_teps << "teps" << std::endl;

	return 0;
}