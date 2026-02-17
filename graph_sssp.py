import matplotlib.pyplot as plt

# Données
nodes = [1, 2, 4, 8, 16, 32, 64]

# Référence Graph500 (ms)
graph500_times = [14.3, 11.8, 6.7, 3.9, 3.8, 3.9, 4.72]

# Dijkstra (constant)
dijkstra_times = [161] * len(nodes)

# Bellman-Ford (ms)
bellman_ford_times = [2218, 1542, 1091, 1065, 689, 645, 734]

# Plot
plt.figure()
plt.plot(nodes, graph500_times, marker='o', label="Graph500 (référence)")
plt.plot(nodes, dijkstra_times, marker='o', label="Dijkstra (séquentiel)")
plt.plot(nodes, bellman_ford_times, marker='o', label="Bellman-Ford")

plt.xlabel("Nombre de threads")
plt.ylabel("Temps moyen (ms)")
plt.title("SSSP – Comparaison des temps d'exécution")
plt.legend()
plt.xscale("log", base=2)
plt.xticks(nodes, labels=nodes)
plt.grid(True)

plt.savefig("sssp.png")
