#include <stdlib.h>
#include "config.h"
#include "config_polarssl.h"
#include "rsa.h"
#include "clee_pub.h"
#include "camellia.h"
#include "sha4.h"

int dechiffrer_rsa(char* cryptogramme, char* sortie, int taille_sortie)
{
	rsa_context rsa;
	char erreur;
	int taille_cryptogramme = TAILLE_CLEE_RSA/8;
	//On initialise le contexte RSA
	rsa_init( &rsa, RSA_PKCS_V15, 0 );
	//Initialiser les valeurs des clee
	erreur = mpi_read_string( &rsa.N, RSA_N_BASE, RSA_N);
	if(erreur != 0)
	{
		printf("[-] Erreur lecture du N : %d\n", erreur);	
		return ERREUR;
	}
	erreur = mpi_read_string( &rsa.E, RSA_E_BASE, RSA_E);
	if(erreur != 0)
	{
		printf("[-] Erreur lecture du E : %d\n", erreur);	
		return ERREUR;
	}
	//On verifie les clee
	rsa.len = ( mpi_msb( &rsa.N ) + 7 ) >> 3;
	if(TAILLE_CLEE_RSA/8 != rsa.len)
	{
		printf("[-] Erreur de taille : %d/%d\n", rsa.len, taille_cryptogramme);	
		return ERREUR;
	}
	//On dechiffre le cryptogramme
	erreur = rsa_pkcs1_decrypt( &rsa, RSA_PUBLIC, &taille_cryptogramme, cryptogramme, sortie, taille_sortie);
	if(erreur != 0)
	{
		printf("[-] Erreur de dechiffrement : %d\n", erreur);	
		return ERREUR;
	}
	printf("[i] Taille crypto : %d\n", taille_cryptogramme);
	return taille_cryptogramme;
}

int dechiffrer_camelia(unsigned char* clee, unsigned char* cryptogramme, int taille_cryptogramme, unsigned char* sortie, unsigned char IV[16])
{
	camellia_context cam;
	if(camellia_setkey_dec(&cam, clee, TAILLE_CLEE_CAMELIA) != 0)
	{
		return ERREUR;
	}
	if(camellia_crypt_cbc(&cam, CAMELLIA_DECRYPT, taille_cryptogramme, IV, cryptogramme, sortie) != 0)
	{
		return ERREUR;
	}
	return SUCCES;
}

int comparer_md5(unsigned char* donnee, int taille_donnee, unsigned char hash[16])
{
	unsigned char hash_courant[16] = {0}, i;
	md5(donnee, taille_donnee, hash_courant);
	for(i = 0; i < 16; i++)
	{
		if(hash_courant[i] != hash[i])
		{
			return ERREUR;
		}
	}
	return SUCCES;
}

unsigned char* protocole_dechiffrement(unsigned char *fichier_chiffre, int taille)
{
	unsigned char *clee_symetrique = NULL;
	unsigned char *IV = NULL;
	unsigned char *hash_origine = NULL;
	unsigned char hash_claire[TAILLE_HASH/8] = {0};
	unsigned char *cryptogramme_rsa = NULL;
	unsigned char data_dechiffre[(TAILLE_CLEE_RSA/8)] = {0};
	unsigned char *fichier = fichier_chiffre + TAILLE_CLEE_RSA/8;
	unsigned char *fichier_claire = NULL;
	char ret;
	camellia_context camellia;
	//recuperer cryptogramme 
	cryptogramme_rsa = fichier_chiffre;
	
	//dechiffrer le cryptogramme
	printf("[i] Dechiffrement du cryptogramme RSA\n");
	ret = dechiffrer_rsa(cryptogramme_rsa, data_dechiffre, (TAILLE_CLEE_RSA/8));
	if(ret == ERREUR)
	{
		printf("[-] Le dechiffrement a pose une erreur\n");
	}	
	//Extrait les données du texte claire
	printf("[i] Association des donnees\n");
	clee_symetrique = data_dechiffre;
	hash_origine = data_dechiffre + TAILLE_CLEE_CAMELIA/8;
	IV = data_dechiffre + TAILLE_CLEE_CAMELIA/8 + TAILLE_HASH/8;

	//dechiffrer fichier
	printf("[i] Dechiffrement du fichier avec la clee symetrique\n");
	fichier_claire = malloc(sizeof(unsigned char) * (taille - RSA_TAILLE/8));
	camellia_setkey_dec( &camellia, clee_symetrique, TAILLE_CLEE_CAMELIA);
	ret = camellia_crypt_cbc( &camellia, CAMELLIA_DECRYPT, taille - TAILLE_CLEE_RSA/8, IV, fichier, fichier_claire);
	if(ret != 0)
	{
		printf("[-] Le dechiffrement a pose une erreur\n");
		free(fichier_claire);
		return NULL;
	}
	//calculer hash  du fichier claire
	printf("[i] Calcule du hash\n");
	sha4(fichier_claire, taille - TAILLE_CLEE_RSA/8, hash_claire, 0);	
	//comparer les hashs
	printf("[i] Comparaison des hash\n");
	if(memcmp(hash_origine, hash_claire, TAILLE_HASH/8) != 0)
	{
		printf("[-] Les hash ne correspondent pas\n");
		free(fichier_claire);
		return NULL;
	}
	return fichier_claire;	
}
