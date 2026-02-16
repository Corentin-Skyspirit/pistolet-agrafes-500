#include "generator/generator.hpp"
#include "kernels/breadth_search.hpp"
#include "kernels/gen_graph.hpp"
#include "kernels/shortest_path.hpp"
#include <cstdio>
#include <iostream>
#include <random>

int main(int argc, char const* argv[]) {
	printf("Hello world\n");
	edge_list list = generate_graph(12, 16);
	// edge_list list = generate_graph(5, 2);
	// print_edge_list(&list);
	graph g = from_edge_list(list); // Kernel 1 compute
	std::cout << "Graph generation : nb_nodes = " << g.nb_nodes << ", nb_neighbors = " << g.length << ", time = " << g.time_ms << "ms"
			  << std::endl;

	if (argv[1] != nullptr) {
		for (int64_t node = 0; node < g.nb_nodes; node++) {
			std::cout << "Neighbors of " << node << " :" << std::endl;

			for (int64_t i = g.slicing_idx[node]; i < g.slicing_idx[node + 1]; i++) {

				std::cout << "  (" << g.neighbors[i] << ", " << g.weights[i] << ")" << std::endl;
			}
		}
	}

	const int64_t NB_NODES_TO_TRY = 64;
	edge_list_destroy(list);

	double k2_time_ms = 0;
	double k2_teps = 0;
	for (int64_t i = 0; i < NB_NODES_TO_TRY;) {
		int64_t node = rand() % g.nb_nodes;
		if (degree_of_node(g, node) <= 0) {
			continue;
		}
		i++;

		// Kernel 2
		bfs_result kernel2 = bfs(g, node);
		k2_time_ms += kernel2.time_ms;
		k2_teps += kernel2.teps;

		if (argv[1] != nullptr) {
			for (int64_t i = 0; i < g.nb_nodes; i++) {
				std::cout << "Node " << i << " parent=" << kernel2.parent_array[i] << std::endl;
			}
		}
	}
	k2_time_ms /= NB_NODES_TO_TRY;
	k2_teps /= NB_NODES_TO_TRY;
	std::cout << "BFS : time avg = " << k2_time_ms << "ms, teps avg = " << k2_teps << "teps" << std::endl;

	double k3_time_ms = 0;
	double k3_teps = 0;
	double k3_para_time_ms = 0;
	double k3_para_teps = 0;
	for (int64_t i = 0; i < NB_NODES_TO_TRY;) {
		int64_t node = rand() % g.nb_nodes;
		if (degree_of_node(g, node) <= 0) {
			continue;
		}
		i++;

		// Kernel 3
		shortest_path kernel3 = sssp(g, node);
		k3_time_ms += kernel3.time_ms;
		k3_teps += kernel3.teps;
		shortest_path_destroy(kernel3);

		shortest_path kernel3_para = sssp_parallel(g, node);
		k3_para_time_ms += kernel3_para.time_ms;
		k3_para_teps += kernel3_para.teps;
		shortest_path_destroy(kernel3_para);

		if (argv[1] != nullptr) {
			for (int64_t i = 0; i < g.nb_nodes; i++) {
				std::cout << "Node " << i << " dist=" << kernel3.distance_array[i] << " parent=" << kernel3.parent_array[i] << std::endl;
			}
		}
	}
	k3_time_ms /= NB_NODES_TO_TRY;
	k3_teps /= NB_NODES_TO_TRY;
	k3_para_time_ms /= NB_NODES_TO_TRY;
	k3_para_teps /= NB_NODES_TO_TRY;
	std::cout << "SSSP : time avg = " << k3_time_ms << "ms, teps avg = " << k3_teps << "teps" << std::endl;
	std::cout << "Parallel SSSP : time avg = " << k3_para_time_ms << "ms, teps avg = " << k3_para_teps << "teps" << std::endl;

	return 0;
}