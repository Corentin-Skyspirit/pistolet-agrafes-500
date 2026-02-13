#include "shortest_path.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>

shortest_path sssp(graph source_graph, int64_t root) {
	auto start = std::chrono::high_resolution_clock::now();
	shortest_path result;
	result.parent_array = (int64_t*)malloc(source_graph.length * sizeof(int64_t));
	result.distance_array = (float*)malloc(source_graph.length * sizeof(float));
	std::cout << "ouais" << std::endl;
	return result;
}