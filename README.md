# pistolet-agrafes-500

Rendu de projet de l'UE MPNA du Master CHPS Paris-Saclay.
Ré-implémentation des algorithmes de benchmark de Graph500.

# Génération des données

La génération des données se fait par les fonctions dans le sous-dossier `generator`, le code a été copié directement de la référence de graph500 pour gagner du temps.
Il n'y a besoin que d'inclure `generator/generator.h` qui renvoie une `edge_list` contenant liens et poids.

# Kernels

Les 3 kernels implémentés sont:
- gen_graph
- breadth_first_search
- shortest_path

Et se trouvent tous dans le sous-dossier `kernels`

# Main

Le main contient le lancement et les comparaisons des différents kernels implémentés, avec des tailles de graphes fixes, et print tous les résultats sur la sortie standard.

# Lancement

Pour ROMEO:
- Lancez juste `sbatch launch_kernels.sh`, il s'occupe automatiquement de charger les bibliothèques utilisées, de compiler le code, et de le lancer pour un nombre de threads de 1 à 64, par paliers de puissances de 2.

Sinon:
- Il faut TBB (Intel Threads Building Blocks).
- Compilez avec `make`.
- Lancez avec OMP_NUM_THREADS=x ./main
- À noter que les kernels séquentiels ne seront lancés que lorsqu'il n'y a qu'un thread, et les parallèles que lorsqu'il y en a plusieurs.

# Plots

Les scripts des plots pour le rapport sont dans le sous-dossier `plots`.
Les résultats sont écrits directement dans le fichier lui-même par soucis de simplification du code.

Il y a besoin de Python3 et de la bibliothèque matplotlib.

# Notes 

Il y a beaucoup de copiés-collés dans le main et dans les kernels car le génie logiciel derrière n'est pas le but principal.
Certains des scripts des plots ont été largement générés par IA.