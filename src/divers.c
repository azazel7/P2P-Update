
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "liste_chaine.h"
#include "divers.h"

extern llist liste_client, liste_serveur, liste_ip;
extern Historique liste_historique;

/*
   Fonction de traitement du signale SIGALRM
   Elle regarde tous les contactes, et pour ceux dont la deadline est depassee, elle les suppriment.
   Pour les autres, elle envoie un KEEP_CONTACTE.
   Puis elle rearme l'alarme.
 */
void handle_sigalrm(int sig)
{
	int temps = time(NULL);
	int i, nb, sock;
	llist tmp = NULL;
	Paquet paquet;

	sock = socket(AF_INET, SOCK_DGRAM, 0);

	//Remplir le paquet	
	paquet.port_usage = PORT_UDP;
	paquet.version = VERSION;
	paquet.action = KEEP_CONTACTE;
	paquet.specification = 0;
	paquet.suite = 0;

	//on nettoie les contactes et on envoie un KEEP_CONTACTE au autres
	nb = nombreElements(liste_client);
	for(i = 0; i < nb; i++)
	{
		tmp = element_i(liste_client, i);
		if(tmp->val.deadline < temps)
		{
			liste_client = supprimerElement_i(liste_client, i);
		}
		else if(sock != SOCKET_ERROR)
		{
			//Envoyer un KEEP_CONTACTE
			envoyer_udp(sock, tmp->val.ip, PORT_UDP, (char*)&paquet, sizeof(paquet));
		}
	}

	nb = nombreElements(liste_serveur);
	for(i = 0; i < nb; i++)
	{
		tmp = element_i(liste_serveur, i);
		if(tmp->val.deadline < temps)
		{
			liste_serveur = supprimerElement_i(liste_serveur, i);
		}
		else if(sock != SOCKET_ERROR)
		{
			//Envoyer un KEEP_CONTACTE
			envoyer_udp(sock, tmp->val.ip, PORT_UDP, (char*)&paquet, sizeof(paquet));
		}
	}
	alarm(MAX_BEFORE_ERASE_CONTACTE/3);//On rearme la prochaine alarme
}

/*
   Fonction verifiant si l'on est deja connecte a une adresse ip
 */
int already_connecte(int ip)
{
	llist tmp;
	Contacte contacte;
	contacte.ip = ip;

	tmp = rechercherElement(liste_client, contacte);
	if(tmp != NULL)
	{
		return 1;
	}
	tmp = rechercherElement(liste_serveur, contacte);
	if(tmp != NULL)
	{
		return 1;
	}
	tmp = rechercherElement(liste_ip, contacte);
	if(tmp != NULL)
	{
		return 1;
	}
	return 0;
}

//Charger un fichier dans un tableau de byte
char* charger_fichier(char* chemin, int *taille)
{
	FILE *fichier = NULL;
	char* retour;
	int i;
	fichier = fopen(chemin, "rb"); 	//on ouvre le fichier
	if(fichier == NULL)
	{
		return NULL;
	}
	(*taille) = taille_fichier(fichier); //On calcule la taille du fichier
	retour = (char*) malloc((*taille)*sizeof(char)); //On alloue un tableau

	for(i = 0; i < (*taille); i++) //On lit le fichier du debut a la fin
	{
		retour[i] = fgetc(fichier);
	}	
	fclose(fichier);
	return retour;
}

//Mesure la taille d'un fichier
int taille_fichier(FILE *fichier)
{
	int retour, tmp = ftell(fichier); //on stock la position actuel
	fseek(fichier, 0, SEEK_END); 	//on se place a  la fin
	retour = ftell(fichier); 	//on stock la position (nombre de caracteres)
	fseek(fichier, tmp, SEEK_SET); //On se replace dans le fichier
	return retour;
}

//remplis un fichier a partir d'un tableau de byte
int remplir_fichier(char* chemin, char* contenu, int taille)
{
	FILE* fichier = NULL;
	int i;
	fichier = fopen(chemin, "wb");
	if(fichier == NULL)
	{
		perror("fopen");
		return ERREUR;
	}
	for(i = 0; i < taille; i++)
	{
		fputc(contenu[i], fichier);
	}
	fclose(fichier);
	return SUCCES;
}

int executer_maj(char* chemin)
{
	printf("[+] Execution de la mise a jour\n");
	execl(chemin, chemin, NULL);	
	return SUCCES;	
}

char* afficher_ip(int ip)
{
	struct sockaddr_in sin;
	sin.sin_addr.s_addr = htonl(ip);
	return inet_ntoa(sin.sin_addr);
}

void affiche_existe(unsigned char* tableau, unsigned int taille)
{
	unsigned int i = 0;
	for(; i < taille; i++)
	{
		printf("%02X", tableau[i]);
	}
}
