/*
   Contient toutes les fonction necessaire au connexion a a l'envoie de message par le réseau
 */
#include <stdlib.h>
#include <ifaddrs.h>
#include "config.h"
#include "fonction_thread.h"
/*
   Traceroute pour découvrir des réseaux

 */

/*
	Envoie un paquet UDP
*/
int envoyer_udp(int sock, long unsigned int ip, int port, char *paquet, int taille)
{
	struct sockaddr_in sin;
	sin.sin_addr.s_addr = htonl(ip);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	return sendto(sock, paquet, taille, 0, (struct sockaddr*)(&sin), sizeof(sin));
}

/*
	Envoie la mise a jour vers une machine.
	On se comporte comme un serveur.
	On attend la connexion de celui qui veut la mise a jour.
*/
int envoyer_maj_tcp(int port, char* paquet, int taille)
{

	struct sockaddr_in sin, csin;
	bzero(&csin, sizeof(struct sockaddr));
	bzero(&sin, sizeof(struct sockaddr));
	
	int sock = socket(AF_INET, SOCK_STREAM, 0), csock, csize = sizeof(struct sockaddr), err, retour = SUCCES;
	if(sock == SOCKET_ERROR)
	{
		return SOCKET_ERROR;
	}

	sin.sin_addr.s_addr = inet_addr("0.0.0.0");
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);

	if(bind(sock, (struct sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		perror("[-] bind");
		return SOCKET_ERROR;
	}
	if(listen(sock, 5) == SOCKET_ERROR)
	{
		perror("[-] listen");
		return SOCKET_ERROR;
	}
	fd_set readfs;
	struct timeval timeout;
	timeout.tv_sec = TIMEOUT_CONNEXION_MAJ;
	timeout.tv_usec = 0;
	FD_ZERO(&readfs); //on vide la selection
	FD_SET(sock, &readfs);
	printf("[i] Attente du select\n");
	err = select(sock + 1, &readfs, NULL, NULL, &timeout);
	printf("[i] Fin d'attente du select\n");
	if(err > 0 && FD_ISSET(sock, &readfs))
	{
		csock = accept(sock, (struct sockaddr*)&csin, &csize);
		send(csock, &taille, sizeof(taille), 0);
		unsigned char hash[150] = {0};
		sha4(paquet, taille, hash, 0);
		printf("[i] Hash avant envoie: ");
		affiche_existe(hash, TAILLE_HASH/8);
		printf("\n");
		int nb_envoie = send(csock, paquet, taille, 0);
		perror("[i] send");
		printf("[i] %d octets envoyés\n");
		closesocket(csock);
	}
	else
	{
		retour = ERREUR;
	}
	closesocket(sock);
	return retour;
}

/*
	Cette fonction permet de recevoir une mise a jour.
	On se connecte a celui qui nous fourni la mise a jour.
*/
int recevoir_maj_tcp(unsigned int ip, int port, unsigned char** paquet, int *taille)
{

	struct sockaddr_in sin;
	int sock = socket(AF_INET, SOCK_STREAM, 0), i = 0;
	int size = 0;
	if(sock == SOCKET_ERROR)
	{
		printf("[-] Socket impossible a creer\n");
		return SOCKET_ERROR;
	}

	sin.sin_addr.s_addr = htonl(ip);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);

	while(connect(sock, (SOCKADDR*)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		i++;
		if(i == NOMBRE_MAX_ESSAI_CONNEXION_MAJ)
		{
			printf("[-] 10 essais\n");
			return SOCKET_ERROR;
		}
		sleep(TIME_BETWEEN_ESSAI_CONNEXION);
	}
	int retour = 0;
	int tmp = recv(sock, &size, sizeof(size), 0);
	*taille = size;
	if(size > 0)
	{
		(*paquet) = (char*)malloc(sizeof(char)*(size));
		bzero(*paquet, size*sizeof(char));
		if((*paquet) != NULL)
		{
			int nb_byte = recv(sock, (void*)(*paquet), size, MSG_WAITALL);
			perror("[i] recv");
			printf("[i] %d octets lus\n");
			unsigned char hash[150] = {0};
			sha4(*paquet, size, hash, 0);
			printf("[i] Hash fichier reçus: ");
			affiche_existe(hash, TAILLE_HASH/8);
			printf("\n");
		}
		else
		{
			retour = SOCKET_ERROR;
		}
	}
	else
	{
		retour = SOCKET_ERROR;
	}
	closesocket(sock);
	return retour;
}

/*
	Recupere la liste des adresses valable de la machine.
	Pour chaque adresse (ce qui correspond a un reseau) on lance un thread de recharche
*/
void recuperer_ip()
{
	struct ifaddrs *liste, *tmp;
	Reseau *reseau = NULL;
	pthread_t thread;
	int erreur;
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == SOCKET_ERROR)
	{
		perror("[-] Erreur de creation de socket : ");
		return;
	}
	getifaddrs(&liste);	
	tmp = liste;
	while(tmp != NULL)
	{
		if(tmp != NULL)
		{
			reseau = (Reseau*) malloc(sizeof(Reseau));					
			reseau->ip = htonl(((struct sockaddr_in*)tmp->ifa_addr)->sin_addr.s_addr);
			reseau->netmask = 0;
			if(tmp->ifa_netmask != NULL)
			{
				reseau->netmask = htonl(((struct sockaddr_in*)tmp->ifa_netmask)->sin_addr.s_addr);
			}
			reseau->sock = sock;
			reseau->reseau = reseau->ip & reseau->netmask;
			if(reseau->ip != htonl(inet_addr("127.0.0.1")) && reseau->netmask != 0)
			{
				erreur = pthread_create(&thread, NULL, &thread_search_reseau, (void*) reseau);
				if(erreur != 0)
				{
					free(reseau);
				}
			}	
			else
			{
				free(reseau);
			}
			tmp = tmp->ifa_next;	
		}
	}
	freeifaddrs(liste);
}

void demander_maj(int ip, short port)
{
	if(already_connecte(ip) == 0)
	{
		return;
	}
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	Paquet paquet;

	paquet.action = ASK_MAJ;	
	paquet.version = VERSION;
	paquet.port_usage = PORT_UDP;
	paquet.specification = 0;
	paquet.suite = 0;

	if(sock == SOCKET_ERROR)
	{
		return;
	}
	envoyer_udp(sock, ip, port, (char*)&paquet, sizeof(paquet));
	closesocket(sock);
}
