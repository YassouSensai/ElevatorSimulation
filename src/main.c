/*
* Ficher : main.c
* Auteur : ELKHALKI Yassine
* Description : Point d'entrée principal pour lancer une simulation d'ascenseur avec un nombre spécifié d'usagers.
*/

#include <stdio.h>
#include <stdlib.h>
#include "simulation.h"

int main(int argc, char *argv[]) {
    // Vérification des arguments
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <nombre_usagers>\n", argv[0]);
        return 1;
    }

    int nb_usagers = atoi(argv[1]);
    if (nb_usagers <= 0) {
        fprintf(stderr, "Erreur: Le nombre d'usagers doit être > 0.\n");
        return 1;
    }

    // Lancement de la simulation unique
    run_elevator_simulation(nb_usagers);

    return 0;
}