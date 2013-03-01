#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "liste_chaine.h"
#include "fonction_thread.h"


extern llist liste_client, liste_serveur, liste_ip;
extern Historique liste_historique;
extern char maj_en_cour;

void traitement_ASK_CONNECTION(int ip, short port)
{
	//on calcule dans la liste serveur et avec already connecte
	//Si bon, on envoie un ANS_CONNECTION_YES et on ajoute
	//sinon, on envoie ANS_CONNECTION_NO
	int nb = nombreElements(liste_serveur), sock = socket(AF_INET, SOCK_DGRAM, 0);
	Contacte pseudo_contacte;
	Paquet paquet;
	
	paquet.version = VERSION;
	paquet.port_usage = PORT_UDP;
	paquet.specification = 0;
	paquet.suite = 0;
	if(sock == SOCKET_ERROR)
	{
		return;
	}
	if(nb < NOMBRE_MAX_SERVEUR && already_connecte(ip) == 0)
	{
		paquet.action = ANS_CONNECTION_YES;
		
		pseudo_contacte.ip = ip;
		pseudo_contacte.deadline = time(NULL) + MAX_BEFORE_ERASE_CONTACTE;
		liste_serveur = ajouterEnTete(liste_serveur, pseudo_contacte);
	}
	else
	{
		paquet.action = ANS_CONNECTION_NO;
	}
	envoyer_udp(sock, ip, port, (char*)&paquet, sizeof(paquet));
	close(sock);
	return;
}
void traitement_ANS_CONNECTION_NO(int ip)
{
	//Ajoute a l'historique
	if(liste_historique.index >= NOMBRE_MAX_HISTORIQUE)
	{
		liste_historique.index = 0;
	}
	liste_historique.liste[liste_historique.index] = ip;
	liste_historique.index = (liste_historique.index+1) % NOMBRE_MAX_HISTORIQUE;
}
void traitement_ANS_CONNECTION_YES(int ip, short port)
{
	//On ajoute dans la liste des clients
	//On envoye VAL_CONNECTION
	int nb = nombreElements(liste_serveur), sock = socket(AF_INET, SOCK_DGRAM, 0);
	Contacte pseudo_contacte;
	Paquet paquet;
	
	paquet.version = VERSION;
	paquet.port_usage = PORT_UDP;
	paquet.specification = 0;
	paquet.suite = 0;
	
	if(sock == SOCKET_ERROR)
	{
		return;
	}
	if(nb < NOMBRE_MAX_CLIENT && already_connecte(ip) == 0)
	{
		paquet.action = VAL_CONNECTION;
		pseudo_contacte.ip = ip;
		pseudo_contacte.deadline = time(NULL) + MAX_BEFORE_ERASE_CONTACTE;
		liste_client = ajouterEnTete(liste_client, pseudo_contacte);
		envoyer_udp(sock, ip, port, (char*)&paquet, sizeof(paquet));
	}
	close(sock);
	return;
}
void traitement_VAL_CONNECTION(int ip)
{
	//RIEN pour le moment
}
void traitement_KEEP_CONTACTE(int ip)
{
	//On balaye la liste serveur et client
	//Si on trouve, on modifie sa deadline
	llist tmp;
	Contacte contacte;
	contacte.ip = ip;
	
	tmp = rechercherElement(liste_client, contacte);
	if(tmp != NULL)
	{
		tmp->val.deadline = time(NULL) + MAX_BEFORE_ERASE_CONTACTE;
		return;
	}
	
	tmp = rechercherElement(liste_serveur, contacte);
	if(tmp != NULL)
	{
		tmp->val.deadline = time(NULL) + MAX_BEFORE_ERASE_CONTACTE;
		return;
	}
}
void traitement_ASK_MAJ(int ip, short port)
{
	int sock = socket(AF_INET, SOCK_DGRAM, 0), i, erreur;
	pthread_t thread;
	Paquet paquet;

	if(maj_en_cour == 0)
	{
		paquet.action = ANS_MAJ_YES;
	}
	else
	{
		paquet.action = ANS_MAJ_NO;
	}
	paquet.version = VERSION;
	paquet.port_usage = PORT_TCP;
	paquet.specification = 0;
	paquet.suite = 0;

	if(sock == SOCKET_ERROR)
	{
		return;
	}
	envoyer_udp(sock, ip, port, (char*)&paquet, sizeof(paquet));
	closesocket(sock);	
	//lancer un thread
	if(maj_en_cour == 0)
	{
		maj_en_cour = 1;
		erreur = pthread_create(&thread, NULL, &thread_envoie_maj, NULL);
		if(erreur != 0)
		{
			printf("[-] Erreur de lancement du thread d'envoie de  maj\n");
		}
	}
}
void traitement_ANS_MAJ_YES(int ip, short port)
{
	pthread_t thread;
	int erreur;
	struct sockaddr_in *sin = malloc(sizeof(struct sockaddr_in));
	sin->sin_addr.s_addr = htonl(ip);
	sin->sin_port = htons(port);

	//on lance un thread
	erreur = pthread_create(&thread, NULL, &thread_reception_maj, (void*) sin);
	if(erreur != 0)
	{
		free(sin);
		printf("[-] Erreur de lancement du thread de reception de maj\n");
	}
}
void traitement_ANS_MAJ_NO()
{

}
void traitement_GIVE_CONTACTE(LargePaquet l_paquet)
{
	int sock = socket(AF_INET, SOCK_DGRAM, 0), i;
	Paquet paquet;
	
	paquet.version = VERSION;
	paquet.port_usage = PORT_UDP;
	paquet.specification = 0;
	paquet.suite = 0;
	
	if(sock == SOCKET_ERROR)
	{
		return;
	}
	
	for(i = 0; i < NOMBRE_MAX_IP_BY_LARGEPAQUET; i++)
	{
		if(l_paquet.ip[i] != 0)
		{
			envoyer_udp(sock, l_paquet.ip[i], PORT_UDP, (char*)&paquet, sizeof(paquet));
		}
	}
	close(sock);
	return;
}
