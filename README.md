# Polytech_TegraTX1
 Projet ET5 sur carte Nvidia TX1


## Spec Timer debug = release ordi perf élevées

 **Parcours entrant et sortant d'une pyramide de taille 3 = 4ms**
 
 ~~Temps de réalisation seuillage de base Timo's Fuction = 12ms sans pyramide, 10ms avec la pyramide niveau 3 et 16ms niveau 4.~~
 
 <ul>
 <li> Fonction Contours version caca pas opti</li>
 <ul>
  <li>avec alloc + désalloc d'images 18 ms</li>
  <li>sans alloc + désalloc d'images 16 ms</li>
  <li>Intérieur </li>
  <ul>
   <li>Extraction et normalisation de l'image actuelle 6ms </li>
   <li>Extraction et normalisation de l'ancienne image 4ms </li>
   <li>Difference entre chaque canal de chaque image et seuil par la moitie de la moyenne de l'image 1.9ms</li>
   <li>Fermeture 0.2 ms</li>
   <li>Seuillage 0.14 ms</li>
   <li>Ou logique entre l'ancienne image et la nouvelle 0.1ms </li>
   <li>Dilatation pour fermer les contours 0.8ms</li>
  </ul>
  </ul>
 <li>Fonction contour opti</li>
 <ul>
  <li>Durée de la fonction 8.8ms</li>
  <li>Intérieur</li>
  <ul>
   <li>Assignation des valeurs 0.01ms</li>
   <li>Extraction et normalisation de l'image actuelle 4.6ms </li>
   <li>Extraction et normalisation de l'ancienne image 2.2ms </li>
   <li>Difference entre chaque canal de chaque image et seuil par la moitie de la moyenne de l'image 0.8ms</li>
   <li>Fermeture 0.2 ms</li>
   <li>Seuillage 0.12 ms</li>
   <li>Ou logique entre l'ancienne image et la nouvelle 0.15ms </li>
   <li>Dilatation pour fermer les contours 0.81ms</li>
  </ul>
 </ul>
 </ul>
