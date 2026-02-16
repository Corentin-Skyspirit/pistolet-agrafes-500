#include "generator/generator.hpp"
#include "kernels/breadth_search.hpp"
#include "kernels/gen_graph.hpp"
#include "kernels/shortest_path.hpp"
#include <cstdio>
#include <iostream>
#include <omp.h>
#include <random>

int main(int argc, char const* argv[]) {
	// edge_list list = generate_graph(5, 2);
	// print_edge_list(&list);

	std::cout << "OpenMP: num_threads = " << omp_get_max_threads() << std::endl;

	// Compare different implementations of from_edge_list
	{
		edge_list list = generate_graph(5, 16);
		from_edge_list_try_all(list);
	}

	printf("\nFINISHED COMPARING FROM EDGE LIST IMPLEMS\n\n");

	// Compare BFS using a bigger one
	edge_list list = generate_graph(10, 16);
	graph g = from_edge_list(list); // Kernel 1 compute
	std::cout << "Graph generation : nb_nodes = " << g.nb_nodes << ", nb_neighbors = " << g.length << ", time = " << g.time_ms << "ms"
			  << '\n';

	printf("\nCOMPARING BFS IMPLEMS\n\n");

	// if (argv[1] != nullptr) {
	// 	for (int64_t node = 0; node < g.nb_nodes; node++) {
	// 		std::cout << "Neighbors of " << node << " :" << std::endl;

	// 		for (int64_t i = g.slicing_idx[node]; i < g.slicing_idx[node + 1]; i++) {

	// 			std::cout << "  (" << g.neighbors[i] << ", " << g.weights[i] << ")" << std::endl;
	// 		}
	// 	}
	// }

	const int64_t NB_NODES_TO_TRY = 64;
	edge_list_destroy(list);

	{
		double k2_time_ms = 0;
		double k2_teps = 0;
		for (int64_t i = 0; i < NB_NODES_TO_TRY;) {
			int64_t node = rand() % g.nb_nodes;
			if (degree_of_node(g, node) <= 0) {
				continue;
			}
			i++;

			// Kernel
			bfs_result kernel2 = bfs_formal(g, node);
			k2_time_ms += kernel2.time_ms;
			k2_teps += kernel2.teps;
		}
		k2_time_ms /= NB_NODES_TO_TRY;
		k2_teps /= NB_NODES_TO_TRY;
		printf("BFS Formal - Avg. time: %fms\n", k2_time_ms);
	}
	{
		double k2_time_ms = 0;
		double k2_teps = 0;
		for (int64_t i = 0; i < NB_NODES_TO_TRY;) {
			int64_t node = rand() % g.nb_nodes;
			if (degree_of_node(g, node) <= 0) {
				continue;
			}
			i++;

			bfs_result kernel2 = bfs_full_top_down(g, node);
			k2_time_ms += kernel2.time_ms;
			k2_teps += kernel2.teps;
		}
		k2_time_ms /= NB_NODES_TO_TRY;
		k2_teps /= NB_NODES_TO_TRY;
		printf("BFS Full Top-Down - Avg. time: %fms\n", k2_time_ms);
	}
	{
		double k2_time_ms = 0;
		double k2_teps = 0;
		for (int64_t i = 0; i < NB_NODES_TO_TRY;) {
			int64_t node = rand() % g.nb_nodes;
			if (degree_of_node(g, node) <= 0) {
				continue;
			}
			i++;

			bfs_result kernel2 = bfs_full_bottom_up(g, node);
			k2_time_ms += kernel2.time_ms;
			k2_teps += kernel2.teps;
		}
		k2_time_ms /= NB_NODES_TO_TRY;
		k2_teps /= NB_NODES_TO_TRY;
		printf("BFS Full Bottom-Up - Avg. time: %fms\n", k2_time_ms);
	}
	{
		double k2_time_ms = 0;
		double k2_teps = 0;
		for (int64_t i = 0; i < NB_NODES_TO_TRY;) {
			int64_t node = rand() % g.nb_nodes;
			if (degree_of_node(g, node) <= 0) {
				continue;
			}
			i++;

			bfs_result kernel2 = bfs_full_top_down_bitset(g, node);
			k2_time_ms += kernel2.time_ms;
			k2_teps += kernel2.teps;
		}
		k2_time_ms /= NB_NODES_TO_TRY;
		k2_teps /= NB_NODES_TO_TRY;
		printf("BFS Full Top-Down (Bitset) - Avg. time: %fms\n", k2_time_ms);
	}
	{
		double k2_time_ms = 0;
		double k2_teps = 0;
		for (int64_t i = 0; i < NB_NODES_TO_TRY;) {
			int64_t node = rand() % g.nb_nodes;
			if (degree_of_node(g, node) <= 0) {
				continue;
			}
			i++;

			bfs_result kernel2 = bfs_full_bottom_up_bitset(g, node);
			k2_time_ms += kernel2.time_ms;
			k2_teps += kernel2.teps;
		}
		k2_time_ms /= NB_NODES_TO_TRY;
		k2_teps /= NB_NODES_TO_TRY;
		printf("BFS Full Bottom-Up (Bitset) - Avg. time: %fms\n", k2_time_ms);
	}
	{
		double k2_time_ms = 0;
		double k2_teps = 0;
		for (int64_t i = 0; i < NB_NODES_TO_TRY;) {
			int64_t node = rand() % g.nb_nodes;
			if (degree_of_node(g, node) <= 0) {
				continue;
			}
			i++;

			bfs_result kernel2 = bfs_full_top_down_parallel_bitset(g, node);
			k2_time_ms += kernel2.time_ms;
			k2_teps += kernel2.teps;
		}
		k2_time_ms /= NB_NODES_TO_TRY;
		k2_teps /= NB_NODES_TO_TRY;
		printf("BFS Full Top-Down (Parallel Bitset) - Avg. time: %fms\n", k2_time_ms);
	}
	{
		double k2_time_ms = 0;
		double k2_teps = 0;
		for (int64_t i = 0; i < NB_NODES_TO_TRY;) {
			int64_t node = rand() % g.nb_nodes;
			if (degree_of_node(g, node) <= 0) {
				continue;
			}
			i++;

			bfs_result kernel2 = bfs_full_bottom_up_parallel_bitset(g, node);
			k2_time_ms += kernel2.time_ms;
			k2_teps += kernel2.teps;
		}
		k2_time_ms /= NB_NODES_TO_TRY;
		k2_teps /= NB_NODES_TO_TRY;
		printf("BFS Full Bottom-Up (Parallel Bitset) - Avg. time: %fms\n", k2_time_ms);
	}
	{
		double k2_time_ms = 0;
		double k2_teps = 0;
		for (int64_t i = 0; i < NB_NODES_TO_TRY;) {
			int64_t node = rand() % g.nb_nodes;
			if (degree_of_node(g, node) <= 0) {
				continue;
			}
			i++;

			bfs_result kernel2 = bfs_hybrid(g, node);
			k2_time_ms += kernel2.time_ms;
			k2_teps += kernel2.teps;
		}
		k2_time_ms /= NB_NODES_TO_TRY;
		k2_teps /= NB_NODES_TO_TRY;
		printf("BFS Hybrid - Avg. time: %fms\n", k2_time_ms);
	}

	// double k3_time_ms = 0;
	// double k3_teps = 0;
	// double k3_para_time_ms = 0;
	// double k3_para_teps = 0;
	// for (int64_t i = 0; i < NB_NODES_TO_TRY;) {
	// 	int64_t node = rand() % g.nb_nodes;
	// 	if (degree_of_node(g, node) <= 0) {
	// 		continue;
	// 	}
	// 	i++;

	// 	// Kernel 3
	// 	shortest_path kernel3 = sssp(g, node);
	// 	k3_time_ms += kernel3.time_ms;
	// 	k3_teps += kernel3.teps;
	// 	shortest_path_destroy(kernel3);

	// 	shortest_path kernel3_para = sssp_parallel(g, node);
	// 	k3_para_time_ms += kernel3_para.time_ms;
	// 	k3_para_teps += kernel3_para.teps;
	// 	shortest_path_destroy(kernel3_para);

	// 	if (argv[1] != nullptr) {
	// 		for (int64_t i = 0; i < g.nb_nodes; i++) {
	// 			std::cout << "Node " << i << " dist=" << kernel3.distance_array[i] << " parent=" << kernel3.parent_array[i] << std::endl;
	// 		}
	// 	}
	// }
	// k3_time_ms /= NB_NODES_TO_TRY;
	// k3_teps /= NB_NODES_TO_TRY;
	// k3_para_time_ms /= NB_NODES_TO_TRY;
	// k3_para_teps /= NB_NODES_TO_TRY;
	// std::cout << "SSSP  - Avg. time: " << k3_time_ms << "ms, teps avg = " << k3_teps << "teps" << std::endl;
	// std::cout << "Parallel SSSP  - Avg. time: " << k3_para_time_ms << "ms, teps avg = " << k3_para_teps << "teps" << std::endl;

	graph_destroy(g);
	return 0;
}