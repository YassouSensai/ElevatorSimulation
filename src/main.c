/*
* Ficher : main.c
* Auteur : ELKHALKI Yassine
* Description : Point d'entrée principal pour lancer une simulation d'ascenseur avec un nombre spécifié d'usagers.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simulation.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s v1 <nb_usagers>\n", argv[0]);
        fprintf(stderr, "  %s v2 <nb_usagers> <capacite_ascenseur>\n", argv[0]);
        return 1;
    }

    char *version = argv[1];
    int nb_usagers = atoi(argv[2]);

    if (strcmp(version, "v1") == 0) {
        run_simulation_v1(nb_usagers);
    } 
    else if (strcmp(version, "v2") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Erreur V2 : Précisez la capacité de l'ascenseur.\n");
            return 1;
        }
        int capacite = atoi(argv[3]);
        run_simulation_v2(nb_usagers, capacite);
    } 
    else {
        fprintf(stderr, "Version inconnue (v1 ou v2).\n");
        return 1;
    }

    return 0;
}