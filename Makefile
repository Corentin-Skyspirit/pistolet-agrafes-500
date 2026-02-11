CFLAGS = -Drestrict=__restrict__ -O3 -DREUSE_CSR_FOR_VALIDATION -I../aml -march=native
LDFLAGS = -lpthread
MPICC = mpicc

all: main

GENERATOR_SOURCES = generator/graph_generator.c generator/make_graph.c generator/splittable_mrg.c generator/utils.c generator/generator.c
SOURCES = src/main.c
HEADERS = 

main: $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES)
	$(MPICC) $(CFLAGS) $(LDFLAGS) -o main $(SOURCES) $(GENERATOR_SOURCES) -lm