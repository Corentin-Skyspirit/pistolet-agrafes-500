#ifndef GEN_GRAPH_H
#define GEN_GRAPH_H

typedef struct {
	uint64_t* slicing_idx;
	uint64_t* neighbors;
	float* weight;
	double time_s;

	graph from_edge_list(edge_list input_list);

} graph;

#endif