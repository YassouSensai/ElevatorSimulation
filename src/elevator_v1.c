#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>      // Nécessaire pour O_CREAT sur Mac
#include "elevator.h"
#include "common.h"

// --- VARIABLES GLOBALES (Comme dans le Listing 6 du cours) ---
Passager buffer;        // La donnée partagée (taille 1)
pthread_mutex_t mutex;  // Pour protéger l'accès au buffer [cite: 469]
sem_t *empty;           // Sémaphore : places libres 
sem_t *full;            // Sémaphore : items disponibles 

/**
 * Consommateur : L'Ascenseur
 * L'ascenseur va consommer les requêtes réalisées par les usagers.
 * Il est initialisé au rez-de-chaussée (étage 0) et attend les demandes.
 */
void *thread_ascenseur(void *arg) {
    (void)arg;
    int etage_actuel = 0;

    while (1) {
        if (etage_actuel == 0) {
            printf("[ASCENSEUR] Je suis au RDC.\n");
        } else {
            printf("[ASCENSEUR] Je suis à l'étage %d.\n", etage_actuel);
        }

        // On attend un passager
        sem_wait(full); 
        // On reccupere le passage
        pthread_mutex_lock(&mutex); 
        Passager p = buffer; 
        pthread_mutex_unlock(&mutex);

        // On fait le trajet
        printf("[ASCENSEUR] Prise en charge client %d (Etage %d -> %d)\n", p.id, p.etage_depart, p.etage_destination);
        sleep(2);         
        etage_actuel = p.etage_destination;
        printf("[ASCENSEUR] Client %d déposé à l'étage %d.\n", p.id, etage_actuel);

        // On libere l'ascenseur
        sem_post(empty); // [cite: 507]
    }
    return NULL;
}

// --- LE PRODUCTEUR (L'Usager) ---
void *thread_usager(void *arg) {
    Passager moi = *(Passager *)arg;

    printf("[CLIENT %d] Je veux aller de %d à %d.\n", moi.id, moi.etage_depart, moi.etage_destination);

    // 1. Attendre que l'ascenseur soit libre (P)
    sem_wait(empty); // [cite: 472]

    // 2. Protéger l'écriture
    pthread_mutex_lock(&mutex); // [cite: 473]
    buffer = moi; // Je rentre dans l'ascenseur (j'écris ma demande)
    pthread_mutex_unlock(&mutex); // [cite: 478]

    // 3. Signaler à l'ascenseur qu'il y a un client (V)
    sem_post(full); // [cite: 479]

    printf("[CLIENT %d] Je suis dans l'ascenseur.\n", moi.id);
    return NULL;
}

// --- INITIALISATION ---
void run_elevator_v1(int nb_usagers) {
    printf("=== Lancement V1 (Strict Producteur-Consommateur) ===\n");

    pthread_t ascenseur;
    pthread_t threads_usagers[nb_usagers];
    Passager donnees_usagers[nb_usagers]; // Tableau pour stocker les infos

    // Initialisation Mutex [cite: 527]
    pthread_mutex_init(&mutex, NULL);

    // Initialisation Sémaphores (Version Mac obligée)
    // empty = 1 (1 place disponible au départ)
    // full = 0 (0 client au départ)
    sem_unlink("/sem_v1_empty"); 
    sem_unlink("/sem_v1_full");
    empty = sem_open("/sem_v1_empty", O_CREAT, 0644, 1);
    full  = sem_open("/sem_v1_full",  O_CREAT, 0644, 0);

    // Création thread Ascenseur [cite: 536]
    pthread_create(&ascenseur, NULL, thread_ascenseur, NULL);

    // Création threads Usagers [cite: 533]
    for (int i = 0; i < nb_usagers; i++) {
        donnees_usagers[i].id = i;
        donnees_usagers[i].etage_depart = rand() % 5;
        donnees_usagers[i].etage_destination = rand() % 5;
        pthread_create(&threads_usagers[i], NULL, thread_usager, &donnees_usagers[i]);
        sleep(1); // Petit délai pour qu'ils n'arrivent pas tous à la milliseconde exacte
    }

    // Attente fin des usagers [cite: 543]
    for (int i = 0; i < nb_usagers; i++) {
        pthread_join(threads_usagers[i], NULL);
    }

    // Nettoyage [cite: 554]
    sem_close(empty);
    sem_close(full);
    sem_unlink("/sem_v1_empty");
    sem_unlink("/sem_v1_full");
    pthread_mutex_destroy(&mutex);
    
    printf("=== Fin simulation ===\n");
    // On laisse le thread ascenseur mourir avec le programme (car boucle infinie)
}