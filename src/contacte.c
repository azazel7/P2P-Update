/*
Toutes les fonction relatif au contacte
*/

#include "config.h"

void initialiser_contacte(Contacte *l_client, Contacte *l_serveur, Historique *l_historique)
{
    int i = 0;
    for(i = 0; i < NOMBRE_MAX_CLIENT; i++)
    {
        l_client[i].deadline = 0;
        l_client[i].ip = 0;
    }

    for(i = 0; i < NOMBRE_MAX_SERVEUR; i++)
    {
        l_serveur[i].deadline = 0;
        l_serveur[i].ip = 0;
    }

    for(i = 0; i < NOMBRE_MAX_HISTORIQUE; i++)
    {
        l_historique->liste[i] = 0;
    }
	l_historique->index = 0;
}


