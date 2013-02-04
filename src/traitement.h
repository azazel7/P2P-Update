#ifndef RESEAU_H_INCLUDED
#define RESEAU_H_INCLUDED
void traitement_ASK_CONNECTION(int ip, short port);
void traitement_ANS_CONNECTION_NO(int ip);
void traitement_ANS_CONNECTION_YES(int ip, short port);
void traitement_VAL_CONNECTION(int ip);
void traitement_KEEP_CONTACTE(int ip);
void traitement_ASK_MAJ(int ip, short port);
void traitement_ANS_MAJ_YES(int ip, short port);
void traitement_ANS_MAJ_NO();
void traitement_GIVE_CONTACTE(LargePaquet l_paquet);

#endif // RESEAU_H_INCLUDED
