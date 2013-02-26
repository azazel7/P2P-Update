#ifndef RESEAU_H_INCLUDED
#define RESEAU_H_INCLUDED
int envoyer_udp(int sock, long unsigned int ip, int port, char *paquet, int taille);
int envoyer_maj_tcp(int port, char* paquet, int taille);
void recuperer_ip();
int recevoir_maj_tcp(unsigned int ip, int port, unsigned char** paquet, int *taille);
void demander_maj(int ip, short port);


#endif // RESEAU_H_INCLUDED
