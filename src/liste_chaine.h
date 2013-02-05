#ifndef LISTE_CHAINE_H_INCLUDED
#define LISTE_CHAINE_H_INCLUDED

#include "config.h"

typedef struct element element;
struct element
{
    Contacte val;
    struct element *nxt;
};

typedef element* llist;

llist ajouterEnTete(llist liste, Contacte valeur);
llist ajouterEnFin(llist liste, Contacte valeur);
int estVide(llist liste);
llist supprimerElementEnTete(llist liste);
llist supprimerElementEnFin(llist liste);
llist rechercherElement(llist liste, Contacte valeur);
llist element_i(llist liste, int indice);
llist supprimerElement(llist liste, Contacte valeur);
int nombreElements(llist liste);
llist effacerListe(llist liste);
llist supprimerElement_i(llist liste, int indice);

#endif // LISTE_CHAINE_H_INCLUDED
