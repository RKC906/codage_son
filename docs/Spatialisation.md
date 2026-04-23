# Spatialisation.c - ÉTAPE 4 : Spatialisation (Stéréo vers 2.1)

## Objectif général (Étape 4 du TP)

L'**étape 4 du TP** porte sur la **spatialisation** : la transformation de la configuration spatiale du son. L'objectif est de créer un fichier **2.1** (trois canaux) à partir d'un fichier **stéréo** (deux canaux) en générant un **canal LFE** (Low Frequency Effects) pour les basses.

Le programme `Spatialisation.c` implémente cette transformation : il prend un audio stéréo et ajoute un troisième canal contenant la moyenne des canaux gauche et droit (le "caisson de basses").

## Concepts fondamentaux

### 1. Stéréo (2 canaux)

La stéréo divise le son en deux canaux :
- **Canal L (Gauche)** : haut-parleur gauche
- **Canal R (Droite)** : haut-parleur droit

**Structure des données :**
```
Fichier stéréo 16-bit (2 octets par canal) :
[L0][R0][L1][R1][L2][R2]...
└──┬──┘ └──┬──┘ └──┬──┘
Frame 0   Frame 1   Frame 2
```

Chaque "frame" (instant dans le temps) contient un échantillon pour le canal gauche ET un pour le canal droit.

### 2. Configuration 2.1 (3 canaux)

Le système **2.1** ajoute un **caisson de basses** :
- **2** : 2 canaux principaux (gauche + droite)
- **.1** : 1 canal pour les fréquences basses (LFE = Low Frequency Effects)

**Structure des données :**
```
Fichier 2.1 (2 octets par canal) :
[L0][R0][LFE0][L1][R1][LFE1][L2][R2][LFE2]...
└────┬──────┘ └────┬──────┘ └────┬──────┘
  Frame 0       Frame 1       Frame 2
```

Chaque frame contient 3 échantillons : gauche, droite, et caisson de basses.

### 3. Caisson de basses (LFE)

Le canal LFE contient les **fréquences basses** (généralement < 120 Hz).

**Génération simple :** 
```
LFE = (L + R) / 2
```

**Raison :** Les basses sont largement omnidirectionnelles (pas besoin de stéréo), donc faire une moyenne de gauche+droite préserve le contenu grave.

## Fonctionnement détaillé

### Étape 1 : Lecture du fichier stéréo

```c
FILE *f_entree = fopen("son/son1.wav", "rb");
unsigned char header[44];
fread(header, 1, 44, f_entree);

int canaux_orig = header[22] | (header[23] << 8);
int freq_orig = header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
int bits = header[34] | (header[35] << 8);
int data_size_orig = header[40] | (header[41] << 8) | (header[42] << 16) | (header[43] << 24);

unsigned char *musique = malloc(data_size_orig);
fread(musique, 1, data_size_orig, f_entree);
fclose(f_entree);
```

Lecture standard : métadonnées et données audio.

**Hypothèse :** Le fichier d'entrée est en **stéréo** (2 canaux).

### Étape 2 : Calcul des nouvelles dimensions

```c
int nb_frames = data_size_orig / 2;
int data_size_21 = nb_frames * 3;
unsigned char *musique_21 = malloc(data_size_21);
```

**Explications :**
- **nb_frames** : nombre d'instants temporels
  - En stéréo 8-bit : chaque frame = 2 octets (1 pour L, 1 pour R)
  - Donc : `nb_frames = data_size_orig / 2`
  
- **data_size_21** : nouvelle taille
  - En 2.1 8-bit : chaque frame = 3 octets (1 pour L, 1 pour R, 1 pour LFE)
  - Donc : `data_size_21 = nb_frames * 3`

**Augmentation de taille :**
```
Avant : data_size_orig
Après : data_size_orig × 1.5
Exemple : 88200 octets → 132300 octets
```

### Étape 3 : Conversion stéréo → 2.1

```c
int j = 0;
for (int i = 0; i < data_size_orig; i += 2) {
    unsigned char gauche = musique[i];
    unsigned char droite = musique[i+1];
    unsigned char caisson = (gauche + droite) / 2;

    musique_21[j++] = gauche;
    musique_21[j++] = droite;
    musique_21[j++] = caisson;
}
```

**Trace détaillée :**

| Itération | `i` | `gauche` | `droite` | `caisson` | `musique_21[j]` |
|-----------|-----|----------|----------|-----------|-----------------|
| 0 | 0 | 100 | 110 | 105 | [100, 110, 105, ...] |
| 1 | 2 | 120 | 130 | 125 | [..., 120, 130, 125, ...] |
| 2 | 4 | 90 | 80 | 85 | [..., 90, 80, 85, ...] |

**Explication du processus :**

1. **Boucle par frames** : `i += 2` (on avance de 2 octets à la fois)
2. **Extraire L et R** :
   - `gauche = musique[i]` (premier octet de la frame)
   - `droite = musique[i+1]` (deuxième octet de la frame)
3. **Générer LFE** :
   - `caisson = (gauche + droite) / 2`
4. **Remplir le nouveau buffer** :
   - Écrire L, R, puis LFE dans `musique_21`
   - `j` avance de 3 à chaque iteration

**Visualisation du remappage :**

```
ENTRÉE (Stéréo) :
[L0][R0] [L1][R1] [L2][R2] [L3][R3]

SORTIE (2.1) :
[L0][R0][LFE0] [L1][R1][LFE1] [L2][R2][LFE2] [L3][R3][LFE3]

Où LFE0 = (L0 + R0) / 2, etc.
```

### Étape 4 : Mise à jour du header pour 2.1

#### 4a. Nombre de canaux (octets 22-23)

```c
header[22] = 3;
header[23] = 0;
```

**Changement :**
- Avant : 2 canaux (stéréo)
- Après : 3 canaux (2.1)

#### 4b. Taille des données (octets 40-43)

```c
header[40] = (unsigned char)(data_size_21 & 0xFF);
header[41] = (unsigned char)((data_size_21 >> 8) & 0xFF);
header[42] = (unsigned char)((data_size_21 >> 16) & 0xFF);
header[43] = (unsigned char)((data_size_21 >> 24) & 0xFF);
```

Encode la **nouvelle taille** (1.5 × l'original) en Little Endian.

**Exemple :**
```
Avant : 88200 octets
Après : 132300 octets (88200 × 1.5)
```

#### 4c. ByteRate (octets 28-31)

```c
int nouveau_byte_rate = freq_orig * 3 * (bits / 8);
header[28] = (unsigned char)(nouveau_byte_rate & 0xFF);
header[29] = (unsigned char)((nouveau_byte_rate >> 8) & 0xFF);
header[30] = (unsigned char)((nouveau_byte_rate >> 16) & 0xFF);
header[31] = (unsigned char)((nouveau_byte_rate >> 24) & 0xFF);
```

**Formule mise à jour :**
```
Ancien : Fréquence × 2 canaux × (Bits / 8)
Nouveau : Fréquence × 3 canaux × (Bits / 8)
```

**Exemple avec 44100 Hz, 8-bit :**
```
Avant : 44100 × 2 × (8/8) = 88200 octets/seconde
Après : 44100 × 3 × (8/8) = 132300 octets/seconde (× 1.5)
```

### Étape 5 : Sauvegarde du fichier 2.1

```c
FILE *f_sortie = fopen("son/son1sortie3.wav", "wb");
fwrite(header, 1, 44, f_sortie);
fwrite(musique_21, 1, data_size_21, f_sortie);

fclose(f_sortie);
free(musique);
free(musique_21);
```

Écrit le nouveau fichier 2.1 avec l'en-tête modifié.

## Exemple complet

### Fichier d'entrée (Stéréo)
```
Fréquence : 44100 Hz
Canaux : 2 (L, R)
Bits : 8
Durée : 1 seconde
Data size : 88200 octets
```

### Fichier de sortie (2.1)
```
Fréquence : 44100 Hz
Canaux : 3 (L, R, LFE)
Bits : 8
Durée : 1 seconde (même durée)
Data size : 132300 octets (× 1.5)
Header[22] : 3 (au lieu de 2)
Header[40-43] : 132300 en Little Endian
Header[28-31] : 132300 (nouveau ByteRate)
```

### Exemple de transformation audio

```
STÉRÉO ORIGINAL (8 échantillons par canal)
Gauche  : [100][110][120][130][90][80][100][110]
Droite  : [110][120][130][140][80][70][110][120]

CALCUL DU LFE (moyenne)
LFE     : [105][115][125][135][85][75][105][115]

FICHIER 2.1 FINAL (interleaved)
[100][110][105] [110][120][115] [120][130][125] [130][140][135] ...
[90][80][85]    [80][70][75]    [100][110][105] [110][120][115]
```

## Cas pratiques

### Entrée : Stéréo CD (16-bit)
```
44100 Hz, 2 canaux, 16-bit, 1 minute
Original : 44100 × 60 × 2 × 2 = 10,584,000 octets
Sortie 2.1 : 10,584,000 × 1.5 = 15,876,000 octets
```

### Entrée : Mono (pas applicable)
```
Ce programme suppose une entrée stéréo
Si l'entrée est mono (1 canal), le résultat sera incorrect
```

## Points clés de l'Étape 4

1. **Augmentation de 50%** : Passer de 2 à 3 canaux augmente la taille de × 1.5
2. **Interleaving** : Les canaux sont entrelacés frame par frame (L, R, LFE, L, R, LFE, ...)
3. **LFE = Moyenne** : Le caisson de basses est simplement la moyenne L+R
4. **Header critique** : 3 champs doivent être modifiés (canaux, dataSize, byteRate)
5. **Même fréquence** : La fréquence d'échantillonnage ne change pas
6. **Même durée** : Le temps total reste identique (juste plus d'informations spatiales)
7. **Compatible DAC** : Beaucoup de systèmes audio supports 2.1 nativement