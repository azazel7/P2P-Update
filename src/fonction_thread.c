#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "reseau.h"
#include "traitement.h"
#include "liste_chaine.h"
#include "divers.h"
#include "chiffrement.h"

extern llist liste_client, liste_serveur, liste_ip;
extern Historique liste_historique;
extern char* chemin_maj, *chemin_executable;
extern char maj_en_cour;

void* thread_search_reseau(void* data)
{
	Reseau reseau = *((Reseau*)data);
	unsigned int nb = (-1) - reseau.netmask;
	unsigned int i;
	int erreur;

	printf("[i] Thread sur %s\n", afficher_ip(reseau.reseau));

	Paquet paquet;
	paquet.action = ASK_CONNECTION;
	paquet.port_usage = PORT_UDP;
	paquet.version = VERSION;
	paquet.specification = 0;

	for(i = 1; i < nb; i++)
	{
		//pour chaque machine, l'adresse étant reseau.reseau + i;
		if(reseau.reseau + i != reseau.ip)
		{
			erreur = envoyer_udp(reseau.sock, reseau.reseau + i, PORT_UDP, (char*) &paquet, sizeof(Paquet));
		}
	}
	printf("[i] Fin thread sur %s\n", afficher_ip(reseau.reseau));
	free(data);
	return NULL;
}


void* thread_reception_data(void* data)
{
	//se mettre en ecoute sur 0.0.0.0
	struct sockaddr_in sin;
	int sock = socket(AF_INET, SOCK_DGRAM, 0), sock_envoie;
	int tmp, taille, ip;
	short port;
	LargePaquet paquet;
	if(sock == SOCKET_ERROR)
	{
		printf("[-] Erreur socket\n");
		return NULL;
	}

	sin.sin_addr.s_addr = inet_addr("0.0.0.0");
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT_UDP);

	tmp = bind(sock, (struct sockaddr*)&sin, sizeof(sin)); //On associe l'adresse au socket
	if(tmp == SOCKET_ERROR)
	{
		perror("[-] bind");
		return;
	}
	//On ecoute en permanence
	while(1)
	{
		taille = sizeof(sin);
		tmp = recvfrom(sock, (char*) &paquet, sizeof(Paquet), 0, (struct sockaddr*)&sin, &taille);
		if(tmp == SOCKET_ERROR)
		{
			perror("[-] recvfrom");
			continue;
		}

		ip = htonl(sin.sin_addr.s_addr);
		port = paquet.paquet.port_usage;
		//On check l'action et on fait une action
		switch(paquet.paquet.action)
		{
			case ASK_CONNECTION:
				printf("[i] reception ASK_CONNECTION\n");
				traitement_ASK_CONNECTION(ip, port);
				break;
			case ANS_CONNECTION_NO:
				printf("[i] reception ANS_CONNECTION_NO\n");
				traitement_ANS_CONNECTION_NO(ip);
				break;
			case ANS_CONNECTION_YES:
				printf("[i] reception ANS_CONNECTION_YES\n");
				traitement_ANS_CONNECTION_YES(ip, port);
				break;
			case VAL_CONNECTION:
				printf("[i] reception VAL_CONNECTION\n");
				traitement_VAL_CONNECTION(ip);
				break;
			case KEEP_CONTACTE:
				printf("[i] reception KEEP_CONTACTE\n");
				traitement_KEEP_CONTACTE(ip);
				break;
			case ASK_MAJ:
				printf("[i] reception ASK_MAJ\n");
				traitement_ASK_MAJ(ip, port);
				break;
			case ANS_MAJ_NO:
				printf("[i] reception ANS_MAJ_NO\n");
				traitement_ANS_MAJ_NO();
				break;
			case ANS_MAJ_YES:
				printf("[i] reception ANS_MAJ_YES\n");
				traitement_ANS_MAJ_YES(ip, port);
				break;
			case GIVE_CONTACTE:
				printf("[i] reception GIVE_CONTACTE\n");
				traitement_GIVE_CONTACTE(paquet);
				break;
		}
		printf("[i] Serveur: %d\tClient: %d\n", nombreElements(liste_serveur), nombreElements(liste_client));
		//verifie la version
		if(paquet.paquet.version > VERSION)
		{
			printf("[+] La version %d semble etre disponible\n", paquet.paquet.version);
			demander_maj(ip, port);
		}
	} 

	return NULL;
}

void* thread_reception_maj(void* data)
{
	struct sockaddr_in *sin = (struct sockaddr_in*)data;	
	unsigned int ip = htonl(sin->sin_addr.s_addr);
	int port = htons(sin->sin_port);	
	int erreur, taille_maj;
	unsigned char *maj = NULL, *maj_claire = NULL;
	unsigned char clee_camelia[TAILLE_CLEE_CAMELIA] = {0}, md5[16] = {0}, IV[16] = {0};

	erreur = recevoir_maj_tcp(ip, port, &maj, &taille_maj);
	if(erreur == SOCKET_ERROR || maj == NULL)
	{
		printf("[-] Erreur de reception de la maj\n");
		return NULL;
	}
	printf("[i] Mise a jour recus : %d octets\n", taille_maj);
	//traitement de la mise a jour
	//On commence par recuperer la clee camelia
	//Checker si c'est une bonne mise à jour
	erreur = remplir_fichier(chemin_maj, maj, taille_maj);
	maj_claire = protocole_dechiffrement(maj, taille_maj);	
	if(maj_claire == NULL)
	{
		printf("[-] Erreur de dechiffrement de la maj\n");
		return NULL;
	}
	//accessoirement, sauvegarder les contactes
	//executer une mise a jour
	erreur = remplir_fichier(chemin_maj, maj, taille_maj);
	free(maj);
	if(erreur == SUCCES)
	{
		printf("[i] Stockage de la maj a : %s\n", chemin_maj);
	}
	else
	{
		printf("[-] Erreur lors du stockage de la maj\n");
	}
	unlink(chemin_executable);
	perror("unlink");
	erreur = remplir_fichier(chemin_executable, maj_claire, taille_maj - (TAILLE_CLEE_RSA/8));
	free(maj_claire);
	if(erreur == SUCCES)
	{
		executer_maj(chemin_executable);
	}
	else
	{
		printf("[-] Erreur de creation du fichier de maj\n");
	}
}

void* thread_envoie_maj(void* data)
{
	char* maj;
	int taille = 0, erreur;
	//charger la maj	
	maj = charger_fichier(chemin_maj, &taille);
	if(maj == NULL)
	{
		return;
	}
	printf("[i] Maj charge : %d octets\n", taille);

	erreur = envoyer_maj_tcp(PORT_TCP, maj, taille);
	free(maj);
	if(erreur == SOCKET_ERROR)
	{
		printf("[-] Erreur de transmission de mise a jour\n");
	}
	maj_en_cour = 0;
	return NULL;
}
