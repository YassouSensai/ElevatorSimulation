/*
* Fichier : simulation.h
* Auteur : ELKHALKI Yassine
* Description : Déclarations prototypes et structure Passager pour la simulation d'ascenseur.
*/

#ifndef SIMULATION_H
#define SIMULATION_H

#include <pthread.h>
#include <semaphore.h>

typedef struct {
    int id;
    int etage_depart;
    int etage_destination;
} Passager;

void run_elevator_simulation(int nb_usagers);
void *thread_ascenseur(void *arg);
void *thread_usager(void *arg);
void afficher_etat(int etage_ascenseur, int id_passager, char *action);

#endif