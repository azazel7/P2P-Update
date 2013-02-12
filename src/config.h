#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define UNIX

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

#define VERSION 1

#define PORT_UDP 40000
#define PORT_TCP 3000

#define NOMBRE_MAX_CLIENT 10
#define NOMBRE_MAX_SERVEUR 10
#define NOMBRE_MAX_HISTORIQUE 10

#define MAX_BETWEEN_KEEP_CONTACTE 60*4
#define MAX_BEFORE_ERASE_CONTACTE 60*4

#define NOMBRE_MAX_IP_BY_LARGEPAQUET 12

#define TIME_BETWEEN_ESSAI_CONNEXION 1
#define TIMEOUT_CONNEXION_MAJ 20
#define NOMBRE_MAX_ESSAI_CONNEXION_MAJ 10

#define ERREUR -1
#define SUCCES 0

#define TAILLE_CLEE_RSA 4096
#define TAILLE_CLEE_CAMELIA 256
#define TAILLE_HASH 512
#define TAILLE_IV 128

typedef struct
{
    char action; //demande de connexion, reponse a une demande de connexion, transfer de connaissance, garder contacte, demande de mise à jour, reponse de demande de mise a jour
    char version; //Version du logiciel qui emet le paquet
    char specification; //specification de l'action
    short port_usage;
    char suite; //Indique si c'est un plus gros paquet qui est reçu
} Paquet;

/*Relatif a Paquet.action et au specification*/
#define ASK_CONNECTION 1 //demande de connection

#define ANS_CONNECTION_NO 2 //reponse negative a une demande de connection

#define ANS_CONNECTION_YES 3 //reponse negative a une demande de connection

#define VAL_CONNECTION 4 //Permet de valider une connection (poignée de main en trois fois)

#define KEEP_CONTACTE 100 //Est envoyer pour garder contacte

#define ASK_MAJ 10 //Est envoyer pour une demande de mise a jour

#define ANS_MAJ_NO 11  //Est envoyer en tant que reponse negative d'une demande de mise a jour

#define ANS_MAJ_YES 12  //Est envoyer en tant que reponse negative d'une demande de mise a jour

#define GIVE_CONTACTE 20 //Signale que l'on envoie une serie de contacte

typedef struct
{
    Paquet paquet;
    int ip[NOMBRE_MAX_IP_BY_LARGEPAQUET];
} LargePaquet;


typedef struct
{
	int ip;
    int reseau; //est en big endian
    int netmask; //est en big endian
    int sock;
} Reseau;

typedef struct
{
    int ip;
    int deadline;
} Contacte;
typedef struct
{
	int index;
	int liste[NOMBRE_MAX_HISTORIQUE];
} Historique;
#endif // CONFIG_H_INCLUDED
