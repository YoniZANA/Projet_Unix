#include "polling.h"

//Pour que le gestionnaire ne le traite pas avant qu'on soit dans l'etat W_REQ
int nb_req_rx=-1;
char *string_state;
int state;

void reception_signal(int nom_signal)
{
	if (nb_req_rx == 0)
		nb_req_rx++;
		
}

void Init()
{
	string_state = (char*)malloc(20*sizeof(char));
	state = W_REQ;
	string_state = string_w_req;
	
}

int main (int argc, char *argv[])
{
	if(argc<5 || argc>10)
	{
		fprintf(stderr,"Usage : %s nb_polling delai_polling n pid_st1 ... pid_stn\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	Init();
	int nb_polling = atoi(argv[1]);
	int delai_polling = atoi(argv[2]);
	int n = atoi(argv[3]);
	int pid_st[n];
	
	
	int nb_polling_execute=0;
	int i, j;

	for(i=0;i<n;i++)
	{
		pid_st[i]=atoi(argv[i+4]);
	}

	sigset_t old_mask, new_mask;

	struct sigaction action;

	action.sa_handler = reception_signal;
	sigemptyset(&action.sa_mask);

	//Installation du gestionnaire pour DATA_RX seulement
	if(sigaction(DATA_RX, &action, NULL)<0)
	{
		perror("Pbm primaire sigaction");
		exit(EXIT_FAILURE);
	}

	sigemptyset(&new_mask);
	sigaddset(&new_mask, DATA_RX);

	//Pour faire le nombre de polling spécifié
	while (nb_polling_execute<nb_polling)
	{
		//Pour faire le tour des automates
		for(i=0; i<n; i++)
		{
			switch(state)
			{
				case W_REQ : 
					
					printf("Prim %s St%d %ds\n", string_state, i+1, delai_polling);
					
					//Bloque les signaux
					sigprocmask(SIG_SETMASK, &new_mask, &old_mask);
					
					//Envoi un signal au secondaire
					//Quand on va rentrer dans le gestionnaire la variable va être incrémenter
					nb_req_rx=0;
					
					//Envoi d'un signal poll vers le secondaire i
					if(kill(pid_st[i],POLL_TX)<0)
					{
						fprintf(stderr, "Erreur lors de l'envoi de POLL_TX vers %d", i);
						exit(EXIT_FAILURE);	;
					}

					//On dort pdt delai_polling
					sleep(delai_polling);
					
					//On debloque les signaux
					sigprocmask(SIG_SETMASK, &old_mask, 0);
					
					//Si signal DATA recu alors il a été incrémenter dans le gestionnaire
					if (nb_req_rx == 0)
					{
						printf("Prim %s St%d No_Data\n", string_state, i+1);
						break;
					}
					
					else if (nb_req_rx)
					{
						printf("Prim %s St%d Data_Rx\n", string_state, i+1);
						nb_req_rx=-1;
						string_state = string_bc_data;
						state=BC_DATA;
					}

				case BC_DATA : 
					
					printf("Prim %s St%d \n", string_state, i+1);
					
					//Diffusion de la donnée à tous les autres secondaire
					for(j=0;j<n;j++)
					{
						if(j!=i)
						{
							if(kill(pid_st[j], DATA_TX)<0)
							{
								fprintf(stderr, "Erreur lors de l'envoi de DATA_TX vers %d", i);
								exit(EXIT_FAILURE);
							}

						}
					}
					
					//Envoi d'un ACK_TX à la station secondaire i
					if(kill(pid_st[i],ACK_TX)<0)
					{
						fprintf(stderr, "Erreur lors de l'envoi de ACK_TX vers %d", i);
						exit(EXIT_FAILURE);
					}
					//Pour laisser le temps à un secondaire de recevoir DATA_RX et après recevoir POLL_TX
					sleep(0.5);
					
					state = W_REQ;
					string_state = string_w_req;
					break;
			}
		}
		nb_polling_execute++;
	}
	//Pour laisser aux secondaire le temps d'afficher leurs DATA_RX 
	sleep(1);
	exit(EXIT_SUCCESS);
	
}
