#include "breadth_search.hpp"
#include "bitset.hpp"
#include "gen_graph.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <queue>
#include <unordered_set>
#include <vector>

static bool verify_bfs_result(const graph& g, int64_t source, int64_t* parents) {
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

static bfs_result formal_bfs(graph& g, int64_t source) {
	auto start = std::chrono::high_resolution_clock::now();

	int64_t* parents = (int64_t*)malloc(g.nb_nodes * sizeof(int64_t));
	std::fill(parents, parents + g.nb_nodes, -1);
	parents[source] = source;

	bool* visited = (bool*)malloc(g.nb_nodes * sizeof(bool));
	std::fill(visited, visited + g.nb_nodes, false);

	std::queue<int64_t> queue;
	queue.push(source);
	visited[source] = true;

	while (!queue.empty()) {
		int64_t node = queue.front();
		queue.pop();
		for_each_neighbor(g, node, [&](int64_t neighbor, float _weight) {
			if (!visited[neighbor]) {
				visited[neighbor] = true;
				parents[neighbor] = node;
				queue.push(neighbor);
			}
		});
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto time_ms = std::chrono::duration<double, std::milli>(end - start).count();

	free(visited);

	return (bfs_result){
		.parent_array = parents,
		.teps = 0.0,
		.time_ms = time_ms,
	};
}

///////////////////////////////////////////////////////////////
//////////// UNORDERED SET FRONTIER IMPLEMENTATION ////////////
///////////////////////////////////////////////////////////////

static void top_down_step(graph& g, std::unordered_set<int64_t>& frontier, std::unordered_set<int64_t>& next, int64_t*& parents) {
	for (auto node : frontier) {
		for_each_neighbor(g, node, [&](int64_t neighbor, float _weight) {
			if (parents[neighbor] == -1) {
				parents[neighbor] = node;
				next.insert(neighbor);
			}
		});
	}
}

static void bottom_up_step(graph& g, std::unordered_set<int64_t>& frontier, std::unordered_set<int64_t>& next, int64_t*& parents) {
	for (int64_t node = 0; node < g.nb_nodes; node++) {
		if (parents[node] == -1) {
			int64_t start = g.slicing_idx[node];
			int64_t end = g.slicing_idx[node + 1];
			for (int64_t i = start; i < end; i++) {
				int64_t neighbor = g.neighbors[i];
				if (frontier.find(neighbor) != frontier.end()) {
					parents[node] = neighbor;
					next.insert(node);
					break;
				}
			}
		}
	}
}

static bfs_result bfs_full_top_down(graph& g, int64_t source) {
	auto start = std::chrono::high_resolution_clock::now();

	auto frontier = std::unordered_set<int64_t>{};
	frontier.insert(source);
	auto next = std::unordered_set<int64_t>{};
	int64_t* parents = (int64_t*)malloc(g.nb_nodes * sizeof(int64_t));
	std::fill(parents, parents + g.nb_nodes, -1);
	parents[source] = source;

	while (!frontier.empty()) {
		top_down_step(g, frontier, next, parents);
		frontier = next;
		next.clear();
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto time_ms = std::chrono::duration<double, std::milli>(end - start).count();

	return (bfs_result){
		.parent_array = parents,
		.teps = 0.0,
		.time_ms = time_ms,
	};
}

static bfs_result bfs_full_bottom_up(graph& g, int64_t source) {
	auto start = std::chrono::high_resolution_clock::now();

	auto frontier = std::unordered_set<int64_t>{};
	frontier.insert(source);
	auto next = std::unordered_set<int64_t>{};
	int64_t* parents = (int64_t*)malloc(g.nb_nodes * sizeof(int64_t));
	std::fill(parents, parents + g.nb_nodes, -1);
	parents[source] = source;

	while (!frontier.empty()) {
		bottom_up_step(g, frontier, next, parents);
		frontier = next;
		next.clear();
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto time_ms = std::chrono::duration<double, std::milli>(end - start).count();

	return (bfs_result){
		.parent_array = parents,
		.teps = 0.0,
		.time_ms = time_ms,
	};
}

// Hybrid direction-optimizing BFS using unordered_set frontier.
// Switches between top-down and bottom-up using a simple heuristic based
// on the number of edges to check from the frontier (m_f) and from
// unexplored vertices (m_u). Constants C_TB and C_BT tune switching.
static bfs_result bfs_hybrid_unordered_set(graph& g, int64_t source) {
	auto start = std::chrono::high_resolution_clock::now();

	// start with unordered_set frontier
	auto frontier_set = std::unordered_set<int64_t>{};
	frontier_set.insert(source);
	auto next_set = std::unordered_set<int64_t>{};

	// atomic bitset frontiers (created once and reused for conversions)
	AtomicBitSet frontier_atomic(g.nb_nodes);
	AtomicBitSet next_atomic(g.nb_nodes);

	int64_t* parents = (int64_t*)malloc(g.nb_nodes * sizeof(int64_t));
	std::fill(parents, parents + g.nb_nodes, -1);
	parents[source] = source;

	bool using_set = true;
	bool top_down = true;
	const double C_TB = 10.0; // threshold for switching top->bottom
	const double C_BT = 40.0; // threshold for switching bottom->top

	while ((using_set && !frontier_set.empty()) || (!using_set && !frontier_atomic.empty())) {
		// compute m_f = sum degrees of frontier
		uint64_t m_f = 0;
		if (using_set) {
			for (auto v : frontier_set) {
				m_f += (uint64_t)(g.slicing_idx[v + 1] - g.slicing_idx[v]);
			}
		}
		else {
			frontier_atomic.for_each_set([&](int64_t v) { m_f += (uint64_t)(g.slicing_idx[v + 1] - g.slicing_idx[v]); });
		}

		// compute m_u = sum degrees of unexplored vertices
		uint64_t m_u = 0;
		for (int64_t v = 0; v < g.nb_nodes; ++v) {
			if (parents[v] == -1) {
				m_u += (uint64_t)(g.slicing_idx[v + 1] - g.slicing_idx[v]);
			}
		}

		if (top_down) {
			if (m_f > m_u / C_TB) {
				top_down = false;
			}
		}
		else {
			if (m_f < m_u / C_BT) {
				top_down = true;
			}
		}

		if (top_down) {
			// ensure we have set representation
			if (!using_set) {
				frontier_set.clear();
				frontier_atomic.for_each_set([&](int64_t v) { frontier_set.insert(v); });
				using_set = true;
			}
			top_down_step(g, frontier_set, next_set, parents);

			frontier_set = std::move(next_set);
			next_set.clear();
		}
		else {
			// ensure we have atomic bitset representation
			if (using_set) {
				frontier_atomic.clear();
				next_atomic.clear();
				for (auto v : frontier_set)
					frontier_atomic.insert(v);
				using_set = false;
			}

			// bottom-up step using atomic frontier, but regular parents array
			for (int64_t node = 0; node < g.nb_nodes; node++) {
				if (parents[node] == -1) {
					int64_t start = g.slicing_idx[node];
					int64_t end = g.slicing_idx[node + 1];
					for (int64_t i = start; i < end; i++) {
						int64_t neighbor = g.neighbors[i];
						if (frontier_atomic.contains(neighbor)) {
							parents[node] = neighbor;
							next_atomic.insert(node);
							break;
						}
					}
				}
			}

			atomic_bitset_swap(frontier_atomic, next_atomic);
			next_atomic.clear();
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto time_ms = std::chrono::duration<double, std::milli>(end - start).count();

	return (bfs_result){
		.parent_array = parents,
		.teps = 0.0,
		.time_ms = time_ms,
	};
}

////////////////////////////////////////////////////////
//////////// BITSET FRONTIER IMPLEMENTATION ////////////
////////////////////////////////////////////////////////

static void top_down_step_bitset(graph& g, BitSet& frontier, BitSet& next, int64_t*& parents) {
	frontier.for_each_set([&](int64_t node) {
		for_each_neighbor(g, node, [&](int64_t neighbor, float _weight) {
			if (parents[neighbor] == -1) {
				parents[neighbor] = node;
				next.insert(neighbor);
			}
		});
	});
}

static void bottom_up_step_bitset(graph& g, BitSet& frontier, BitSet& next, int64_t*& parents) {
	for (int64_t node = 0; node < g.nb_nodes; node++) {
		if (parents[node] == -1) {
			int64_t start = g.slicing_idx[node];
			int64_t end = g.slicing_idx[node + 1];
			for (int64_t i = start; i < end; i++) {
				int64_t neighbor = g.neighbors[i];
				if (frontier.contains(neighbor)) {
					parents[node] = neighbor;
					next.insert(node);
					break;
				}
			}
		}
	}
}

static bfs_result bfs_full_top_down_bitset(graph& g, int64_t source) {
	auto start = std::chrono::high_resolution_clock::now();

	auto frontier = BitSet(g.nb_nodes);
	auto next = BitSet(g.nb_nodes);
	int64_t* parents = (int64_t*)malloc(g.nb_nodes * sizeof(int64_t));
	std::fill(parents, parents + g.nb_nodes, -1);
	parents[source] = source;
	frontier.insert(source);
	while (!frontier.empty()) {
		top_down_step_bitset(g, frontier, next, parents);
		bitset_swap(frontier, next);
		next.clear();
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto time_ms = std::chrono::duration<double, std::milli>(end - start).count();

	return (bfs_result){
		.parent_array = parents,
		.teps = 0.0,
		.time_ms = time_ms,
	};
}

static bfs_result bfs_full_bottom_up_bitset(graph& g, int64_t source) {
	auto start = std::chrono::high_resolution_clock::now();

	auto frontier = BitSet(g.nb_nodes);
	auto next = BitSet(g.nb_nodes);
	int64_t* parents = (int64_t*)malloc(g.nb_nodes * sizeof(int64_t));
	std::fill(parents, parents + g.nb_nodes, -1);
	parents[source] = source;
	frontier.insert(source);
	while (!frontier.empty()) {
		bottom_up_step_bitset(g, frontier, next, parents);
		bitset_swap(frontier, next);
		next.clear();
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto time_ms = std::chrono::duration<double, std::milli>(end - start).count();

	return (bfs_result){
		.parent_array = parents,
		.teps = 0.0,
		.time_ms = time_ms,
	};
}

////////////////////////////////////////////////////////
/////// PARALLEL BITSET FRONTIER IMPLEMENTATION ////////
////////////////////////////////////////////////////////

static void top_down_step_atomic_bitset(graph& g, AtomicBitSet& frontier, AtomicBitSet& next, std::atomic<int64_t>*& parents) {
	frontier.parallel_for_each_set([&](int64_t node) {
		for_each_neighbor(g, node, [&](int64_t neighbor, float _weight) {
			if (parents[neighbor].load(std::memory_order_acquire) == -1) {
				parents[neighbor].store(node, std::memory_order_release);
				next.insert(neighbor);
			}
		});
	});
}

static void bottom_up_step_atomic_bitset(graph& g, AtomicBitSet& frontier, AtomicBitSet& next, std::atomic<int64_t>*& parents) {
#pragma omp parallel for schedule(dynamic, 1024)
	for (int64_t node = 0; node < g.nb_nodes; node++) {
		if (parents[node].load(std::memory_order_acquire) == -1) {
			int64_t start = g.slicing_idx[node];
			int64_t end = g.slicing_idx[node + 1];
			for (int64_t i = start; i < end; i++) {
				int64_t neighbor = g.neighbors[i];
				if (frontier.contains(neighbor)) {
					parents[node].store(neighbor, std::memory_order_release);
					next.insert(node);
					break;
				}
			}
		}
	}
}

static bfs_result bfs_full_top_down_atomic_bitset(graph& g, int64_t source) {
	auto start = std::chrono::high_resolution_clock::now();

	auto frontier = AtomicBitSet(g.nb_nodes);
	auto next = AtomicBitSet(g.nb_nodes);
	std::atomic<int64_t>* parents = new std::atomic<int64_t>[g.nb_nodes];
	for (int64_t i = 0; i < g.nb_nodes; i++) {
		parents[i].store(-1, std::memory_order_release);
	}
	parents[source].store(source, std::memory_order_release);
	frontier.insert(source);
	while (!frontier.empty()) {
		top_down_step_atomic_bitset(g, frontier, next, parents);
		atomic_bitset_swap(frontier, next);
		next.clear();
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto time_ms = std::chrono::duration<double, std::milli>(end - start).count();

	// Turn atomic parents into regular int64_t array for easier handling by caller, not timing it because we have the result already in the
	// atomic array
	int64_t* parents_array = (int64_t*)malloc(g.nb_nodes * sizeof(int64_t));
	for (int64_t i = 0; i < g.nb_nodes; i++) {
		parents_array[i] = parents[i].load(std::memory_order_acquire);
	}
	delete[] parents;

	return (bfs_result){
		.parent_array = parents_array,
		.teps = 0.0,
		.time_ms = time_ms,
	};
}

static bfs_result bfs_full_bottom_up_atomic_bitset(graph& g, int64_t source) {
	auto start = std::chrono::high_resolution_clock::now();

	auto frontier = AtomicBitSet(g.nb_nodes);
	auto next = AtomicBitSet(g.nb_nodes);
	std::atomic<int64_t>* parents = new std::atomic<int64_t>[g.nb_nodes];
	for (int64_t i = 0; i < g.nb_nodes; i++) {
		parents[i].store(-1, std::memory_order_release);
	}
	parents[source].store(source, std::memory_order_release);
	frontier.insert(source);
	while (!frontier.empty()) {
		bottom_up_step_atomic_bitset(g, frontier, next, parents);
		atomic_bitset_swap(frontier, next);
		next.clear();
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto time_ms = std::chrono::duration<double, std::milli>(end - start).count();

	// Turn atomic parents into regular int64_t array for same reason as in top-down version
	int64_t* parents_array = (int64_t*)malloc(g.nb_nodes * sizeof(int64_t));
	for (int64_t i = 0; i < g.nb_nodes; i++) {
		parents_array[i] = parents[i].load(std::memory_order_acquire);
	}
	delete[] parents;

	return (bfs_result){
		.parent_array = parents_array,
		.teps = 0.0,
		.time_ms = time_ms,
	};
}

bfs_result bfs(graph& g, int64_t source) {
	// bfs_result result = bfs_full_bottom_up(g, source);
	// bool is_correct = verify_bfs_result(g, source, result.parent_array);
	// if (!is_correct) {
	// 	printf("Error in BFS kernel for node %lu\n", source);
	// }

	bfs_result result;

	{
		result = formal_bfs(g, source);
		bool is_correct = verify_bfs_result(g, source, result.parent_array);
		if (!is_correct) {
			printf("Error in formal BFS kernel for node %lu\n", source);
		}
		printf("Formal BFS: time = %.2f ms\n", result.time_ms);
	}

	{
		result = bfs_hybrid_unordered_set(g, source);
		bool is_correct = verify_bfs_result(g, source, result.parent_array);
		if (!is_correct) {
			printf("Error in hybrid BFS kernel for node %lu\n", source);
		}
		printf("Hybrid (unordered_set) BFS: time = %.2f ms\n", result.time_ms);
	}

	{
		result = bfs_full_bottom_up(g, source);
		bool is_correct = verify_bfs_result(g, source, result.parent_array);
		if (!is_correct) {
			printf("Error in bottom-up BFS kernel for node %lu\n", source);
		}
		printf("Bottom-up BFS: time = %.2f ms\n", result.time_ms);
	}

	{
		result = bfs_full_top_down(g, source);
		bool is_correct = verify_bfs_result(g, source, result.parent_array);
		if (!is_correct) {
			printf("Error in top-down BFS kernel for node %lu\n", source);
		}
		printf("Top-down BFS: time = %.2f ms\n", result.time_ms);
	}

	{
		result = bfs_full_bottom_up_bitset(g, source);
		bool is_correct = verify_bfs_result(g, source, result.parent_array);
		if (!is_correct) {
			printf("Error in bottom-up bitset BFS kernel for node %lu\n", source);
		}
		printf("Bottom-up bitset BFS: time = %.2f ms\n", result.time_ms);
	}

	{
		result = bfs_full_top_down_bitset(g, source);
		bool is_correct = verify_bfs_result(g, source, result.parent_array);
		if (!is_correct) {
			printf("Error in top-down bitset BFS kernel for node %lu\n", source);
		}
		printf("Top-down bitset BFS: time = %.2f ms\n", result.time_ms);
	}

	{
		result = bfs_full_bottom_up_atomic_bitset(g, source);
		bool is_correct = verify_bfs_result(g, source, result.parent_array);
		if (!is_correct) {
			printf("Error in bottom-up atomic bitset BFS kernel for node %lu\n", source);
		}
		printf("Bottom-up atomic bitset BFS: time = %.2f ms\n", result.time_ms);
	}

	{
		result = bfs_full_top_down_atomic_bitset(g, source);
		bool is_correct = verify_bfs_result(g, source, result.parent_array);
		if (!is_correct) {
			printf("Error in top-down atomic bitset BFS kernel for node %lu\n", source);
		}
		printf("Top-down atomic bitset BFS: time = %.2f ms\n", result.time_ms);
	}

	return result;
}