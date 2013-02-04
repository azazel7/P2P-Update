#include "config.h"
#include "liste_chaine.h"
#include <stdio.h>
#include <stdlib.h>



llist ajouterEnTete(llist liste, Contacte valeur)
{
    /* On cr�e un nouvel �l�ment */
    element* nouvelElement = malloc(sizeof(element));

    /* On assigne la valeur au nouvel �l�ment */
    nouvelElement->val = valeur;

    /* On assigne l'adresse de l'�l�ment suivant au nouvel �l�ment */
    nouvelElement->nxt = liste;

    /* On retourne la nouvelle liste, i.e. le pointeur sur le premier �l�ment */
    return nouvelElement;
}

llist ajouterEnFin(llist liste, Contacte valeur)
{
    /* On cr�e un nouvel �l�ment */
    element* nouvelElement = malloc(sizeof(element));

    /* On assigne la valeur au nouvel �l�ment */
    nouvelElement->val = valeur;

    /* On ajoute en fin, donc aucun �l�ment ne va suivre */
    nouvelElement->nxt = NULL;

    if(liste == NULL)
    {
        /* Si la liste est vide� il suffit de renvoyer l'�l�ment cr�� */
        return nouvelElement;
    }
    else
    {
        /* Sinon, on parcourt la liste � l'aide d'un pointeur temporaire et on
        indique que le dernier �l�ment de la liste est reli� au nouvel �l�ment */
        element* temp=liste;
        while(temp->nxt != NULL)
        {
            temp = temp->nxt;
        }
        temp->nxt = nouvelElement;
        return liste;
    }
}

void afficherListe(llist liste)
{
    element *tmp = liste;
    /* Tant que l'on n'est pas au bout de la liste */
    while(tmp != NULL)
    {
        /* On affiche */
        //printf("%d ", tmp->val);
        /* On avance d'une case */
        tmp = tmp->nxt;
    }
}


int estVide(llist liste)
{
    if(liste == NULL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

llist supprimerElementEnTete(llist liste)
{
    if(liste != NULL)
    {
        /* Si la liste est non vide, on se pr�pare � renvoyer l'adresse de
        l'�l�ment en 2�me position */
        element* aRenvoyer = liste->nxt;
        /* On lib�re le premier �l�ment */
        free(liste);
        /* On retourne le nouveau d�but de la liste */
        return aRenvoyer;
    }
    else
    {
        return NULL;
    }
}

llist supprimerElementEnFin(llist liste)
{
    /* Si la liste est vide, on retourne NULL */
    if(liste == NULL)
        return NULL;

    /* Si la liste contient un seul �l�ment */
    if(liste->nxt == NULL)
    {
        /* On le lib�re et on retourne NULL (la liste est maintenant vide) */
        free(liste);
        return NULL;
    }

    /* Si la liste contient au moins deux �l�ments */
    element* tmp = liste;
    element* ptmp = liste;
    /* Tant qu'on n'est pas au dernier �l�ment */
    while(tmp->nxt != NULL)
    {
        /* ptmp stock l'adresse de tmp */
        ptmp = tmp;
        /* On d�place tmp (mais ptmp garde l'ancienne valeur de tmp */
        tmp = tmp->nxt;
    }
    /* A la sortie de la boucle, tmp pointe sur le dernier �l�ment, et ptmp sur
    l'avant-dernier. On indique que l'avant-dernier devient la fin de la liste
    et on supprime le dernier �l�ment */
    ptmp->nxt = NULL;
    free(tmp);
    return liste;
}

llist rechercherElement(llist liste, Contacte valeur)
{
    element *tmp=liste;
    /* Tant que l'on n'est pas au bout de la liste */
    while(tmp != NULL)
    {
        if(tmp->val.ip == valeur.ip)
        {
            /* Si l'�l�ment a la valeur recherch�e, on renvoie son adresse */
            return tmp;
        }
        tmp = tmp->nxt;
    }
    return NULL;
}

llist element_i(llist liste, int indice)
{
    int i;
    /* On se d�place de i cases, tant que c'est possible */
    for(i=0; i<indice && liste != NULL; i++)
    {
        liste = liste->nxt;
    }

    /* Si l'�l�ment est NULL, c'est que la liste contient moins de i �l�ments */
    if(liste == NULL)
    {
        return NULL;
    }
    else
    {
        /* Sinon on renvoie l'adresse de l'�l�ment i */
        return liste;
    }
}

llist supprimerElement(llist liste, Contacte valeur)
{
    /* Liste vide, il n'y a plus rien � supprimer */
    if(liste == NULL)
        return NULL;

    /* Si l'�l�ment en cours de traitement doit �tre supprim� */
    if(liste->val.ip == valeur.ip)
    {
        /* On le supprime en prenant soin de m�moriser
        l'adresse de l'�l�ment suivant */
        element* tmp = liste->nxt;
        free(liste);
        /* L'�l�ment ayant �t� supprim�, la liste commencera � l'�l�ment suivant
        pointant sur une liste qui ne contient plus aucun �l�ment ayant la valeur recherch�e */
        tmp = supprimerElement(tmp, valeur);
        return tmp;
    }
    else
    {
        /* Si l'�lement en cours de traitement ne doit pas �tre supprim�,
        alors la liste finale commencera par cet �l�ment et suivra une liste ne contenant
        plus d'�l�ment ayant la valeur recherch�e */
        liste->nxt = supprimerElement(liste->nxt, valeur);
        return liste;
    }
}

int nombreElements(llist liste)
{
    int nb = 0;
    element* tmp = liste;

    /* On parcours la liste */
    while(tmp != NULL)
    {
        /* On incr�mente */
        nb++;
        tmp = tmp->nxt;
    }
    /* On retourne le nombre d'�l�ments parcourus */
    return nb;
}

llist effacerListe(llist liste)
{
    element* tmp = liste;
    element* tmpnxt;

    /* Tant que l'on n'est pas au bout de la liste */
    while(tmp != NULL)
    {
        /* On stocke l'�l�ment suivant pour pouvoir ensuite avancer */
        tmpnxt = tmp->nxt;
        /* On efface l'�l�ment courant */
        free(tmp);
        /* On avance d'une case */
        tmp = tmpnxt;
    }
    /* La liste est vide : on retourne NULL */
    return NULL;
}

llist supprimerElement_i(llist liste, int indice)
{
    int i = 0;
    element* tmp = liste;
    element* previous = NULL;

     /* Liste vide, il n'y a plus rien � supprimer */

    if(liste == NULL)
    {
        return NULL;
    }
    if(indice == 0)
    {
        return supprimerElementEnTete(liste);
    }


    while(tmp->nxt != NULL && i < indice)
    {
        previous = tmp;
        tmp = tmp->nxt;
        i++;
    }
    if(i == indice)
    {
        previous->nxt = tmp->nxt;
        free(tmp);
    }
    return liste;
}
