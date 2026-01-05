/*
* Fichier : simulation_v2.c
* Auteur : ELKHALKI Yassine
* Description : Simulation d'ascenseur "Intelligent" (SCAN) avec capacité multiple.
* Gère l'embarquement et le débarquement "au vol".
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>      // Nécessaire pour O_CREAT sur Mac
#include <time.h>
#include <string.h>

#include "simulation.h"

// --- CONSTANTES & MACROS ---
#define MAX_BUFFER_SIZE 100 // Taille max de la file d'attente (arbitraire)

// --- VARIABLES GLOBALES ---
Passager *file_attente;      // Buffer circulaire (Personnes dehors)
Passager *cabine;            // Personnes DANS l'ascenseur
int capacite_max;            // Capacité de la cabine
int nb_dans_cabine = 0;      // Nombre actuel de passagers dans la cabine

// Indices du Buffer Circulaire
int in = 0;                  // Indice écriture (Usager)
int out = 0;                 // Indice lecture (Ascenseur)
int nb_en_attente = 0;       // Nombre de personnes dans la file d'attente

// Synchronisation
pthread_mutex_t mutex_data;  // Protège buffer, cabine et indices
pthread_mutex_t mutex_aff;   // Protège l'affichage
sem_t *sem_empty;            // Places libres dans le file_attente
sem_t *sem_full;             // Personnes en attente dans le file_attente

int passagers_restants_v2;   // Compteur global pour l'arrêt du programme

// --- PROTOTYPES LOCAUX ---
void afficher_etat_v2(int etage_ascenseur, char *action);
void debarquer_passagers(int etage_actuel);
void embarquer_passagers(int etage_actuel);
int choisir_direction(int etage_actuel, int direction_actuelle);

/**
 * Thread Consommateur : L'Ascenseur Intelligent
 * Implémente une logique de type SCAN : il s'arrête en chemin pour prendre
 * ou déposer des gens si c'est sur sa route.
 * * @param arg Inutilisé
 * @return void*
 */
void *thread_ascenseur_v2(void *arg) {
    (void)arg;
    int etage_actuel = 0;
    int direction = 1; // 1 pour monter, -1 pour descendre, 0 pour attendre

    // Affichage de l'entête
    printf("\n\t RDC | 1  | 2  | 3  | 4  |   ETAT ASCENSEUR (SMART V2)\n");
    printf("\t----------------------------------------------\n");

    while (1) {
        // 1. Vérification de fin de simulation
        pthread_mutex_lock(&mutex_data);
        if (passagers_restants_v2 == 0 && nb_dans_cabine == 0 && nb_en_attente == 0) {
            pthread_mutex_unlock(&mutex_data);
            break;
        }
        pthread_mutex_unlock(&mutex_data);

        // 2. Gestion de l'attente (Si personne ni dedans ni dehors)
        if (nb_dans_cabine == 0) {
            // S'il n'y a personne en attente, on attend (bloquant)
            // On utilise sem_trywait pour ne pas bloquer si on a déjà des cibles, 
            // mais ici on veut bloquer si VRAIMENT rien à faire.
            
            // Astuce : On regarde si quelqu'un attend. Si non, on bloque sur sem_full.
            // Si oui, on continue pour aller le chercher.
            // int val; 
            // Sur mac sem_getvalue est deprecated, on utilise notre compteur manuel
            pthread_mutex_lock(&mutex_data);
            int waiting = nb_en_attente;
            pthread_mutex_unlock(&mutex_data);
            
            if (waiting == 0) {
                afficher_etat_v2(etage_actuel, "En attente...");
                sem_wait(sem_full); // On dort jusqu'à ce qu'un client arrive
                // Un client est arrivé, on reprend le sémaphore (jeton consommé)
                // On doit "rendre" ce jeton virtuel car la logique ci-dessous va gérer le buffer manuellement
                // C'est une petite entorse au modèle pur pour faire du "Smart Logic"
                // On ne consomme pas le passager tout de suite, on va le chercher.
                sem_post(sem_full); 
            }
        }

        // 3. Logique d'étage (Arrêt ?)
        
        // A. DÉBARQUEMENT (Prioritaire)
        debarquer_passagers(etage_actuel);

        // B. EMBARQUEMENT (Si place disponible)
        embarquer_passagers(etage_actuel);

        // 4. Mouvement
        direction = choisir_direction(etage_actuel, direction);
        
        if (direction != 0) {
            // Simulation du trajet
            usleep(500000); // 0.5s par étage
            etage_actuel += direction;
            
            // Affichage pendant le trajet
            char info[50];
            if (nb_dans_cabine > 0) sprintf(info, "Transport (%d pers.)", nb_dans_cabine);
            else sprintf(info, "Vide, vers appel...");
            afficher_etat_v2(etage_actuel, info);
        } else {
            // Pas de mouvement nécessaire, petite pause pour ne pas spammer CPU
            usleep(100000);
        }
    }
    return NULL;
}

/**
 * Fonction helper : Débarque les passagers arrivés à destination
 */
void debarquer_passagers(int etage_actuel) {
    pthread_mutex_lock(&mutex_data);
    int a_ouvert_portes = 0;

    for (int i = 0; i < nb_dans_cabine; i++) {
        if (cabine[i].etage_destination == etage_actuel) {
            if (!a_ouvert_portes) {
                pthread_mutex_unlock(&mutex_data); // On lâche pour afficher
                afficher_etat_v2(etage_actuel, "Ouverture portes (Dépose)");
                usleep(300000);
                pthread_mutex_lock(&mutex_data); // On reprend
                a_ouvert_portes = 1;
            }

            // Afficher info spécifique
            char msg[50];
            sprintf(msg, "P%d est descendu", cabine[i].id);
            pthread_mutex_unlock(&mutex_data);
            afficher_etat_v2(etage_actuel, msg);
            pthread_mutex_lock(&mutex_data);

            // Décalage du tableau (suppression du passager)
            // On remplace le passager parti par le dernier du tableau (optimisation simple)
            cabine[i] = cabine[nb_dans_cabine - 1];
            nb_dans_cabine--;
            passagers_restants_v2--; // Un de moins dans le système global
            i--; // On revérifie la position actuelle car on a swapé
        }
    }
    pthread_mutex_unlock(&mutex_data);
    
    if (a_ouvert_portes) {
        afficher_etat_v2(etage_actuel, "Fermeture portes...");
        usleep(300000);
    }
}

/**
 * Fonction helper : Embarque les passagers qui attendent à cet étage
 */
void embarquer_passagers(int etage_actuel) {
    pthread_mutex_lock(&mutex_data);
    int a_ouvert_portes = 0;

    // On parcourt le buffer circulaire pour voir si quelqu'un attend ICI
    int count = nb_en_attente;
    int idx = out; // On commence à lire depuis la tête de file

    for (int k = 0; k < count; k++) {
        // Si cabine pleine, on arrête
        if (nb_dans_cabine >= capacite_max) break;

        Passager p = file_attente[idx];
        
        // Si le passager attend à CET étage
        // Note: Dans un vrai SCAN, on vérifierait aussi sa direction, 
        // mais ici on prend tout le monde qui est là si on a de la place.
        if (p.etage_depart == etage_actuel) {
            
            if (!a_ouvert_portes) {
                pthread_mutex_unlock(&mutex_data);
                afficher_etat_v2(etage_actuel, "Ouverture portes (Embarq.)");
                usleep(300000);
                pthread_mutex_lock(&mutex_data);
                a_ouvert_portes = 1;
            }

            // Transfert Buffer -> Cabine
            cabine[nb_dans_cabine] = p;
            nb_dans_cabine++;

            // Retrait du Buffer (c'est un peu tricky avec un buffer circulaire strict,
            // ici on va faire une simplification : on marque le passager comme "pris" 
            // et on avancera l'indice out globalement plus tard ou on tasse le tableau.
            // Pour faire simple et robuste : On utilise sem_wait(sem_full) dans la boucle principale
            // pour consommer les jetons. Ici on va tricher un peu pour l'algo "Smart" :
            // On considère qu'on a consommé un item du buffer.
            
            // Affichage
            char msg[50];
            sprintf(msg, "P%d monte -> %d", p.id, p.etage_destination);
            pthread_mutex_unlock(&mutex_data);
            afficher_etat_v2(etage_actuel, msg);
            
            // Consommer le sémaphore full (car on retire un item)
            sem_trywait(sem_full); // Non-bloquant car on sait qu'il est là
            sem_post(sem_empty);   // Signaler une place libre dehors
            
            pthread_mutex_lock(&mutex_data);
            
            // Hack Buffer Circulaire : On doit enlever cet élément précis.
            // Pour simplifier cette démo, on suppose que out pointe toujours sur le plus ancien.
            // Si on prend "au milieu" de la file, c'est dur.
            // Simplification : On ne prend que si c'est le 'out' (tête de file) 
            // OU on reconstruit la file.
            // Pour cette V2 parfaite : On reconstruit la file d'attente sans ce passager.
            
            // (Logique de suppression dans buffer circulaire omise pour brièveté, 
            // on suppose ici que l'ascenseur prend les gens dans l'ordre d'arrivée 
            // OU que file_attente n'est pas strictement FIFO pour le ramassage au vol).
            // -> On va dire qu'on le marque "VIDE" (-1) et le thread ignore les vides.
            file_attente[idx].id = -1; 
            nb_en_attente--;
        }
        
        idx = (idx + 1) % MAX_BUFFER_SIZE;
    }
    
    // Nettoyage rapide du buffer (avancer out si vide)
    while (nb_en_attente > 0 && file_attente[out].id == -1) {
        out = (out + 1) % MAX_BUFFER_SIZE;
    }
    if (nb_en_attente == 0) { // Reset propre si vide
        in = 0; out = 0; 
    }

    pthread_mutex_unlock(&mutex_data);

    if (a_ouvert_portes) {
        afficher_etat_v2(etage_actuel, "Fermeture portes...");
        usleep(300000);
    }
}

/**
 * Fonction helper : Décide de la prochaine direction
 * Logique : Continue dans la direction actuelle s'il y a des drop-offs ou pick-ups.
 * Sinon, va vers la demande la plus proche.
 */
int choisir_direction(int etage_actuel, int direction_actuelle) {
    pthread_mutex_lock(&mutex_data);
    
    // 1. Si on a des passagers, on va vers leur destination
    // Stratégie simple : Si quelqu'un veut aller plus haut, on monte.
    int veut_monter = 0;
    int veut_descendre = 0;
    
    for(int i=0; i<nb_dans_cabine; i++) {
        if (cabine[i].etage_destination > etage_actuel) veut_monter = 1;
        if (cabine[i].etage_destination < etage_actuel) veut_descendre = 1;
    }
    
    // On garde la direction si elle est valide
    if (direction_actuelle == 1 && veut_monter) { pthread_mutex_unlock(&mutex_data); return 1; }
    if (direction_actuelle == -1 && veut_descendre) { pthread_mutex_unlock(&mutex_data); return -1; }
    
    // Sinon on change
    if (veut_monter) { pthread_mutex_unlock(&mutex_data); return 1; }
    if (veut_descendre) { pthread_mutex_unlock(&mutex_data); return -1; }
    
    // 2. Si cabine vide, on va chercher les appels
    if (nb_dans_cabine == 0 && nb_en_attente > 0) {
        // On cherche l'appel le plus proche (parcours buffer)
        // Simplification : on regarde le premier en attente
        int target = -1;
        // Trouver un ID valide
        int idx = out;
        for(int k=0; k<MAX_BUFFER_SIZE; k++) { // Scan tout pour être sûr
             if (file_attente[idx].id != -1) {
                 target = file_attente[idx].etage_depart;
                 break;
             }
             idx = (idx + 1) % MAX_BUFFER_SIZE;
        }
        
        if (target != -1) {
            if (target > etage_actuel) { pthread_mutex_unlock(&mutex_data); return 1; }
            if (target < etage_actuel) { pthread_mutex_unlock(&mutex_data); return -1; }
        }
    }
    
    pthread_mutex_unlock(&mutex_data);
    return 0; // Reste sur place
}


/**
 * Thread Producteur : L'Usager
 * Crée une requête et la dépose dans le buffer partagé.
 * * @param arg Pointeur vers structure Passager
 * @return void*
 */
void *thread_usager_v2(void *arg) {
    Passager moi = *(Passager *)arg;
    
    // Délai aléatoire (0 à 4 secondes)
    usleep(rand() % 4000000);

    if (moi.etage_depart == moi.etage_destination) {
        pthread_mutex_lock(&mutex_data);
        passagers_restants_v2--;
        pthread_mutex_unlock(&mutex_data);
        return NULL;
    }

    printf("[PASSAGER %d] Appelle ascenseur (Etage %d -> %d)\n", moi.id, moi.etage_depart, moi.etage_destination);

    // 1. Attente place Buffer
    sem_wait(sem_empty);

    // 2. Ecriture Buffer
    pthread_mutex_lock(&mutex_data);
    file_attente[in] = moi;
    in = (in + 1) % MAX_BUFFER_SIZE;
    nb_en_attente++;
    pthread_mutex_unlock(&mutex_data);

    // 3. Signalement
    sem_post(sem_full);

    return NULL;
}

/**
 * Affiche l'état de l'ascenseur style "Gantt"
 * Gère l'affichage de multiples passagers [P0,P2]
 * * @param etage_ascenseur Etage actuel
 * @param action Description texte
 */
void afficher_etat_v2(int etage_ascenseur, char *action) {
    pthread_mutex_lock(&mutex_aff);
    
    printf("\t");
    for (int i = 0; i < 5; i++) {
        if (i == etage_ascenseur) {
            pthread_mutex_lock(&mutex_data);
            if (nb_dans_cabine > 0) {
                // Construction string passagers ex: [P0,P2]
                printf("[\033[1;32m");
                for(int k=0; k<nb_dans_cabine; k++) {
                    printf("P%d", cabine[k].id);
                    if(k < nb_dans_cabine-1) printf(",");
                }
                printf("\033[0m]");
            } else {
                printf("[\033[1;33m E \033[0m]"); 
            }
            pthread_mutex_unlock(&mutex_data);
        } else {
            printf("  |  ");
        }
    }
    printf("   <-- %s\n", action);
    
    pthread_mutex_unlock(&mutex_aff);
}

/**
 * Lanceur V2
 */
void run_simulation_v2(int nb_usagers, int capacite) {
    printf("\n\n=== Lancement V2 SMART (Capacité: %d) ===\n\n", capacite);

    passagers_restants_v2 = nb_usagers;
    capacite_max = capacite;
    
    // Allocation dynamique
    file_attente = malloc(sizeof(Passager) * MAX_BUFFER_SIZE);
    cabine = malloc(sizeof(Passager) * capacite);
    
    // Init array pour gestion des trous
    for(int i=0; i<MAX_BUFFER_SIZE; i++) file_attente[i].id = -1;

    pthread_mutex_init(&mutex_data, NULL);
    pthread_mutex_init(&mutex_aff, NULL);
    
    sem_unlink("/sem_v2_empty");
    sem_unlink("/sem_v2_full");
    // Le buffer externe a une taille MAX_BUFFER_SIZE (file d'attente hall)
    sem_empty = sem_open("/sem_v2_empty", O_CREAT, 0644, MAX_BUFFER_SIZE); 
    sem_full  = sem_open("/sem_v2_full", O_CREAT, 0644, 0);

    pthread_t ascenseur;
    pthread_create(&ascenseur, NULL, thread_ascenseur_v2, NULL);

    srand(time(NULL));
    pthread_t *usagers = malloc(sizeof(pthread_t) * nb_usagers);
    Passager *data = malloc(sizeof(Passager) * nb_usagers);

    for (int i = 0; i < nb_usagers; i++) {
        data[i].id = i;
        do {
            data[i].etage_depart = rand() % 5;
            data[i].etage_destination = rand() % 5;
        } while (data[i].etage_depart == data[i].etage_destination);
        pthread_create(&usagers[i], NULL, thread_usager_v2, &data[i]);
    }

    // Attente fin
    for (int i = 0; i < nb_usagers; i++) {
        pthread_join(usagers[i], NULL);
    }
    pthread_join(ascenseur, NULL);

    // Nettoyage
    sem_close(sem_empty); sem_close(sem_full);
    sem_unlink("/sem_v2_empty"); sem_unlink("/sem_v2_full");
    pthread_mutex_destroy(&mutex_data);
    pthread_mutex_destroy(&mutex_aff);
    free(file_attente);
    free(cabine);
    free(usagers);
    free(data);

    printf("\n=== Fin V2 ===\n");
}