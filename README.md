# ElevatorSimulation
> Simulation d'un ascenseur  
> Auteur : ELKHALKI Yassine  
> Contact :  
> * yassine.elkhalki@outlook.fr  
> * yassine.elkhalki.auditeur@lecnam.net

---

### Consignes : Simulation d’un ascenseur (threads et mutex)
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

### Implémentation 
**3 versions implémentées :**  
1. Ascenceur simple, a une capacité d'un individu.
2. Ascenceur qui peut prendre un nombre défini de personnes qui se peuvent se trouver à différents étages de départ, mais uniquement si elles vont dans le même sens (monter ou descendre).
3. Ascenceur qui gère l'appel d'individus VIP c'est-à-dire. Si un VIP appelle l'ascenseur, alors il se dirige vers le VIP tout en remplissant son devoir envers les autres passagers : si en allant chercher le VIP il passe par un étage où des individus descendent, alors il les déposes.
