##################################################################################
# Plots the performance of different Graph Generation (Kernel 1) implementations #
##################################################################################

import numpy as np

v2 = [1.789017, 11.7407, 0.250080]
v2_parallel = [56.44, 48.939, 0.302653]
v2_not_so_parallel = [1.81659, 4.939, 0.328631]
names_v2 = ["Mise en liste", "Trier liens", "Construction CSR"]

v3 = [23.95, 0.093, 0.000024, 46.549, 0.455]
v3_parallel = [0.4537, 0.4790, 0.000024, 1.77, 0.424]
names_v3 = ["Compter les noeuds", "Count les degrés", "Somme préfixe", "Remplir voisins", "Enlever les doublons"]

nb_threads = [1, 2, 4, 8, 16, 32, 64]

import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

# Data for plotting
data = [v2, v2_parallel,  v2_not_so_parallel, v3, v3_parallel]
labels = [
        [r"Pré-Tri: " + n for n in names_v2],
        [r"Pré-Tri (parallèle): " + n for n in names_v2],
        [r"Pré-Tri (semi-parallèle): " + n for n in names_v2],
        [r"2-pass: " + n for n in names_v3],
        [r"2-pass (parallèle): " + n for n in names_v3],
]
bar_names = ["Pré-Tri", "Pré-Tri\n(parallèle)", "Pré-Tri\n(semi-parallèle)", "2-passes", "2-passes\n(parallèle)"]

# Find the maximum number of components
max_len = max(len(d) for d in data)

# Pad data and labels to the same length
for i in range(len(data)):
        if len(data[i]) < max_len:
                data[i] += [0.0] * (max_len - len(data[i]))
                labels[i] += [""] * (max_len - len(labels[i]))

data = np.array(data)

plt.xscale('log', base=2)
plt.xticks(nb_threads, labels=nb_threads)

fig, ax = plt.subplots(figsize=(8, 6))
bar_width = 0.6
# Create x positions and add a small horizontal gap between V2 (first two)
# and V3 (last two) groups for visual separation.
indices = np.arange(len(data), dtype=float)
gap = 0.6
N_before = 3  # number of groups before the gap
if len(indices) > N_before:
        indices[N_before:] += gap

bottom = np.zeros(len(data))
colors = plt.cm.tab20.colors

for i in range(max_len):
        ax.bar(indices, data[:, i], bar_width, bottom=bottom, label=f"Step {i+1}", color=colors[i % len(colors)])
        bottom += data[:, i]

ax.set_xticks(indices)
ax.set_xticklabels(bar_names)
ax.set_ylabel("Temps (s)")
ax.set_title("Performance des algorithmes Pré-Tri et 2-passes")

# Create separate legends for Pré-Sort and 2-pass showing step colors
handles_v2 = [mpatches.Patch(color=colors[i % len(colors)]) for i in range(len(names_v2))]
handles_v3 = [mpatches.Patch(color=colors[i % len(colors)]) for i in range(len(names_v3))]

leg1 = ax.legend(handles_v2, names_v2, title="Pré-Tri", loc="upper left", fontsize="small", bbox_to_anchor=(1.02, 1))
ax.add_artist(leg1)
leg2 = ax.legend(handles_v3, names_v3, title="2-passes", loc="upper left", fontsize="small", bbox_to_anchor=(1.02, 0.45))

# Add a bit of vertical "air" above the tallest stacked bar
ymax = bottom.max() if bottom.size > 0 else 1.0
ax.set_ylim(0, ymax * 1.12)

plt.tight_layout()
plt.savefig("graph_gen.png")

nb_threads = [1, 2, 4, 8, 16, 32, 64]
ref = [0.00152132, 0.00323817, 0.00317167, 0.00288119, 0.00382385, 0.00506352, 0.00545395] 
dup_n_tri = [22.131, 12.147, 7.341, 5.074, 3.998, 4.030, 4.038]
two_passes = [2.048, 2.519, 1.885, 1.419, 1.859, 1.649, 1.495]
plt.clf()
plt.plot(nb_threads, ref, marker='o', label='Référence')
plt.plot(nb_threads, dup_n_tri, marker='o', label='Pré-Tri')
plt.plot(nb_threads, two_passes, marker='o', label='2-passes') 
plt.xlabel('Nombre de Threads')
plt.ylabel('Temps (s)')
plt.title('Comparaison des performances des algorithmes de génération de graphes')
plt.legend()
plt.xscale('log', base=2)
plt.xticks(nb_threads, labels=nb_threads)
plt.grid(True, alpha=0.5)
plt.tight_layout()
plt.savefig("graph_gen_comparison.png")