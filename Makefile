CFLAGS = -Drestrict=__restrict__ -O3 -DREUSE_CSR_FOR_VALIDATION -I../aml -march=native
LDFLAGS = -lpthread
MPICC = mpicxx

all: main

GENERATOR_SOURCES = generator/graph_generator.cpp generator/make_graph.cpp generator/splittable_mrg.cpp generator/utils.cpp generator/generator.cpp
SOURCES = main.cpp
HEADERS = 

main: $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES)
	$(MPICC) $(CFLAGS) $(LDFLAGS) -o main $(SOURCES) $(GENERATOR_SOURCES) -lm