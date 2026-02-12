#include "generator/generator.hpp"
#include "kernels/breadth_search.hpp"
#include "kernels/gen_graph.hpp"
#include "kernels/shortest_path.hpp"
#include <cstdio>
#include <iostream>

int main() {
	printf("Hello world\n");
	edge_list list = generate_graph(3, 5);
	print_edge_list(&list);
	graph g = from_edge_list(list);
	int64_t cpt = 0;
	for (int64_t id_n = 0; id_n < g.length; id_n++) {
		if (id_n == g.slicing_idx[cpt]) {
			std::cout << "Neighbors of " << cpt << std::endl;
			cpt++;
		}
		std::cout << "(" << g.neighbors[id_n] << "," << g.weights[id_n] << ")" << std::endl;
	}
	edge_list_destroy(list);
}