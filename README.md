# projet-tuto
Programmation graphique Master I
Stack technique : C/C++ |  OpenGl - OpenCv - SDL 


Que-ce qui fait ce programme : 

1 - Capturer une vidéo en temps réél par WebCam . 
  
  
2 - Générer des bulles de savon :
    Les bulles de savon sont des images .png.
    Les diamètres des bulles ainsi que les couleurs sont differents et générés aléatoirement au lancement du programme.
    
    
3 - interaction entre les bulles:
    Les mouvements et directions des bulles au lancement sont aléatoires.
    Les mouvements, vitesse et direction sont interactifs aux frottement entre les bulles.
    exemple : si une boule touche une autre cela implique le changement de direction, de vitesse + une explostion.
    
    
4 - interaction avec corps humain:
    Un corps est capturé par la webCam , puis detecté et reconnu grâce à OpenCv, une interaction est donc possible avec les bulles, 
    quand un corps rentre collision / touche ou frotte une ou plusieurs bulles, cela engendre une explosion et implique le changement de la direction et de la vitesse       des bulles.
