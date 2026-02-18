/// @file breadth_first_search.cpp
/// @brief BFS implementations

#include "breadth_first_search.hpp"
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

/// @brief Verify the correctness of a BFS result.
/// Checks that the parents and children are connected, that the source is its own parent, and that there are no cycles in the BFS result.
/// @param g The graph on which BFS was run.
/// @param source The source node from which BFS was run.
/// @param parents The array of parent nodes returned by BFS, where parents[i] is the parent of node i (or -1 if unreachable).
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

	// Ensure no cycles in the result.
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
			if (next == current) { // Self-parent -> treat as root
				current = -1;
				break;
			}
			current = next;
		}

		if (current != -1 && state[current] == VISITING) { // Found a cycle

			// Collect nodes in the cycle
			std::vector<int64_t> cycle;
			int64_t start = current;
			cycle.push_back(start);
			int64_t cur = parents[start];
			while (cur != start && cur != -1) {
				cycle.push_back(cur);
				cur = parents[cur];
			}

			// Print the cycle
			std::fprintf(stderr, "verify_bfs_result: cycle detected (length %zu):", cycle.size());
			for (size_t k = 0; k < cycle.size(); ++k) {
				std::fprintf(stderr, " %lld", (long long)cycle[k]);
			}
			std::fprintf(stderr, "\n");

			return false;
		}

		// Mark the chain as done (stop at self-parent too)
		current = i;
		while (current != -1 && state[current] == VISITING) {
			int64_t next = parents[current];
			state[current] = DONE;
			if (next == current) {
				break;
			}
			current = next;
		}
	}

	return true;
}

/// @brief "Formal" BFS implementation using a queue and a visited array.
/// @param g The graph on which to run BFS.
/// @param source The source node from which to start BFS.
bfs_result bfs_formal(graph& g, int64_t source) {
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

/// @brief Perform one step of the top-down BFS using unordered_set as frontier representation.
/// @param g The graph on which to run BFS.
/// @param frontier The current frontier of the BFS.
/// @param next The next frontier to be filled by this step.
/// @param parents The array of parent nodes.
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

/// @brief Perform one step of the bottom-up BFS using unordered_set as frontier representation.
/// @param g The graph on which to run BFS.
/// @param frontier The current frontier of the BFS.
/// @param next The next frontier to be filled by this step.
/// @param parents The array of parent nodes.
static void bottom_up_step(graph& g, std::unordered_set<int64_t>& frontier, std::unordered_set<int64_t>& next, int64_t*& parents) {
	for (int64_t node = 0; node < g.nb_nodes; node++) {
		if (parents[node] != -1) {
			continue; // Already visited
		}

		// Find a neighbor which belongs in the frontier
		// Can't use my fancy for_each_neighbor macro here because I have to break early when I find a parent :(
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

/// @brief BFS implementation using unordered_set as frontier representation, and only top-down steps.
/// @param g The graph on which to run BFS.
/// @param source The source node from which to start BFS.
bfs_result bfs_full_top_down(graph& g, int64_t source) {
	auto start = std::chrono::high_resolution_clock::now();

	auto frontier = std::unordered_set<int64_t>{};
	frontier.insert(source);
	auto next = std::unordered_set<int64_t>{};
	int64_t* parents = (int64_t*)malloc(g.nb_nodes * sizeof(int64_t));
	std::fill(parents, parents + g.nb_nodes, -1);
	parents[source] = source;

	// Perform BFS until no more nodes to visit
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

/// @brief BFS implementation using unordered_set as frontier representation, and only bottom-up steps.
/// @param g The graph on which to run BFS.
/// @param source The source node from which to start BFS.
bfs_result bfs_full_bottom_up(graph& g, int64_t source) {
	auto start = std::chrono::high_resolution_clock::now();

	auto frontier = std::unordered_set<int64_t>{};
	frontier.insert(source);
	auto next = std::unordered_set<int64_t>{};
	int64_t* parents = (int64_t*)malloc(g.nb_nodes * sizeof(int64_t));
	std::fill(parents, parents + g.nb_nodes, -1);
	parents[source] = source;

	// Perform BFS until no more nodes to visit
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

////////////////////////////////////////////////////////
//////////// BITSET FRONTIER IMPLEMENTATION ////////////
////////////////////////////////////////////////////////

/// @brief Perform one step of the top-down BFS using bitset as frontier representation.
/// @param g The graph on which to run BFS.
/// @param frontier The current frontier of the BFS.
/// @param next The next frontier to be filled by this step.
/// @param parents The array of parent nodes.
static void top_down_step_bitset(graph& g, bitset& frontier, bitset& next, int64_t*& parents) {
	frontier.for_each([&](int64_t node) {
		for_each_neighbor(g, node, [&](int64_t neighbor, float _weight) {
			if (parents[neighbor] == -1) {
				parents[neighbor] = node;
				next.insert(neighbor);
			}
		});
	});
}

/// @brief Perform one step of the bottom-up BFS using bitset as frontier representation.
/// @param g The graph on which to run BFS.
/// @param frontier The current frontier of the BFS.
/// @param next The next frontier to be filled by this step.
/// @param parents The array of parent nodes.
static void bottom_up_step_bitset(graph& g, bitset& frontier, bitset& next, int64_t*& parents) {
	for (int64_t node = 0; node < g.nb_nodes; node++) {
		if (parents[node] != -1) {
			continue; // Already visited
		}

		// Find a neighbor which belongs in the frontier
		// Can't use my fancy for_each_neighbor macro either for the same reason.
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

/// @brief BFS implementation using bitset as frontier representation, and only top-down steps.
/// @param g The graph on which to run BFS.
/// @param source The source node from which to start BFS.
bfs_result bfs_full_top_down_bitset(graph& g, int64_t source) {
	auto start = std::chrono::high_resolution_clock::now();

	bitset frontier(g.nb_nodes);
	bitset next(g.nb_nodes);
	int64_t* parents = (int64_t*)malloc(g.nb_nodes * sizeof(int64_t));
	std::fill(parents, parents + g.nb_nodes, -1);
	parents[source] = source;
	frontier.insert(source);

	// Perform BFS until no more nodes to visit
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

/// @brief BFS implementation using bitset as frontier representation, and only bottom-up steps.
/// @param g The graph on which to run BFS.
/// @param source The source node from which to start BFS.
bfs_result bfs_full_bottom_up_bitset(graph& g, int64_t source) {
	auto start = std::chrono::high_resolution_clock::now();

	bitset frontier(g.nb_nodes);
	bitset next(g.nb_nodes);
	int64_t* parents = (int64_t*)malloc(g.nb_nodes * sizeof(int64_t));
	std::fill(parents, parents + g.nb_nodes, -1);
	parents[source] = source;
	frontier.insert(source);

	// Perform BFS until no more nodes to visit
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

/// @brief Perform one step of the top-down BFS using bitset as frontier representation, and parallel.
static void top_down_step_parallel_bitset(graph& g, atomic_bitset& frontier, atomic_bitset& next, std::atomic<int64_t>*& parents) {
	frontier.parallel_for_each([&](int64_t node) {
		for_each_neighbor(g, node, [&](int64_t neighbor, float _weight) {
			if (parents[neighbor].load(std::memory_order_acquire) == -1) {
				parents[neighbor].store(node, std::memory_order_release);
				next.insert(neighbor);
			}
		});
	});
}

/// @brief Perform one step of the bottom-up BFS using atomic_bitset as frontier representation, and parallel.
static void bottom_up_step_parallel_bitset(graph& g, atomic_bitset& frontier, atomic_bitset& next, std::atomic<int64_t>*& parents) {
#pragma omp parallel for schedule(dynamic, 1024)
	for (int64_t node = 0; node < g.nb_nodes; node++) {
		if (parents[node].load(std::memory_order_acquire) != -1) {
			continue; // Already visited
		}

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

/// @brief BFS implementation using atomic_bitset as frontier representation, and only top-down steps, and parallel.
/// @param g The graph on which to run BFS.
/// @param source The source node from which to start BFS.
bfs_result bfs_full_top_down_parallel_bitset(graph& g, int64_t source) {
	auto start = std::chrono::high_resolution_clock::now();

	auto frontier = atomic_bitset(g.nb_nodes);
	auto next = atomic_bitset(g.nb_nodes);
	std::atomic<int64_t>* parents = new std::atomic<int64_t>[g.nb_nodes];
	for (int64_t i = 0; i < g.nb_nodes; i++) {
		parents[i].store(-1, std::memory_order_release);
	}
	parents[source].store(source, std::memory_order_release);
	frontier.insert(source);

	// Perform BFS until no more nodes to visit
	while (!frontier.empty()) {
		// Perform step k in parallel
		top_down_step_parallel_bitset(g, frontier, next, parents);
		atomic_bitset_swap(frontier, next);
		next.clear();
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto time_ms = std::chrono::duration<double, std::milli>(end - start).count();

	return (bfs_result){
		.parent_array = (int64_t*)parents, // Should be fine to cast from atomic<int64>* to int64* I think?
		.teps = 0.0,
		.time_ms = time_ms,
	};
}

/// @brief BFS implementation using atomic_bitset as frontier representation, and only bottom-up steps, and parallel.
/// @param g The graph on which to run BFS.
/// @param source The source node from which to start BFS.
bfs_result bfs_full_bottom_up_parallel_bitset(graph& g, int64_t source) {
	auto start = std::chrono::high_resolution_clock::now();

	auto frontier = atomic_bitset(g.nb_nodes);
	auto next = atomic_bitset(g.nb_nodes);
	std::atomic<int64_t>* parents = new std::atomic<int64_t>[g.nb_nodes];
	for (int64_t i = 0; i < g.nb_nodes; i++) {
		parents[i].store(-1, std::memory_order_release);
	}
	parents[source].store(source, std::memory_order_release);
	frontier.insert(source);

	// Perform BFS until no more nodes to visit
	while (!frontier.empty()) {
		bottom_up_step_parallel_bitset(g, frontier, next, parents);
		atomic_bitset_swap(frontier, next);
		next.clear();
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto time_ms = std::chrono::duration<double, std::milli>(end - start).count();

	return (bfs_result){
		.parent_array = (int64_t*)parents, // Same as top_down about casting from atomic<int64>* to int64*
		.teps = 0.0,
		.time_ms = time_ms,
	};
}

////////////////////////////////////////////////////////
////////// HYBRID DIRECTION IMPLEMENTATION /////////////
////////////////////////////////////////////////////////

/// @brief Compute the sum of degrees of the nodes in a frontier represented as an atomic_bitset.
static int64_t atomic_bitset_sum_of_degrees(const atomic_bitset& bitset, const graph& g) {
	int64_t sum = 0;
	bitset.for_each([&](int64_t node) { sum += degree_of_node(g, node); });
	return sum;
}

/// @brief A set of integers that handles switching between an unordered_set and a bitset representation as requested.
/// It's an abstraction to help reduce code bloat when switching in the hybrid algorithm is needed.
typedef struct hybrid_set {
	///////////////////// Representations //////////////////////
	std::unordered_set<int64_t> unordered_set; ///< unordered_set representation of the set.
	atomic_bitset bitset;					   ///< bitset representation of the set.

	enum FrontierType : uint8_t {
		UNORDERED_SET,
		BITSET,
	} currently_using; ///< Indicates which representation is currently used.

	/// @brief Construct a new hybrid_set with the given number of nodes (for bitset size). Initially empty and using unordered_set
	/// representation.
	hybrid_set(int64_t nb_nodes) : currently_using(UNORDERED_SET), unordered_set(), bitset(nb_nodes) {}

	/// @brief Get the unordered_set representation of the set, converting from bitset if necessary.
	std::unordered_set<int64_t>& as_unordered_set() {
		if (this->currently_using == FrontierType::UNORDERED_SET) {
			return unordered_set;
		}
		else {
			unordered_set.clear();
			bitset.for_each([&](int64_t v) { unordered_set.insert(v); });
			this->currently_using = FrontierType::UNORDERED_SET;
			return unordered_set;
		}
	}

	/// @brief Get the bitset representation of the set, converting from unordered_set if necessary.
	atomic_bitset& as_bitset() {
		if (this->currently_using == FrontierType::BITSET) {
			return bitset;
		}
		else {
			bitset.clear();
			for (auto v : unordered_set) {
				bitset.insert(v);
			}
			this->currently_using = FrontierType::BITSET;
			return bitset;
		}
	}

	/// @brief Check if the set is empty.
	bool empty() const {
		if (this->currently_using == FrontierType::UNORDERED_SET) {
			return unordered_set.empty();
		}
		else {
			return bitset.empty();
		}
	}

	/// @brief Clear the set.
	void clear() {
		if (this->currently_using == FrontierType::UNORDERED_SET) {
			unordered_set.clear();
		}
		else {
			bitset.clear();
		}
	}

	/// @brief Compute the sum of degrees of the nodes in the frontier.
	int64_t sum_of_degrees(const graph& g) const {
		int64_t sum = 0;
		if (this->currently_using == FrontierType::UNORDERED_SET) {
			for (auto v : unordered_set) {
				sum += degree_of_node(g, v);
			}
		}
		else {
			sum = atomic_bitset_sum_of_degrees(bitset, g);
		}
		return sum;
	}
} hybrid_set;

/// @brief Hybrid direction-optimizing BFS using an unordered_set for top-down steps and a bitset for bottom-up steps, like advised in the
/// paper. Switch heuristic taken from it too.
/// @param g The graph on which to run BFS.
/// @param source The source node from which to start BFS.
bfs_result bfs_hybrid_paper(graph& g, int64_t source) {
	auto start = std::chrono::high_resolution_clock::now();

	hybrid_set frontier(g.nb_nodes);
	frontier.as_unordered_set().insert(source);

	hybrid_set next(g.nb_nodes);

	// Same pointers for both implementations (atomic and regular ints) to simplify switching and avoid copying when not needed
	int64_t* parents = (int64_t*)malloc(g.nb_nodes * sizeof(int64_t));
	std::atomic<int64_t>* parents_atomic = (std::atomic<int64_t>*)parents;
	std::fill(parents, parents + g.nb_nodes, -1);
	parents[source] = source;

	bool top_down = true;
	const double C_TB = 10.0; // Threshold for switching top->bottom
	const double C_BT = 40.0; // Threshold for switching bottom->top

	while (!frontier.empty()) {
		uint64_t sum_of_degrees_in_frontier = frontier.sum_of_degrees(g);

		uint64_t sum_of_degrees_unexplored = 0;
		for (int64_t u = 0; u < g.nb_nodes; u++) {
			if (parents[u] == -1) {
				sum_of_degrees_unexplored += degree_of_node(g, u);
			}
		}

		// Switch direction if needed according to the heuristic from the paper (m_f > m_u / C_TB to switch top->bottom, and m_f < m_u /
		// C_BT to switch bottom->top)
		if (top_down) {
			if (sum_of_degrees_in_frontier > sum_of_degrees_unexplored / C_TB) {
				top_down = false;
			}
		}
		else {
			if (sum_of_degrees_in_frontier < sum_of_degrees_unexplored / C_BT) {
				top_down = true;
			}
		}

		// Perform step k in the calculated direction
		if (top_down) {
			top_down_step(g, frontier.as_unordered_set(), next.as_unordered_set(), parents);
			frontier.as_unordered_set() = std::move(next.as_unordered_set());
			next.as_unordered_set().clear();
		}
		else {
			bottom_up_step_parallel_bitset(g, frontier.as_bitset(), next.as_bitset(), parents_atomic);
			atomic_bitset_swap(frontier.as_bitset(), next.as_bitset());
			next.as_bitset().clear();
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

/// @brief Hybrid direction-optimizing BFS using a unique atomic_bitset for both top-down and bottom-up steps.
/// No more switching of data structures needed.
/// @param g The graph on which to run BFS.
/// @param source The source node from which to start BFS.
bfs_result bfs_hybrid(graph& g, int64_t source) {
	auto start = std::chrono::high_resolution_clock::now();

	atomic_bitset frontier(g.nb_nodes);
	frontier.insert(source);

	atomic_bitset next(g.nb_nodes);

	// Same pointers for both implementations to simplify switching and avoid copying when not needed
	int64_t* parents = (int64_t*)malloc(g.nb_nodes * sizeof(int64_t));
	std::atomic<int64_t>* parents_atomic = (std::atomic<int64_t>*)parents;
	std::fill(parents, parents + g.nb_nodes, -1);
	parents[source] = source;

	bool top_down = true;
	const double C_TB = 10.0; // threshold for switching top->bottom
	const double C_BT = 40.0; // threshold for switching bottom->top

	while (!frontier.empty()) {
		uint64_t sum_of_degrees_in_frontier = atomic_bitset_sum_of_degrees(frontier, g);

		uint64_t sum_of_degrees_unexplored = 0;
		for (int64_t u = 0; u < g.nb_nodes; u++) {
			if (parents[u] == -1) {
				sum_of_degrees_unexplored += degree_of_node(g, u);
			}
		}

		// Switch direction if needed according to the heuristic from the paper (m_f > m_u / C_TB to switch top->bottom, and m_f < m_u /
		// C_BT to switch bottom->top)
		if (top_down) {
			if (sum_of_degrees_in_frontier > sum_of_degrees_unexplored / C_TB) {
				top_down = false;
			}
		}
		else {
			if (sum_of_degrees_in_frontier < sum_of_degrees_unexplored / C_BT) {
				top_down = true;
			}
		}

		// Perform step k in the calculated direction
		if (top_down) {
			top_down_step_parallel_bitset(g, frontier, next, parents_atomic);
			atomic_bitset_swap(frontier, next);
			next.clear();
		}
		else {
			bottom_up_step_parallel_bitset(g, frontier, next, parents_atomic);
			atomic_bitset_swap(frontier, next);
			next.clear();
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

/// @brief Run all BFS implementations on the given graph and source node, verify their correctness, and print their execution times.
/// @param g The graph on which to run BFS.
/// @param source The source node from which to start BFS.
bfs_result all_bfs(graph& g, int64_t source) {
	bfs_result result;

	{
		result = bfs_formal(g, source);
		bool is_correct = verify_bfs_result(g, source, result.parent_array);
		if (!is_correct) {
			printf("Error in formal BFS kernel for node %lu\n", source);
		}
		printf("Formal BFS: time = %.2f ms\n", result.time_ms);
	}

	{
		result = bfs_hybrid(g, source);
		bool is_correct = verify_bfs_result(g, source, result.parent_array);
		if (!is_correct) {
			printf("Error in hybrid BFS kernel for node %lu\n", source);
		}
		printf("Hybrid BFS: time = %.2f ms\n", result.time_ms);
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
		result = bfs_full_bottom_up_parallel_bitset(g, source);
		bool is_correct = verify_bfs_result(g, source, result.parent_array);
		if (!is_correct) {
			printf("Error in bottom-up atomic bitset BFS kernel for node %lu\n", source);
		}
		printf("Bottom-up atomic bitset BFS: time = %.2f ms\n", result.time_ms);
	}

	{
		result = bfs_full_top_down_parallel_bitset(g, source);
		bool is_correct = verify_bfs_result(g, source, result.parent_array);
		if (!is_correct) {
			printf("Error in top-down atomic bitset BFS kernel for node %lu\n", source);
		}
		printf("Top-down atomic bitset BFS: time = %.2f ms\n", result.time_ms);
	}

	return result;
}