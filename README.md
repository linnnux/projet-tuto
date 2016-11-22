# projet-tuto
projet tureuré m1

Que ce qui fait ce programme : 

1 - Capture de vidéo en temps réél par WebCam . ( OpenGl - OpenCv - SDL )
2 - Génèration de bulles de savons :
    Les bulles de savons sont sous forme .png qui prend la transparence en charge.
    Les diamètres des bulles ainsi que les couleurs sont differentes et générés au débit de programme aléatoirement .
3 - interaction entre les bulles:
    Les mouvements et directions des bulles au début de programme sont aléatoires.
    Les mouvements, vitesse et direction sont interactifs aux frottement entre les bulles.
    exemple : si une boule touche une autre le changement de direction et de vitesse plus une explostion sont réalisés.
4 - interaction avec corps humain:
    Un corps est capturé par la webCam , puis detecté et reconnu grace à OpenCv , une interaction est donc possible avec les bulles, 
    quand un corps rentre collision avec une ou plusieurs bulles ça engendre une explosion de la ou les bulles et ça affecte le mouvement,
    la direction et la vitesse de autres .
