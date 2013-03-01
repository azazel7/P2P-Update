#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "src/config.h"
#include "src/fonction_thread.h"
#include "src/divers.h"
#include "src/liste_chaine.h"
#include "src/reseau.h"
#include "src/chiffrement.h"

#include "src/camellia.h"

llist liste_client = NULL, liste_serveur = NULL, liste_ip = NULL;
Historique liste_historique;
char* chemin_maj = NULL, *chemin_executable = NULL;
char maj_en_cour = 0;
//getifaddrs
/*
   Pour le moment, 
 */
int main(int  argc, char** argv)
{
	printf("[i] Demarrage version: %d\n", VERSION);
	chemin_maj = "narval.txt";
	printf("[i] Chemin de maj: %s\n", chemin_maj);
	chemin_executable = argv[0];
	printf("[i] Chemin de l'executable: %s\n", chemin_executable);
	if(argc == 3)
	{
		Paquet paquet;
		paquet.action = atoi(argv[2]);
		paquet.port_usage = PORT_UDP;
		paquet.specification = 0;
		paquet.suite = 0;
		paquet.version = VERSION;
		int sock = socket(AF_INET, SOCK_DGRAM, 0);
		envoyer_udp(sock, htonl(inet_addr(argv[1])), PORT_UDP, (char*)&paquet, sizeof(paquet)); 
	}
	else
	{
		signal(SIGALRM, &handle_sigalrm);
		recuperer_ip();
		alarm(MAX_BEFORE_ERASE_CONTACTE);
		thread_reception_data(NULL);
	}

	
	printf("[i] Fin version: %d\n", VERSION);
	return 0;
}
