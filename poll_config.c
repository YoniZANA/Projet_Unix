#include "polling.h"

 
#define NBMAX 15

//Autant de fichier que de potentiels secondaires
//l'indice 0 pour le primaire et les autres pour les secondaires
int fichiers[6];

void poll_config(int argc, int nb_fichiers)
{
	wait(NULL);
	int k;
	//Fermeture de tous les fichiers une fois sorti de primaire
	if (argc==10)
	{
		for (k = 0; k < nb_fichiers; k++)
		{
	 		close(fichiers[k]);
		}
	}
	//Pour ne pas laisser le  terminal reprendre le dessus
	sleep(3);
	exit(EXIT_SUCCESS);
}

void creation_fichier(char *prefixe, char *suffixe, int indice_fichier)
{
	
	char nom_du_fichier[30];
	strcat(strcpy(nom_du_fichier,prefixe),suffixe);
	nom_du_fichier[strlen(nom_du_fichier)]='\0';
		
	fichiers[indice_fichier] = creat(nom_du_fichier, 0777 );
	
	if (fichiers[indice_fichier]<0)
	{
		perror("Erreur creation fichier\n");
		exit(EXIT_FAILURE);
	}
	
	close (STDOUT_FILENO);

	if(dup2(fichiers[indice_fichier],STDOUT_FILENO)<0)
	{
		perror("Erreur dup2\n");
		exit(EXIT_FAILURE);
	}

}

void trafic (char **argv, char *string_i, int i)
{
	char string_pid_Sti[NBMAX];
	sprintf(string_pid_Sti,"%d",getppid()); 
	/*Choix du exec prenant les parametres en liste et non en tableau car le nombre d'arguments est invariable*/
	/*Remplacement du processus courant par celui du trafic et verification du bon déroulement*/
	if(execl(argv[3],argv[3],string_i,string_pid_Sti,argv[7],argv[8],NULL) == -1)
	{
		fprintf(stderr,"erreur lors du exec du trafic de %d", i+1);
		exit(EXIT_FAILURE);
	}
}
void secondaire (char **argv, int argc, int i)
{
	char string_ppid[NBMAX];
	char string_i[NBMAX];
	sprintf(string_i,"%d",i+1); 


	int pid_trafic;

	/*Vérification du bon déroulement de secondaire i pour créer son trafic*/
	if((pid_trafic=fork())==-1)
	{
		fprintf(stderr, "erreur lors du fork du secondaire %d", i+1);
		exit(EXIT_FAILURE);
	}

	/*Entrée dans secondaire*/
	else if(pid_trafic!=0)
	{
		/*Transformation du pid de primaire en chaine de caractere pour pouvoir l'utiliser dans exec*/
		sprintf(string_ppid, "%d",getppid());
		
		sleep(1);

		/*Remplacement du processus courant par celui du secondaire et verification du bon déroulement*/
		if (argc == 10)
		{
			char suff_sec[10];
			strcat(strcpy(suff_sec,"_St"),string_i);
			creation_fichier(argv[9],suff_sec,i+1);
		}

		if (execl(argv[2],argv[2],string_i,string_ppid,NULL) == -1)
		{
			fprintf(stderr,"erreur lors du exec du secondaire %d",i+1);
	   		exit(EXIT_FAILURE);
		}
	}
 	/*Entrée dans trafic associé au secondaire précédent*/
	else
	{
		trafic(argv, string_i, i);

	}
}

void primaire (char **argv, int argc, int nb_secondaire)
{
	int i, j;
	int pid_Sti[nb_secondaire];


	/*Boucle permettant de créer tout les processus secondaire*/
	for(i=0;i<nb_secondaire;i++)
	{
		/*Vérification du bon déroulement du fork de primaire pour créer le secondaire i*/			
		if((pid_Sti[i]=fork())<0)
		{
			fprintf(stderr, "erreur lors du fork du primaire pour créer le secondaire %d",i+1 );
			exit(EXIT_FAILURE);
		}
		
		/*Entrée dans primaire*/
		else if(pid_Sti[i]!=0)
		{
				
		}
		

		/*Entrée dans secondaire*/
		else    					
		{
			secondaire (argv, argc, i);
			
		}		
	}
	sleep(3);
	fflush(stdout);

	/*Creation d'un tablau de pointeurs sur chaine de caracteres pour pouvoir stocker les arguments necessaires 
	a l'execution du code de primaire*/
	/*allocation d'espace necessaire pour stocker les chaines*/
	char **argv_exec;
	argv_exec = (char**)malloc(10 * sizeof(char*));
	/*allocation de chaque case du tableau qui va contenir les chaines de caracteres*/
	for(i=0;i<4+nb_secondaire;i++)
	/*4+nb_secondaire c'est le nombre de case maximum dont on aura besoin pour stocker les differents éléments dans le tableau, 
	4 pour les 4 suivant et nb_secondaire pour le nombre de pid_Sti*/
		argv_exec[i]= (char*)malloc(10*sizeof(char));

	/*Recuperation des arguments passés dans poll_config et necessaires a l'execution de primaire*/
	strcpy(argv_exec[0],argv[1]); //nom du programme
	strcpy(argv_exec[1],argv[5]); //nb_polling
	strcpy(argv_exec[2],argv[6]); //delai_polling
	strcpy(argv_exec[3],argv[4]); //n
	
	/*Utilisation d'une boucle for car le nombre de stations secondaires peut varier de 1 a 5 pour stocker les pid de tout les secondaires*/
	for (j = 0; j < nb_secondaire; j++)
	{
		sprintf(argv_exec[(4+j)],"%d",pid_Sti[j]);
	}


	/*Pour inserer un cran d'arrêt dans le tableau que l'on va faire passer en parametre dans le execv*/ 
	argv_exec[nb_secondaire+4]=NULL;

	if(argc == 10)
	{
		creation_fichier(argv[9],"_prim",0);
	}	

	/*Choix du exec prenant les parametres en tableaux et non en liste car le nombre d'arguments peut varier*/
	/*Remplacement du processus courant par celui du trafic et verification du bon déroulement*/
	if (execv(argv_exec[0],argv_exec) == -1)
	{
		perror("erreur lors du exec du primaire");
		exit(EXIT_FAILURE);
	}
}

void installation_gestionnaire()
{
	int i;
	
	int signaux[NB_SIGNAUX]={SIGUSR1, SIGUSR2, SIGALRM, SIGPIPE};

	sigset_t new_mask;

	struct sigaction action;

	//On ignore tous les signaux reçus car on ne va les traiter que dans les automates
	//On l'installe ici pour éviter la mort des processus avant l'installation de leurs
	//propres gestonnaires
	action.sa_handler = SIG_IGN;

	sigemptyset(&new_mask);

	//On modifie l'action associé à chaque signal
	for(i=0; i<NB_SIGNAUX; i++)
	{
		if(sigaction(signaux[i], &action, NULL))
		{
			perror("pb signal SIGUSR1");
			exit(EXIT_FAILURE);
		}
	}

  
	//Ajout de tous les signaux au masque des signaux bloqués
	sigemptyset(&new_mask);
	for(i=0; i<NB_SIGNAUX; i++)
	{
		//Ajoute les signaux dans new_mask
		sigaddset(&new_mask, signaux[i]);
	}
}

int main (int argc, char *argv[])
{
	system("clear");

	/*Verification du nombre d'argument passé en paramètre*/
	if(argc<9 || argc>10)
	{
		fprintf(stderr,"Usage : %s primaire secondaire trafic n nb_polling delai_polling delai_min_requette delai_max_requette [prefixe_fichier]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int nb_secondaire = atoi(argv[4]);
    
	/*Nombre de processus que peut créer le primaire*/
	if(nb_secondaire > MAX_ST)
		nb_secondaire = MAX_ST;

	installation_gestionnaire();

	int pid_primaire;

	/*Création du processus primaire*/
	/*Vérification du bon déroulement du fork de primaire*/
	if((pid_primaire = fork())<0)
	{
		perror("erreur lors du fork pour le primaire");
		exit(EXIT_FAILURE);
	}
	
	
	/*Entrée dans poll_config*/
	else if(pid_primaire>0)
	{
		poll_config(argc,nb_secondaire+1);
	}
	
	/*Entrée dans primaire*/
	else                    
 	{
 		primaire (argv, argc, nb_secondaire);
 	}

} 
