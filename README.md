# Projet_Unix

Il s'agit d'un projet qui va simuler un Lan en étoile.

Une station "primaire" va inviter chaque station secondaires tour à tour à emettre des données à diffuser à toute les autres stations.
Si une station n'a rien à emettre alors la station primaire va intéragir avec la suivante sinon elle va diffuser le message que la station lui aura envoyer.

Pour mieux comprendre le fonctionnement de ce programme il faut lire le pdf inclus dans le projet.

Pour éxécuter le programme il faut faire se placer dans le dossier contenant le fichier makefile et éxecuter la commande "make" avec le terminal. 
Ensuite, il faut executer "./primaire secondaire trafic n nb_polling delai_polling delai_min_requette delai_max_requette [prefixe_fichier]", avec "n" le nombre de stations secondaire désirée, "nb_polling" le nombre de tour que la station primaire devra effectuer, "delai_polling" le temps que la station primaire va accorder à chaque station secondaire pour emmetre sa donnée, "delai_min_requette" et "delai_max_requette" pour les requête que devront envoyer chaque secondaire et le "prefix_fichier" pour le nom du fichier si l'utilisateur désire enregistrer les données dans un fichier.

