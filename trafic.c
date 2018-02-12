#include "polling.h"


int main (int argc, char *argv[])
{

	if(argc!=5)
	{
		fprintf(stderr,"Usage : %s i pid_Sti delai_min_requete delai_max_requete\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int nb_aleatoire;
	int i = atoi(argv[1]);
	int pid_Sti = atoi(argv[2]);
	int delai_min_requete = atoi(argv[3]);
	int delai_max_requete = atoi(argv[4]);
	
	int etat = G_REQ;
	//Graine de sequence pseudo aléatoire i
	srand(i);
	for EVER
	{
		switch(etat)
		{
			case G_REQ : 
				nb_aleatoire = rand()%(delai_max_requete-delai_min_requete)+delai_min_requete;
				sleep(nb_aleatoire);
				//Si le kill vers le secondaire ne fonctionne pas c'est qu'il est mort donc on quitte
				if(kill(pid_Sti, SIGALRM)<0)
				{
					exit(EXIT_SUCCESS);
				}	
				break;
		}
	}
	
}
