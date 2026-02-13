#include "shortest_path.hpp"
#include <chrono>
#include <limits>
#include <queue>

shortest_path sssp(graph g, int64_t root) {
	auto start = std::chrono::high_resolution_clock::now();

	shortest_path sp;
	int64_t N = g.nb_nodes;

	sp.distance_array = (float*)malloc(N * sizeof(float));
	sp.parent_array = (int64_t*)malloc(N * sizeof(int64_t));

	const float INF = std::numeric_limits<float>::infinity();

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

		for (int64_t i = g.slicing_idx[u]; i < g.slicing_idx[u + 1]; i++) {
			int64_t v = g.neighbors[i];
			float w = g.weights[i];

			float alt = sp.distance_array[u] + w;
			if (alt < sp.distance_array[v]) {
				sp.distance_array[v] = alt;
				sp.parent_array[v] = u;
				pq.push({alt, v});
			}
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	sp.time_ms = std::chrono::duration<double, std::milli>(end - start).count();
	sp.teps = (double)g.length / (sp.time_ms * 1e-3); // edges per second

	return sp;
}