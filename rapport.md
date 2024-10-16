# Laboratoire 03 : Gestion d'accès concurrents

Auteurs : Léon Surbeck, Alex Berberat

## Introduction au problème

Le but de ce laboratoire est de simuler la gestion d'une crise sanitaire dans une petite ville, en mettant en avant les défis liés aux accès concurrents. Quatre acteurs principaux doivent interagir efficacement : les ambulances, les fournisseurs de ressources médicales, les hôpitaux et les cliniques spécialisées. Chaque acteur gère des ressources limitées, et notre tâche est de réguler les flux tout en protégeant les sections critiques à l'aide de mutex.

## Choix d'implémentation

Nous avons décidé d'utiliser des **mutex** pour protéger les sections critiques liées à la gestion des stocks et des fonds. Chaque entité (ambulance, fournisseur, hôpital, clinique) est modélisée par un thread, et la concurrence est gérée en synchronisant les accès aux ressources partagées.

#### **Ambulances**

- La méthode `run` a été mise à jour pour s'arrêter dès qu'un `requestStop` est appelé pour ce thread.
- la méthode `sendPatient` a été implémentée pour choisir au hasard un des hôpitaux et y envoyer (si place disponible) 1 patient.

**Hôpitaux et cliniques**

- La méthode `run` a été mise à jour pour s'arrêter dès qu'un `requestStop` est appelé pour ce thread.
- la méthode `sendPatient` a été implémentée pour vérifier si l'hôpital possède assez de fond pour payer les frais d'un nouveau patient et vérifier si assez de lits sont disponibles.
- 

#### **Fournisseurs**

- La méthode `request()` gère les transactions d'achat de ressources en vérifiant la disponibilité des stocks et en mettant à jour les fonds.

- 

## Tests effectués

Nous avons testé le programme dans différents scénarios pour identifier les zones critiques :

- **Tests de concurrence** : Nous avons simulé plusieurs ambulances essayant simultanément d'envoyer des patients vers les hôpitaux, pour s'assurer que les mutex empêchent les erreurs de stock.
- **Stress tests** : Des simulations avec un grand nombre de patients et de ressources ont été effectuées pour vérifier la stabilité du système sous forte charge.
- **Tests unitaires** : Des tests unitaires ont été mis en place pour chaque acteur, en utilisant `gtest`, pour s'assurer que les transactions d'achat et de traitement sont correctement gérées.

## Conclusion

Le laboratoire nous a permis de comprendre les défis de la gestion de la concurrence dans un système multi-threadé. Grâce à l'utilisation des mutex, nous avons réussi à sécuriser les sections critiques et à éviter les problèmes classiques tels que les deadlocks et la corruption de données.