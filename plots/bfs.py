import matplotlib.pyplot as plt

nb_threads = [1, 2, 4, 8, 16, 32, 64]

ref = [0.0541074, 0.110687, 0.0687073, 0.0429885, 0.0481026, 0.0238852, 0.00540369]
formal = [2.718, 2.718, 2.718, 2.718, 2.718, 2.718, 2.718]
full_top_down = [2.705, 2.705, 2.705, 2.705, 2.705, 2.705, 2.705]
full_bottom_up = [6.712, 6.712, 6.712, 6.712, 6.712, 6.712, 6.712]
full_top_down_bitset = [3.435, 1.516, 0.7788, 0.57949, 0.288540, 0.21576, 0.16592]
full_bottom_up_bitset = [2.219, 1.543, 0.80820, 0.53479, 0.269789, 0.19468, 0.20786]
hybrid_paper = [0.09127, 0.09094, 0.08935, 0.08795, 0.086499, 0.079900, 0.088632]
hybrid = [0.071196, 0.052892, 0.022871, 0.015052, 0.008029, 0.007433, 0.006806]

# plt.plot(nb_threads, ref, marker='o', label='Référence')
plt.plot(nb_threads, formal, marker='o', label='Formel')
plt.plot(nb_threads, full_top_down, marker='o', label='Top-Down (std::unordered_set)')
plt.plot(nb_threads, full_bottom_up, marker='o', label='Bottom-Up (std::unordered_set)')
plt.plot(nb_threads, full_top_down_bitset, marker='o', label='Top-Down (bitset)')
plt.plot(nb_threads, full_bottom_up_bitset, marker='o', label='Bottom-Up (bitset)')
plt.plot(nb_threads, hybrid_paper, marker='o', label='Hybride (Papier)')
plt.plot(nb_threads, hybrid, marker='o', label='Hybride (Optimisé)')
plt.xlabel('Nombre de Threads')
plt.ylabel('Temps (s)')
plt.title('Comparaison des algorithmes BFS')
plt.legend()
plt.xscale('log', base=2)
plt.xticks(nb_threads, labels=nb_threads)
plt.grid(True, alpha=0.5)
plt.tight_layout()
plt.savefig("bfs_all.svg")

plt.clf()
plt.plot(nb_threads, ref, marker='o', label='Référence')
plt.plot(nb_threads, hybrid_paper, marker='o', label='Hybride (Papier)')
plt.plot(nb_threads, hybrid, marker='o', label='Hybride (Optimisé)')

plt.xlabel('Nombre de Threads')
plt.ylabel('Temps (s)')
plt.title('Zoom sur BFS Hybride vs Référence')
plt.legend()
plt.grid(True, alpha=0.5)
plt.tight_layout()
plt.xscale('log', base=2)
plt.xticks(nb_threads, labels=nb_threads)
plt.savefig("bfs_ref_hybrid.svg")
