#include "breadth_search.hpp"
#include "gen_graph.hpp"
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <queue>
#include <unordered_set>
#include <utility>
#include <vector>

bool verify_bfs_result(const graph& g, int64_t source, int64_t* parents) {
	if (parents[source] != source) {
		std::fprintf(stderr, "verify_bfs_result: source %lld has parent %lld\n", (long long)source, (long long)parents[source]);
		return false;
	}

	// Verify that the parent and child are connected and they have valid IDs
	for (int64_t node = 0; node < g.nb_nodes; node++) {
		if (node == source) {
			continue;
		}

		int64_t parent = parents[node];

		// If the parent is -1 -> node wasn't able to be reached
		if (parent == -1) {
			continue;
		}
		// Verify that the parent has a valid ID
		if (parent < 0 || parent >= g.nb_nodes) {
			std::fprintf(stderr, "verify_bfs_result: node %lld has out-of-range parent %lld\n", (long long)node, (long long)parent);
			return false;
		}

		// Verify that the parent and child are connected
		bool found = false;
		for_each_neighbor(g, parent, [&](int64_t neigh, float) {
			if (neigh == node) {
				found = true;
			}
		});
		if (!found) {
			std::fprintf(
				stderr, "verify_bfs_result: parent %lld of node %lld is not connected to it\n", (long long)parent, (long long)node);
			return false;
		}
	}

	// Ensure no cycles in the result. Treat a node whose parent equals itself
	// as a root (terminal), so it does not form a cycle.
	typedef enum : uint8_t {
		UNVISITED,
		VISITING,
		DONE,
	} cycles_verification;
	std::vector<cycles_verification> state(g.nb_nodes, UNVISITED);
	for (int64_t i = 0; i < g.nb_nodes; ++i) {
		if (parents[i] == -1 || state[i] != UNVISITED) {
			continue;
		}
		int64_t current = i;
		while (current != -1 && state[current] == UNVISITED) {
			state[current] = VISITING;
			int64_t next = parents[current];
			if (next == current) { // self-parent -> treat as root
				current = -1;
				break;
			}
			current = next;
		}

		if (current != -1 && state[current] == VISITING) {
			// collect nodes in the cycle
			std::vector<int64_t> cycle;
			int64_t start = current;
			cycle.push_back(start);
			int64_t cur = parents[start];
			while (cur != start && cur != -1) {
				cycle.push_back(cur);
				cur = parents[cur];
			}
			std::fprintf(stderr, "verify_bfs_result: cycle detected (length %zu):", cycle.size());
			for (size_t k = 0; k < cycle.size(); ++k) {
				std::fprintf(stderr, " %lld", (long long)cycle[k]);
			}
			std::fprintf(stderr, "\n");
			return false;
		}

		// mark the chain as done (stop at self-parent too)
		current = i;
		while (current != -1 && state[current] == VISITING) {
			int64_t next = parents[current];
			state[current] = DONE;
			if (next == current)
				break;
			current = next;
		}
	}

	return true;
}

bfs_result formal_bfs(graph& g, int64_t source) {
	auto start = std::chrono::high_resolution_clock::now();

	int64_t* parents = (int64_t*)malloc(g.nb_nodes * sizeof(int64_t));
	memset(parents, -1, g.nb_nodes * (int64_t)sizeof(int64_t));
	parents[source] = source;

	bool* visited = (bool*)malloc(g.nb_nodes * sizeof(bool));

	std::queue<int64_t> queue;
	queue.push(source);

	while (queue.empty()) {
		int64_t node = queue.front();
		queue.pop();
		for_each_neighbor(g, node, [&](int64_t neighbor, float _weight) {
			if (!visited[node]) {
				visited[neighbor] = true;
				parents[neighbor] = node;
				queue.push(neighbor);
			}
		});
		visited[node] = true;
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto time_ms = std::chrono::duration<double, std::milli>(end - start).count();

	return (bfs_result){
		.parent_array = parents,
		.teps = 0.0,
		.time_ms = time_ms,
	};
}

bfs_result top_down_bfs(graph& g, int64_t source) {
	auto start = std::chrono::high_resolution_clock::now();

	int64_t* parents = (int64_t*)malloc(g.nb_nodes * sizeof(int64_t));
	memset(parents, -1, g.nb_nodes * (int64_t)sizeof(int64_t));
	parents[source] = source;

	std::unordered_set<int64_t> to_explore_now = {source};
	std::unordered_set<int64_t> to_explore_next = {};
	while (!to_explore_now.empty()) {
		for (const auto& node : to_explore_now) {
			for_each_neighbor(g, node, [&](int64_t neighbor, float _weight) {
				if (parents[neighbor] == -1) {
					to_explore_next.insert(neighbor);
					parents[neighbor] = node;
				}
			});
		}
		to_explore_now.clear();
		std::swap(to_explore_next, to_explore_now);
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto time_ms = std::chrono::duration<double, std::milli>(end - start).count();

	return (bfs_result){
		.parent_array = parents,
		.teps = 0.0,
		.time_ms = time_ms,
	};
}

bfs_result bfs(graph& g, int64_t source) {
	bfs_result result = formal_bfs(g, source);
	bool is_correct = verify_bfs_result(g, source, result.parent_array);
	if (!is_correct) {
		printf("Error in BFS kernel for node %lu\n", source);
	}

	return result;
}