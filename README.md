# Polytech_TegraTX1
 Projet ET5 sur carte Nvidia TX1


## Spec Timer debug = release ordi perf élevées

 **Parcours entrant et sortant d'une pyramide de taille 3 = 4ms**
 
 ~~Temps de réalisation seuillage de base Timo's Fuction = 12ms sans pyramide, 10ms avec la pyramide niveau 3 et 16ms niveau 4.~~
 
 <ul>
 <li> Fonction Contours</li>
 <ul>
  <li>avec alloc + désalloc d'images 18 ms</li>
  <li>sans alloc + désalloc d'images 16 ms</li>
  <li>Intérieur </li>
  <ul>
   <li>Extraction et normalisation de l'image actuelle 6ms </li>
   <li>Extraction et normalisation de l'ancienne image 4ms </li>
   <li>Difference entre chaque canal de chaque image et seuil par la moitie de la moyenne de l'image 1.9ms</li>
   <li>Fermeture 0.2 ms</li>
   <li>Seuillage 0.1 ms</li>
   <li>Ou logique entre l'ancienne image et la nouvelle 0.1ms </li>
   <li>Dilatation pour fermer les contours 0.8ms</li>
  </ul>
  </ul>
 </ul>
