#include "polling.h"


int nb_data_req_rx;
int state;
char *string_state;
//A cause de reception
int numero_station;
char tabulation[18];
//En variable globale car necessaire pour le gestionnaire
int entre_w_poll;
//Pour controler le data_req_rx et l'empécher d'interferer au mileu d'un affichage
//On l'initialise à 0 avant chaque affichage
int bool_data_req_rx;


void reception_signal(int nom_signal)
{
	switch(nom_signal)
	{
		case POLL_RX :
			bool_data_req_rx=0;
			//Ce signal n'a d'incidence que si l'on est dans l'état W_POLL 
			if(state == W_POLL && entre_w_poll)
			{
				printf("%sSt%d %s Poll_Rx\n",tabulation, numero_station, string_state);
				string_state = string_sd_data;
				state = SD_DATA;
			}
			
			break;

		case DATA_RX :
			//Ce signal n'a d'incidence que dans l'état IDLE ou W_POLL
			if(state == IDLE || ((state == W_POLL) && (string_state == string_w_poll)))
			{
				printf("%sSt%d %s Data_Rx\n",tabulation, numero_station, string_state);
			}
			break;
		
		case DATA_REQ_RX : 
			if(bool_data_req_rx)
			{	
				bool_data_req_rx=0;
				nb_data_req_rx++;
				printf("%sSt%d %s Data_Req_Rx %d\n",tabulation, numero_station, string_state, nb_data_req_rx);
				
				//Ce signal permet de faire changer d'état seulement IDLE
				if(state == IDLE)
				{
					string_state = string_w_poll;
					state = W_POLL;
				}
			}
			break;
		
		case ACK_RX : 
			//Signal intercepté que dans W_ACK
			if(state == W_ACK)
			{
				bool_data_req_rx=0;
				nb_data_req_rx--;
				 
				printf("%sSt%d %s Ack_Rx %d\n",tabulation, numero_station, string_state, nb_data_req_rx);
				if(nb_data_req_rx>0)
				{
					string_state=string_w_poll;
					state=W_POLL;
				}

				else if (nb_data_req_rx==0)
				{
					string_state=string_idle;
					state=IDLE;
				}	

			}
	
			break;
		
	}
	
	return;
}// reception_signal

//Initialisation des variables globales
void Init()
{
	int i;
	for (i = 0; i < 3*numero_station+1; i++)
	{
		tabulation[i]='\t';
	}
	tabulation[i]='\0';
	nb_data_req_rx=0;
	state=IDLE;
	string_state = (char*)malloc(20*sizeof(char));
	string_state = string_idle;
	bool_data_req_rx=0;

}



int main (int argc, char *argv[])
{

	if(argc!=3)
	{
		fprintf(stderr,"Usage : %s numero_station pid_primaire\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	numero_station = atoi(argv[1]);
	int pid_primaire = atoi(argv[2]);

	//automate secondaire

	Init();
	int i;

	//Pour ne pas que l'on réaffiche l'entrée dans un état on crée une variable pour chaque
	//état, que l'on incrémente lorsqu'on rentre dans cet état et que l'on met à 0 lorsqu'on
	//change d'état
	int entre_idle = 0;
	//Deja créé en variable global
	entre_w_poll = 0;
	int entre_sd_data = 0;
	int entre_w_ack = 0;

	int signaux[NB_SIGNAUX]={SIGUSR1, SIGUSR2, SIGALRM, SIGPIPE};

	sigset_t new_mask, old_mask;

	//Pour tous les signaux
	struct sigaction action;

	//Installation du gestionnaire pour tous les signaux
	action.sa_handler = reception_signal;

	action.sa_flags = SA_RESTART;

	sigemptyset(&new_mask);

	//On modifie l'action associé à chaque signal
	for(i=0; i<NB_SIGNAUX; i++)
	{
		if(sigaction(signaux[i], &action, NULL))
		{
			perror("pb signal lors de l'installation du gestionnaire");
			exit(EXIT_FAILURE);
		}
	}

	for EVER
	{
		//Si erreur du kill alors primaire est terminé donc on sort du secondaire
		if(kill(pid_primaire, 0)<0)
		{
			exit(EXIT_SUCCESS);
		}

		switch(state)
		{
			case IDLE :
				
				//Si l'on rentre depuis w_ack
				entre_w_ack = 0;
				
				if(!entre_idle)
					printf("%sSt%d %s\n",tabulation, numero_station, string_state);
				
				bool_data_req_rx=1;
				entre_idle ++;
				
				//attente d'un signal
				sigsuspend(&old_mask);

				break;

			case W_POLL : 
								
				if(!entre_w_poll)
					printf("%sSt%d %s Attente\n",tabulation, numero_station, string_state);
				
				bool_data_req_rx=1;
				//si l'on rentre depuis idle ou w_ack
				entre_idle=0;
				entre_w_ack=0;
				entre_w_poll++;

				//attente signal
				sigsuspend(&old_mask);

				break;

			case SD_DATA : 
				
				//si l'on rentre depuis w_poll
				entre_w_poll=0;
				
				if(!entre_sd_data)
					printf("%sSt%d %s\n", tabulation, numero_station, string_state);
				bool_data_req_rx=1;
				entre_sd_data ++;

				//envoi de DATA_TX a primaire
				//Arret de secondaire si primaire mort
				if(kill(pid_primaire, DATA_TX)<0)
				{
					perror("primaire est mort\n");
					exit(EXIT_SUCCESS);
				}

				//On change d'état dès que l'on rentre dans SD_DATA
				state = W_ACK;
				string_state = string_w_ack;
				
				break;

			case W_ACK : 
				
				//Si l'on rentre depuis SD_DATA
				entre_sd_data = 0;

				if(!entre_w_ack)
					printf("%sSt%d %s\n",tabulation, numero_station, string_state);
				
				bool_data_req_rx=1;

				entre_w_ack++;

				sigsuspend(&old_mask);
				
				break;
		}
	}
}
