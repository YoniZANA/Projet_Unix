poll_config : poll_config.c secondaire.c trafic.c primaire.c

	gcc primaire.c -o primaire
	
	gcc secondaire.c -o secondaire
	
	gcc trafic.c -o trafic
	
	gcc poll_config.c -o poll_config
	
	@echo "Veuillez lancer poll_config en respectant la syntaxe suivante :\n./poll_config primaire secondaire trafic n nb_polling delai_polling delai_min_requette delai_max_requette [prefixe_fichier]\nPensez aussi a ouvrir le terminal sur grand ecran pour avoir un affichage optimal"