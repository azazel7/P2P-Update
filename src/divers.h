#ifndef DIVERS_H_INCLUDED
#define DIVERS_H_INCLUDED

void handle_sigalrm(int sig);
int already_connecte(int ip);
int taille_fichier(FILE *fichier);
char* charger_fichier(char* chemin, int *taille);
int remplir_fichier(char* chemin, char* contenu, int taille);
int executer_maj(char* chemin);
char* afficher_ip(int ip);
void affiche_existe(unsigned char* tableau, unsigned int taille);


#endif // DIVERS_H_INCLUDED
