#include "shortest_path.hpp"
#include <assert.h>
#include <chrono>
#include <limits>
#include <omp.h>
#include <queue>

void shortest_path_destroy(shortest_path& sp) {
	free(sp.distance_array);
	free(sp.parent_array);
}

shortest_path sssp(graph g, int64_t root) {
	auto start = std::chrono::high_resolution_clock::now();

	const float INF = std::numeric_limits<float>::infinity();
	int64_t N = g.nb_nodes;

	shortest_path sp;
	sp.distance_array = (float*)malloc(N * sizeof(float));
	sp.parent_array = (int64_t*)malloc(N * sizeof(int64_t));

	for (int64_t i = 0; i < N; i++) {
		sp.distance_array[i] = INF;
		sp.parent_array[i] = -1;
	}

	sp.distance_array[root] = 0.0f;
	sp.parent_array[root] = root;

	// (distance, node)
	using pq_elem = std::pair<float, int64_t>;
	std::priority_queue<pq_elem, std::vector<pq_elem>, std::greater<pq_elem>> pq;

	pq.push({0.0f, root});

	while (!pq.empty()) {
		auto [dist_u, u] = pq.top();
		pq.pop();

		// skip si pas la meilleure distance
		if (dist_u > sp.distance_array[u])
			continue;

		for (int64_t i = g.slicing_idx[u]; i < g.slicing_idx[u + 1]; i++) {
			int64_t v = g.neighbors[i];
			float w = g.weights[i];

			float alt = dist_u + w;
			if (alt < sp.distance_array[v]) {
				sp.distance_array[v] = alt;
				sp.parent_array[v] = u;
				pq.push({alt, v});
			}
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	sp.time_ms = std::chrono::duration<double, std::milli>(end - start).count();

	sp.teps = (double)g.length / (sp.time_ms * 1e-3);

	for (int64_t i = 0; i < g.nb_nodes; i++) {
		if (i == root)
			continue;
		assert(sp.parent_array[i] != -1);
		assert(sp.distance_array[i] >= 0.0f);
	}

	return sp;
}

shortest_path sssp_parallel(graph g, int64_t root) {
	auto start = std::chrono::high_resolution_clock::now();

	const float INF = std::numeric_limits<float>::infinity();
	int64_t N = g.nb_nodes;

	shortest_path sp;
	sp.distance_array = (float*)malloc(N * sizeof(float));
	sp.parent_array = (int64_t*)malloc(N * sizeof(int64_t));

	for (int64_t i = 0; i < N; i++) {
		sp.distance_array[i] = INF;
		sp.parent_array[i] = -1;
	}

	sp.distance_array[root] = 0.0f;
	sp.parent_array[root] = root;

	using pq_elem = std::pair<float, int64_t>;
	std::priority_queue<pq_elem, std::vector<pq_elem>, std::greater<pq_elem>> pq;
	pq.push({0.0f, root});

	while (!pq.empty()) {
		auto [dist_u, u] = pq.top();
		pq.pop();

		if (dist_u > sp.distance_array[u])
			continue;

		int64_t begin = g.slicing_idx[u];
		int64_t end = g.slicing_idx[u + 1];

// 🔹 relaxation parallèle des voisins
#pragma omp parallel for schedule(static)
		for (int64_t i = begin; i < end; i++) {
			int64_t v = g.neighbors[i];
			float w = g.weights[i];
			float alt = dist_u + w;

			if (alt < sp.distance_array[v]) {
#pragma omp critical
				{
					if (alt < sp.distance_array[v]) {
						sp.distance_array[v] = alt;
						sp.parent_array[v] = u;
						pq.push({alt, v});
					}
				}
			}
		}
	}

	auto end_t = std::chrono::high_resolution_clock::now();
	sp.time_ms = std::chrono::duration<double, std::milli>(end_t - start).count();

	sp.teps = (double)g.length / (sp.time_ms * 1e-3);

	return sp;

	for (int64_t i = 0; i < g.nb_nodes; i++) {
		if (i == root)
			continue;
		assert(sp.parent_array[i] != -1);
		assert(sp.distance_array[i] >= 0.0f);
	}
}