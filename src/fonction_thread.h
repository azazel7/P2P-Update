#ifndef FONCTION_THREAD_H_INCLUDED
#define FONCTION_THREAD_H_INCLUDED

void* thread_search_reseau(void* data);
void* thread_reception_data(void* data);
void* thread_reception_maj(void* data);
void* thread_envoie_maj(void* data);

#endif // FONCTION_THREAD_H_INCLUDED
