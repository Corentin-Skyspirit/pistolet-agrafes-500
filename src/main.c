#include "../generator/generator.h"
#include <stdio.h>

int main() {
	printf("Hello world\n");
	edge_list list = generate_graph(3, 5);
	print_edge_list(&list);
	edge_list_destroy(list);
}