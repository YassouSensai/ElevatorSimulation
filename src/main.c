#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elevator.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <version> <nombre_usagers>\n", argv[0]);
        printf("Exemple: %s v1 5\n", argv[0]);
        return 1;
    }

    char *version = argv[1];
    int nb_usagers = atoi(argv[2]);

    if (strcmp(version, "v1") == 0) {
        run_elevator_v1(nb_usagers);
    } 
    // Tu ajouteras v2 et v3 ici plus tard
    else {
        printf("Version inconnue. Utilisez 'v1'.\n");
    }

    return 0;
}