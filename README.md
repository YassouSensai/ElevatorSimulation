# ElevatorSimulation
> Simulation d'un ascenseur  
> Auteur : ELKHALKI Yassine  
> Contact :  
> * yassine.elkhalki@outlook.fr  
> * yassine.elkhalki.auditeur@lecnam.net

---
## Sommaire

1. [Consignes](#consignes--simulation-dun-ascenseur-threads-et-mutex)
2. [Implémentations](#implémentations)
3. [Usage](#usage)

## Consignes : Simulation d’un ascenseur (threads et mutex)
**Contexte :**
* Implémentez un système d’ascenseur pour un bâtiment de 5 étages.
* Les utilisateurs (threads) demandent à monter ou descendre.

**Fonctionnalités à implémenter :**
* Gestion des requêtes d’appel et de destination.
* Simulation du mouvement de l’ascenseur.
* Synchronisation des utilisateurs pour accéder à l’ascenseur.

**Objectifs pédagogiques :**
* Synchronisation avancée des threads.
* Gestion des événements concurrents.

## Implémentations 
> L'ascenseur implémenté est un petit ascenseur. Il ne peut prendre qu'une personne à la fois.

Cette version repose sur le modèle Producteur-Consommateur décrit dans le cours. J'utilise donc :

* Deux mutex (***pthread_mutex_t***). Un pour garantir l'exclusion mutuelle lors de l'accès au buffer de requête partagé. Un autre pour s'assurer que les requêtes de tous les passagers ont été satisfaites.
* Deux sémaophores (***sem_t***). 
    - **empty :** pour dire si l'ascenceur est libre ou non
    - **full :** pour dire qu'un passager est prêt à être transporté

La simulation propose un affichage pas à pas dans le terminal pour suivre l'ascenceur à travers les 5 étages. Voici un aperçu du rendu : 

```plaintext
	 RDC | 1  | 2  | 3  | 4  |   ETAT ASCENSEUR
	----------------------------------------------
	[ E ]  |    |    |    |     <-- En attente...
[USAGER 0] Je veux aller de 2 à 4.
[USAGER 0] C'est moi le prochain !
	  |  [ E ]  |    |    |     <-- Je vais chercher P0 à l'étage 2
[USAGER 1] Je veux aller de 3 à 3.
[USAGER 1] Demande annulée. Je suis déjà à l'étage 3, pas besoin d'ascenseur.
	  |    |  [ E ]  |    |     <-- Je vais chercher P0 à l'étage 2
	  |    |  [ E ]  |    |     <-- Ouverture des portes ...
[USAGER 2] Je veux aller de 0 à 2.
	  |    |  [P0]  |    |     <-- Passager P0 est monté
	  |    |  [P0]  |    |     <-- Fermeture des portes ...
	  |    |    |  [P0]  |     <-- Transport P0 -> 4
	  |    |    |    |  [P0]   <-- Transport P0 -> 4
	  |    |    |    |  [P0]   <-- Ouverture des portes ...
	  |    |    |    |  [ E ]   <-- Passager P0 est descendu
	  |    |    |    |  [ E ]   <-- En attente...
[USAGER 2] C'est moi le prochain !
	  |    |    |  [ E ]  |     <-- Je vais chercher P2 à l'étage 0
	  |    |  [ E ]  |    |     <-- Je vais chercher P2 à l'étage 0
	  |  [ E ]  |    |    |     <-- Je vais chercher P2 à l'étage 0
	[ E ]  |    |    |    |     <-- Je vais chercher P2 à l'étage 0
	[ E ]  |    |    |    |     <-- Ouverture des portes ...
	[P2]  |    |    |    |     <-- Passager P2 est monté
	[P2]  |    |    |    |     <-- Fermeture des portes ...
	  |  [P2]  |    |    |     <-- Transport P2 -> 2
	  |    |  [P2]  |    |     <-- Transport P2 -> 2
	  |    |  [P2]  |    |     <-- Ouverture des portes ...
	  |    |  [ E ]  |    |     <-- Passager P2 est descendu
	  |    |  [ E ]  |    |     <-- En attente...
```

## Usage
Le projet utilise un ```Makefile``` avec le compilateur ```gcc```. Pour compiler et obtenir l'éxecutable sur Linux et macOS, rendez vous à la racine du projet, c'est à dire [ici](./) puis éxecutez la commande suivante :

```bash
make
```

un exécutable du nom de ```ElevatorSimulation``` sera créé et il attend en paramètre la version du simulateur ainsi que le nombre d'usagers.

```bash
./ElevatorSimulation v1 4
```

Pour supprimer les fichiers objets aisni que l'exécutable, vous pouvez lancer cette commande :

```bash
make clean
```


