#include "generator/generator.hpp"
#include "kernels/breadth_first_search.hpp"
#include "kernels/gen_graph.hpp"
#include "kernels/shortest_path.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <omp.h>

int main(int argc, char const* argv[]) {
	std::cout << "OpenMP: num_threads = " << omp_get_max_threads() << '\n';

	////////////////////////////////////////////////////////////////////////////
	///////////////////////////////// KERNEL 1 /////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	// Compare different implementations of from_edge_list
	{
		edge_list list = generate_graph(8, 16);
		from_edge_list_try_all(list);
		edge_list_destroy(list);
	}

	printf("\nFINISHED COMPARING FROM EDGE LIST IMPLEMS\n\n");

	////////////////////////////////////////////////////////////////////////////
	///////////////////////////////// KERNEL 2 /////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	// Compare BFS using a bigger one
	// Parallel graph generation to speed up the process
	auto old_nb_threads = omp_get_max_threads();
	omp_set_num_threads(64);
	edge_list list = generate_graph(18, 16);
	omp_set_num_threads(old_nb_threads);

	graph g = from_edge_list(list);
	std::cout << "Graph generation : nb_nodes = " << g.nb_nodes << ", nb_neighbors = " << g.length << ", time = " << g.time_ms << "ms"
			  << '\n';
	edge_list_destroy(list);

	printf("\nCOMPARING BFS IMPLEMS\n\n");

	const int64_t NB_NODES_TO_TRY = 64;

	// Sequential BFS implementations
	if (omp_get_max_threads() == 1) {
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

				bfs_result kernel2 = bfs_hybrid_paper(g, node);
				k2_time_ms += kernel2.time_ms;
				k2_teps += kernel2.teps;
			}
			k2_time_ms /= NB_NODES_TO_TRY;
			k2_teps /= NB_NODES_TO_TRY;
			printf("BFS Hybrid (Paper) - Avg. time: %fms\n", k2_time_ms);
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
	}
	// Parallel BFS implementations
	else {
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

				bfs_result kernel2 = bfs_hybrid_paper(g, node);
				k2_time_ms += kernel2.time_ms;
				k2_teps += kernel2.teps;
			}
			k2_time_ms /= NB_NODES_TO_TRY;
			k2_teps /= NB_NODES_TO_TRY;
			printf("BFS Hybrid (Paper) - Avg. time: %fms\n", k2_time_ms);
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
	}

	////////////////////////////////////////////////////////////////////////////
	///////////////////////////////// KERNEL 3 /////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	// Compare SSSP using a smaller one
	// Parallel graph generation to speed up the process
	graph_destroy(g);
	old_nb_threads = omp_get_max_threads();
	omp_set_num_threads(64);
	list = generate_graph(15, 16);
	omp_set_num_threads(old_nb_threads);

	g = from_edge_list(list);
	std::cout << "Graph generation : nb_nodes = " << g.nb_nodes << ", nb_neighbors = " << g.length << ", time = " << g.time_ms << "ms"
			  << '\n';
	edge_list_destroy(list);

	// All results of different SSSP implementations.
	double k3_dj_time_ms = 0;
	double k3_dj_teps = 0;
	double k3_bf_time_ms = 0;
	double k3_bf_teps = 0;
	double k3_para_time_ms = 0;
	double k3_para_teps = 0;

	// Check NB_NODES_TO_TRY random nodes in the graph, non-degenerate ones (with degree > 0).
	for (int64_t i = 0; i < NB_NODES_TO_TRY;) {
		int64_t node = rand() % g.nb_nodes;
		if (degree_of_node(g, node) <= 0) {
			continue;
		}
		i++;

		// Kernel 3
		// Check sequential implementations if we only have one thread
		if (omp_get_max_threads() == 1) {
			// Dijkstra's algorithm
			shortest_path kernel3_dj = sssp_dj(g, node);
			k3_dj_time_ms += kernel3_dj.time_ms;
			k3_dj_teps += kernel3_dj.teps;
			shortest_path_destroy(kernel3_dj);

			// Bellman-Ford's algorithm
			shortest_path kernel3_bf = sssp_bf(g, node);
			k3_bf_time_ms += kernel3_bf.time_ms;
			k3_bf_teps += kernel3_bf.teps;
			shortest_path_destroy(kernel3_bf);
		}
		// Check parallel implementation otherwise
		else {
			// Parallel Bellman-Ford
			shortest_path kernel3_para = sssp_parallel(g, node);
			k3_para_time_ms += kernel3_para.time_ms;
			k3_para_teps += kernel3_para.teps;
			shortest_path_destroy(kernel3_para);
		}
	}
	// Average the results over the number of nodes we tried.
	k3_dj_time_ms /= NB_NODES_TO_TRY;
	k3_dj_teps /= NB_NODES_TO_TRY;
	k3_bf_time_ms /= NB_NODES_TO_TRY;
	k3_bf_teps /= NB_NODES_TO_TRY;
	k3_para_time_ms /= NB_NODES_TO_TRY;
	k3_para_teps /= NB_NODES_TO_TRY;
	std::cout << "Dijkstra SSSP  - Avg. time: " << k3_dj_time_ms << "ms, teps avg = " << k3_dj_teps << "teps" << '\n';
	std::cout << "Bellman-Ford SSSP  - Avg. time: " << k3_bf_time_ms << "ms, teps avg = " << k3_bf_teps << "teps" << '\n';
	std::cout << "Parallel B-F SSSP  - Avg. time: " << k3_para_time_ms << "ms, teps avg = " << k3_para_teps << "teps" << '\n';

	graph_destroy(g);
	return 0;
}