#ifndef SHORTEST_PATH_H
#define SHORTEST_PATH_H

typedef struct {
	int64_t* parent_array;
	float* distance_array;
	double teps;

} shortest_path;

shortest_path sssp(graph source_graph);

#endif