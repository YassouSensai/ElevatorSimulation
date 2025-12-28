#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>      // Nécessaire pour O_CREAT sur Mac

#include "elevator.h"
#include "common.h"

void afficher_etat(int etage_ascenseur, int id_passager, char *action);

// VARIABLES GLOBALES (Comme dans le Listing 6 du cours)
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

    // Affichage de l'entête du graphique une seule fois
    printf("\n\t RDC | 1  | 2  | 3  | 4  |   ETAT ASCENSEUR\n");
    printf("\t----------------------------------------------\n");

    while (1) {
        // État initial (Attente)
        afficher_etat(etage_actuel, -1, "En attente...");

        // On attend un passager (P)
        sem_wait(full); 
        
        // On récupère le passager (Section critique)
        pthread_mutex_lock(&mutex); 
        Passager p = buffer; 
        pthread_mutex_unlock(&mutex);

        // On se rend à l'étage de départ du passager
        if (etage_actuel != p.etage_depart) {
            char info[50];
            sprintf(info, "Je vais chercher P%d à l'étage %d", p.id, p.etage_depart);
            
            int direction = (p.etage_depart > etage_actuel) ? 1 : -1;

            while (etage_actuel != p.etage_depart) {
                sleep(1); 
                etage_actuel += direction;
                afficher_etat(etage_actuel, -1, info);
            }
        }

        // Le passager monte
        afficher_etat(etage_actuel, -1, "Ouverture des portes ...");
        sleep(2);
        char info2[50];
        sprintf(info2, "Passager P%d est monté", p.id);
        afficher_etat(etage_actuel, p.id, info2);
        afficher_etat(etage_actuel, p.id, "Fermeture des portes ...");
        sleep(2);



        // On se rend à l'étage d'arrivée du passager
        if (etage_actuel != p.etage_destination) {
            char info[50];
            sprintf(info, "Transport P%d -> %d", p.id, p.etage_destination);
            
            int direction = (p.etage_destination > etage_actuel) ? 1 : -1;

            while (etage_actuel != p.etage_destination) {
                sleep(1);
                etage_actuel += direction;
                afficher_etat(etage_actuel, p.id, info);
            }
        }

        // On dépose le passager
        afficher_etat(etage_actuel, p.id, "Ouverture des portes ...");
        sleep(2);
        char info3[50];
        sprintf(info3, "Passager P%d est descendu", p.id);
        afficher_etat(etage_actuel, -1, info3);
        
        // On libère l'ascenseur (V)
        sem_post(empty);
    }
    return NULL;
}

/**
 * Producteur : L'Usager
 * Chaque usager crée une requête pour aller d'un étage à un autre.
 */
void *thread_usager(void *arg) {
    Passager moi = *(Passager *)arg;

    printf("[USAGER %d] Je veux aller de %d à %d.\n", moi.id, moi.etage_depart, moi.etage_destination);

    // Si le client est déjà à l'étage demandé.
    if (moi.etage_depart == moi.etage_destination) {
        printf("[USAGER %d] Demande annulée. Je suis déjà à l'étage %d, pas besoin d'ascenseur.\n", moi.id, moi.etage_depart);
        return NULL;
    }

    // On Attend que l'ascenseur soit libre (P)
    sem_wait(empty); 

    // Lorsque l'ascenseur est libre, on fait notre requête
    pthread_mutex_lock(&mutex); 
    buffer = moi;
    pthread_mutex_unlock(&mutex); 

    // On prévient que l'ascenseur est plein (V)
    sem_post(full); 

    printf("[USAGER %d] C'est moi le prochain !\n", moi.id);
    return NULL;
}

/**
 * Lance la simulation V1 : Ascenseur capacité 1 personne
 */
void run_elevator_v1(int nb_usagers) {
    printf("=== Lancement V1 (Strict Producteur-Consommateur) ===\n");

    pthread_t ascenseur;
    pthread_t threads_usagers[nb_usagers];   // Tableau des threads usagers
    Passager donnees_usagers[nb_usagers];    // Tableau pour stocker les infos

    // Initialisation Mutex
    pthread_mutex_init(&mutex, NULL);

    // Initialisation Sémaphores (Version Mac obligée)
    // empty = 1 (1 place disponible au départ)
    // full = 0 (0 client au départ)
    sem_unlink("/sem_v1_empty"); 
    sem_unlink("/sem_v1_full");
    empty = sem_open("/sem_v1_empty", O_CREAT, 0644, 1);
    full  = sem_open("/sem_v1_full",  O_CREAT, 0644, 0);

    // Création thread Ascenseur
    pthread_create(&ascenseur, NULL, thread_ascenseur, NULL);

    // Création threads Usagers
    for (int i = 0; i < nb_usagers; i++) {
        donnees_usagers[i].id = i;
        donnees_usagers[i].etage_depart = rand() % 5;
        donnees_usagers[i].etage_destination = rand() % 5;
        pthread_create(&threads_usagers[i], NULL, thread_usager, &donnees_usagers[i]);
        sleep(1);
    }

    // Attendre que tous les threads se terminent
    for (int i = 0; i < nb_usagers; i++) {
        pthread_join(threads_usagers[i], NULL);
    }

    // Fermeture et destruction des sémaphores et mutex
    sem_close(empty);
    sem_close(full);
    sem_unlink("/sem_v1_empty");
    sem_unlink("/sem_v1_full");
    pthread_mutex_destroy(&mutex);
    
    printf("=== Fin simulation ===\n");
}


/**
 * Fonction d'affichage de l'état de l'ascenseur
 * dans le terminal
 * 
 * @param etage_ascenseur : Étage actuel de l'ascenseur (0-4)
 * @param id_passager : ID du passager dans l'ascenseur (-1 si vide)
 * @param action : Description de l'action en cours
 * @return void
 */
void afficher_etat(int etage_ascenseur, int id_passager, char *action) {
    
    printf("\t");

    for (int i = 0; i < 5; i++) {
        if (i == etage_ascenseur) {
            // On verifie s'il s'agit d'un passager ou non
            if (id_passager >= 0) {
                printf("[\033[1;32mP%d\033[0m]", id_passager); 
            } else {
                printf("[\033[1;33m E \033[0m]"); 
            }
        } else {
            printf("  |  ");
        }
    }

    printf("   <-- %s\n", action);
}