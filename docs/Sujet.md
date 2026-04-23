Ce TP porte sur la manipulation binaire, ce qui signifie que vous allez traiter un fichier audio non pas comme un morceau de musique, mais comme une suite de nombres (des octets). L'objectif est de modifier ces nombres pour transformer le son sans utiliser de logiciel d'édition tout fait.

Voici une explication détaillée des étapes et des concepts clés :
1. Concepts de base indispensables

Avant de commencer, il faut comprendre trois termes techniques :

    L'Octet (Byte) : C'est l'unité de base. Un fichier est une longue liste d'octets (valeurs entre 0 et 255).

Le Header (En-tête) : Ce sont les 44 premiers octets du fichier. Ils ne contiennent pas de musique, mais la "carte d'identité" du son (vitesse, nombre de canaux, etc.).

Little Endian : C'est une façon de lire les nombres. Si un nombre utilise deux octets, le plus petit morceau est écrit en premier. C'est comme si, pour écrire "123", on écrivait "321".

2. Étape 1 : Le Parsing (L'analyse)

Il s'agit de "découper" votre fichier pour séparer la carte d'identité de la musique elle-même.

    Lecture de l'en-tête : Vous devez trouver dans les premiers octets des informations comme la Fréquence d'échantillonnage (nombre de "points" de son par seconde, ex: 44100 Hz).

Identification des données : Vous devez repérer le mot "data" dans le fichier. Tout ce qui vient après ce mot correspond aux échantillons de son réels.

3. Étape 2 : Réduction de la qualité

Ici, vous allez dégrader volontairement le son en manipulant les chiffres.

    Sous-échantillonnage : Pour diviser la qualité par 2, vous prenez deux échantillons de son consécutifs et vous en faites la moyenne pour n'en garder qu'un seul.

    Attention : Si vous supprimez la moitié des données, vous devez modifier l'en-tête (le Header) pour prévenir l'ordinateur que le fichier est maintenant deux fois plus petit.

Changement de quantification : C'est comme passer d'une image en couleur à une image avec seulement 4 couleurs. On réduit la précision de l'amplitude du son (par exemple en passant de 16 bits à 8 bits).

4. Étape 3 : Traitement du Signal (Mathématiques)

    Saturation : Si le son est trop fort, il "tape" contre le plafond des valeurs autorisées (écrêtage). Vous devez trouver ces valeurs limites et les réduire doucement pour "dé-saturer".

Normalisation : Vous cherchez le point le plus fort du son et vous augmentez tout le reste de façon proportionnelle pour que le son soit le plus fort possible sans jamais saturer.

5. Étape 4 & 5 : Spatialisation et Synthèse

    Stéréo vers 2.1 : Un fichier stéréo a deux canaux (Gauche et Droite). On vous demande de créer un troisième canal pour les basses (LFE) en faisant la moyenne de la Gauche et de la Droite.

Génération d'onde : Au lieu de lire un fichier existant, vous devez utiliser une formule mathématique (sinus) pour créer une seconde de "La" (440 Hz) pure.

[Image d'une onde sinusoïdale pure]
6. Résumé de la méthode de travail

    Ouvrir le fichier .wav en mode binaire ("rb" en Python).

Stocker tout dans un tableau d'octets.

Modifier les valeurs selon l'exercice (moyennes, divisions, tris).

Recalculer l'en-tête (la taille totale a souvent changé !).

Enregistrer le nouveau tableau dans un nouveau fichier .wav pour l'écouter.