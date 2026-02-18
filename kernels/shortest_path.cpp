/// @file shortest_path.cpp
/// @brief Shortest path implementations

#include "shortest_path.hpp"
#include <assert.h>
#include <chrono>
#include <cstring>
#include <limits>
#include <omp.h>
#include <queue>

/// @brief Free the memory allocated for a shortest_path result.
void shortest_path_destroy(shortest_path& sp) {
	free(sp.distance_array);
	free(sp.parent_array);
}

/// @brief Shortest path with Dijkstra's algorithm (sequential).
/// @param g Input graph
/// @param root Starting node for SSSP
/// @return List of shortest distances from root to each node, and parent nodes for path reconstruction.
shortest_path sssp_dj(graph& g, int64_t root) {
	auto start = std::chrono::high_resolution_clock::now();

	const float INF = std::numeric_limits<float>::infinity();
	int64_t N = g.nb_nodes;

	shortest_path sp;
	sp.distance_array = (float*)malloc(N * sizeof(float));
	sp.parent_array = (int64_t*)malloc(N * sizeof(int64_t));

	// We set up distances and parents to infinity and -1 respectively (unvisited)
	for (int64_t i = 0; i < N; i++) {
		sp.distance_array[i] = INF;
		sp.parent_array[i] = -1;
	}
	sp.distance_array[root] = 0.0F;
	sp.parent_array[root] = root;

	using pq_elem = std::pair<float, int64_t>; // (distance, node)
	std::priority_queue<pq_elem, std::vector<pq_elem>, std::greater<pq_elem>> pq;

	pq.push({0.0F, root});

	while (!pq.empty()) {
		auto [dist_u, u] = pq.top();
		pq.pop();

		// Skip if not best distance
		if (dist_u > sp.distance_array[u]) {
			continue;
		}

		for_each_neighbor(g, u, [&](int64_t neighbor, float weight) {
			float alt = dist_u + weight;
			// If the distance is less, we update the infos and add to the priority queue
			if (alt < sp.distance_array[neighbor]) {
				sp.distance_array[neighbor] = alt;
				sp.parent_array[neighbor] = u;
				pq.push({alt, neighbor});
			}
		});
	}

	auto end = std::chrono::high_resolution_clock::now();
	sp.time_ms = std::chrono::duration<double, std::milli>(end - start).count();

	// FIXME: le calcul de teps n'est pas très clair, je pense qu'il est erronné ici
	sp.teps = (double)g.length / (sp.time_ms * 1e-3);

	return sp;
}

/// @brief Shortest path with Bellman-Ford's algorithm (sequential).
/// @param g Input graph
/// @param root Starting node for SSSP
/// @return List of shortest distances from root to each node, and parent nodes for path reconstruction.
shortest_path sssp_bf(graph& g, int64_t root) {
	auto start = std::chrono::high_resolution_clock::now();

	const float INF = std::numeric_limits<float>::infinity();
	int64_t N = g.nb_nodes;

	shortest_path sp;
	sp.distance_array = (float*)malloc(N * sizeof(float));
	sp.parent_array = (int64_t*)malloc(N * sizeof(int64_t));

	float* dist_old = (float*)malloc(N * sizeof(float));
	float* dist_new = (float*)malloc(N * sizeof(float));

	// We set up distances and parents to infinity and -1 respectively (unvisited)
	for (int64_t i = 0; i < N; i++) {
		dist_old[i] = INF;
		dist_new[i] = INF;
		sp.parent_array[i] = -1;
	}
	dist_old[root] = 0.0F;
	sp.parent_array[root] = root;

	bool changed = true;

	// We iterate at most N-1 times, or until no more changes (no more improvements) are found.
	for (int64_t iter = 0; iter < N - 1 && changed; iter++) {
		changed = false;

		std::memcpy(dist_new, dist_old, N * sizeof(float));

		for (int64_t u = 0; u < N; u++) {
			float du = dist_old[u];
			if (du == INF) {
				continue;
			}

			// If we find a shorter path, we update informations.
			for_each_neighbor(g, u, [&](int64_t neighbor, float weight) {
				float alt = du + weight;
				if (alt < dist_new[neighbor]) {
					dist_new[neighbor] = alt;
					sp.parent_array[neighbor] = u;
					changed = true;
				}
			});
		}

		std::swap(dist_old, dist_new);
	}

	std::memcpy(sp.distance_array, dist_old, N * sizeof(float));

	// We free here because they're temporary buffers.
	free(dist_old);
	free(dist_new);

	auto end = std::chrono::high_resolution_clock::now();
	sp.time_ms = std::chrono::duration<double, std::milli>(end - start).count();

	// FIXME: Toujours ce calcul douteux
	sp.teps = (double)g.length / (sp.time_ms * 1e-3);

	return sp;
}

/// @brief Shortest path with Bellman-Ford's algorithm (parallel).
/// @param g Input graph
/// @param root Starting node for SSSP
/// @return List of shortest distances from root to each node, and parent nodes for path reconstruction.
shortest_path sssp_parallel(graph& g, int64_t root) {
	auto start = std::chrono::high_resolution_clock::now();

	const float INF = std::numeric_limits<float>::infinity();
	int64_t N = g.nb_nodes;

	shortest_path sp;
	sp.distance_array = (float*)malloc(N * sizeof(float));
	sp.parent_array = (int64_t*)malloc(N * sizeof(int64_t));

	float* dist_old = (float*)malloc(N * sizeof(float));
	float* dist_new = (float*)malloc(N * sizeof(float));

	// We set up distances and parents to infinity and -1 respectively (unvisited)
	for (int64_t i = 0; i < N; i++) {
		dist_old[i] = INF;
		dist_new[i] = INF;
		sp.parent_array[i] = -1;
	}

	dist_old[root] = 0.0F;
	sp.parent_array[root] = root;

	bool changed = true;

	for (int64_t iter = 0; iter < N - 1 && changed; iter++) {
		changed = false;

		// Copy old distances to new distances
		std::memcpy(dist_new, dist_old, N * sizeof(float));

		// Here we can parallelize the distance updates for each node.
#pragma omp parallel for schedule(static)
		for (int64_t u = 0; u < N; u++) {
			float du = dist_old[u];
			if (du == INF) {
				continue;
			}

			for (int64_t i = g.slicing_idx[u]; i < g.slicing_idx[u + 1]; i++) {
				int64_t v = g.neighbors[i];
				float w = g.weights[i];
				float alt = du + w;

				if (alt < dist_new[v]) {
					// Critical section in the if because we remplace infos. (should NOT write while we compare another value)
#pragma omp critical
					{
						if (alt < dist_new[v]) {
							dist_new[v] = alt;
							sp.parent_array[v] = u;
							changed = true;
						}
					}
				}
			}
		}

		// Swap buffers
		std::swap(dist_old, dist_new);
	}

	// Recopy final results
	std::memcpy(sp.distance_array, dist_old, N * sizeof(float));

	free(dist_old);
	free(dist_new);

	auto end = std::chrono::high_resolution_clock::now();
	sp.time_ms = std::chrono::duration<double, std::milli>(end - start).count();

	// FIXME
	sp.teps = (double)g.length / (sp.time_ms * 1e-3);

	return sp;
}