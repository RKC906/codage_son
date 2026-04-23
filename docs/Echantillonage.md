# Echantillonage.c - ÉTAPE 2 : Réduction de la qualité audio

## Objectif général (Étape 2 du TP)

L'**étape 2 du TP** porte sur la **réduction de la qualité audio**. L'objectif est de **dégrader volontairement le son** en manipulant les chiffres pour créer des versions compressées d'un fichier audio.

Le programme `Echantillonage.c` implémente une des techniques principales : le **sous-échantillonnage** (réduction de la fréquence d'échantillonnage par 2).

## Concept : Qu'est-ce que le sous-échantillonnage ?

### L'idée simple

Un fichier audio est une série de **nombres** représentant l'amplitude du son à des instants réguliers.

**Exemple :**
```
Fréquence : 44100 Hz
Cela signifie : 44100 nombres par seconde
Chaque nombre représente l'intensité du son à ce moment précis
```

Le **sous-échantillonnage** consiste à :
1. Prendre deux échantillons consécutifs
2. Calculer leur moyenne
3. Garder seulement cette moyenne
4. Supprimer les deux originaux

**Résultat :** On a 2 fois moins de données, mais aussi 2 fois moins de détails audio.

### Formule mathématique

Pour diviser la qualité par 2 :
```
Nouveau fichier = fusion des paires (2i, 2i+1)
Formule : echantillon_nouveau[i] = (echantillon_original[2i] + echantillon_original[2i+1]) / 2
```

### Visualisation

```
Fichier ORIGINAL (44100 Hz):
[100][102][98][101][99][103][100][104]...
 ^    ^    ^   ^    ^   ^    ^    ^

Regrouper par paires:
[100, 102]  [98, 101]  [99, 103]  [100, 104]...
   ↓            ↓          ↓          ↓
 moyenne      moyenne    moyenne    moyenne
   101          99.5       101        102

Fichier RÉDUIT (22050 Hz):
[101][99.5][101][102]...
 ^    ^     ^    ^
```

## Fonctionnement détaillé du Echantillonage

### Étape 1 : Ouverture et lecture du fichier source

```c
FILE *f_entree = fopen("entree.wav", "rb");
if (f_entree == NULL) {
    printf("Erreur : Impossible d'ouvrir entree.wav\n");
    return 1;
}
```

Ouvre le fichier à traiter en mode lecture binaire.

### Étape 2 : Parsing du header (récupération des métadonnées)

```c
unsigned char header[44];
fread(header, 1, 44, f_entree);

int canaux = header[22] | (header[23] << 8);
int freq_originale = header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
int bits_per_sample = header[34] | (header[35] << 8);
int data_size_original = header[40] | (header[41] << 8) | (header[42] << 16) | (header[43] << 24);
```

**Explications :**
- Lit les 44 octets d'en-tête (comme en ÉTAPE 1)
- Extrait les quatre informations critiques :

| Variable | Offset | Signification |
|----------|--------|---------------|
| `canaux` | 22-23 | 1 = mono, 2 = stéréo, etc. |
| `freq_originale` | 24-27 | Nombre d'échantillons par seconde |
| `bits_per_sample` | 34-35 | Bits de précision par échantillon (16 bits en général) |
| `data_size_original` | 40-43 | Nombre total d'octets audio |

**Exemple de valeurs typiques :**
```
Canaux : 2
Fréquence originale : 44100 Hz
Bits par échantillon : 16
Taille des données : 176400 octets (pour 1 seconde en stéréo 16-bit)
```

### Étape 3 : Chargement des données audio en mémoire

```c
unsigned char *musique = malloc(data_size_original);
fread(musique, 1, data_size_original, f_entree);
fclose(f_entree);
```

**Explications :**
- `malloc(data_size_original)` : alloue dynamiquement de la mémoire pour stocker TOUS les octets audio
  - Ceci peut être très volumineux (plusieurs MB pour quelques secondes de son)
- `fread(musique, 1, data_size_original, f_entree)` : lit tous les octets audio du fichier dans ce tableau
- `fclose(f_entree)` : ferme le fichier source, on en a fini avec lui

**Important :** Le pointeur `musique` pointe vers un tableau contenant tous les échantillons bruts du fichier.

### Étape 4 : Calcul des nouvelles dimensions

```c
int nouvelle_freq = freq_originale / 2;           // 44100 → 22050 Hz
int data_size_nouveau = data_size_original / 2;   // 176400 → 88200 octets
unsigned char *musique_reduite = malloc(data_size_nouveau);
```

**Explications :**
- Divise la fréquence par 2 : les nouvelles données seront lues à **22050 Hz** au lieu de **44100 Hz**
- Divise la taille des données par 2 : le nouveau fichier sera **2 fois plus petit**
- Alloue un nouveau tableau pour stocker les données réduites

### Étape 5 : Le cœur du traitement - Sous-échantillonnage

```c
for (int i = 0; i < data_size_nouveau; i++) {
    // On prend deux échantillons successifs et on fait la moyenne
    int moyenne = (musique[2 * i] + musique[2 * i + 1]) / 2;
    musique_reduite[i] = (unsigned char)moyenne;
}
```

**Explications détaillées :**

| Itération | Opération | Illustration |
|-----------|-----------|--------------|
| `i = 0` | `moyenne = (musique[0] + musique[1]) / 2` | Prend les deux premiers octets, calcule la moyenne |
| `i = 1` | `moyenne = (musique[2] + musique[3]) / 2` | Prend les deux suivants |
| `i = 2` | `moyenne = (musique[4] + musique[5]) / 2` | Continue... |
| ... | ... | Jusqu'à la fin du fichier |

**Trace d'exécution :**
```
Fichier original : [100][102][98][101][99][103][100][104]
Indices :          [0]  [1]  [2]  [3] [4]  [5]  [6]  [7]

i=0 : moyenne = (100+102)/2 = 101 → musique_reduite[0] = 101
i=1 : moyenne = (98+101)/2 = 99.5 → musique_reduite[1] = 99
i=2 : moyenne = (99+103)/2 = 101 → musique_reduite[2] = 101
i=3 : moyenne = (100+104)/2 = 102 → musique_reduite[3] = 102

Fichier réduit : [101][99][101][102]
```

### Étape 6 : Mise à jour critique de l'en-tête

**IMPORTANT :** Si vous ne mettez PAS à jour l'en-tête, le lecteur audio lira les données incorrectement !

#### 6a. Mise à jour de la fréquence d'échantillonnage (octets 24-27)

```c
header[24] = (unsigned char)(nouvelle_freq & 0xFF);
header[25] = (unsigned char)((nouvelle_freq >> 8) & 0xFF);
header[26] = (unsigned char)((nouvelle_freq >> 16) & 0xFF);
header[27] = (unsigned char)((nouvelle_freq >> 24) & 0xFF);
```

**Explications :**
- On encode `nouvelle_freq` (22050) en **Little Endian** sur 4 octets
- `& 0xFF` isole les 8 bits de poids faible
- `>> 8` décale de 8 bits vers la droite pour extraire l'octet suivant

**Formule générale (Little Endian encoding) :**
```
Pour un nombre N sur 4 octets :
octets[0] = N & 0xFF              (les 8 bits de poids faible)
octets[1] = (N >> 8) & 0xFF       (bits 8-15)
octets[2] = (N >> 16) & 0xFF      (bits 16-23)
octets[3] = (N >> 24) & 0xFF      (bits 24-31, les plus forts)
```

**Exemple avec 22050 :**
```
22050 en binaire : 0000101011000010
Découpé en 4 octets (Little Endian) :
- octets[0] = 11000010 (194)
- octets[1] = 01010110 (86)
- octets[2] = 00000000 (0)
- octets[3] = 00000000 (0)
```

#### 6b. Mise à jour du ByteRate (octets 28-31)

```c
int nouveau_byte_rate = nouvelle_freq * canaux * (bits_per_sample / 8);
header[28] = (unsigned char)(nouveau_byte_rate & 0xFF);
header[29] = (unsigned char)((nouveau_byte_rate >> 8) & 0xFF);
header[30] = (unsigned char)((nouveau_byte_rate >> 16) & 0xFF);
header[31] = (unsigned char)((nouveau_byte_rate >> 24) & 0xFF);
```

**Qu'est-ce que le ByteRate ?**
- C'est le nombre d'octets à lire par seconde
- **Formule :** `ByteRate = Fréquence × Canaux × (Bits par échantillon / 8)`

**Exemple :**
```
Avant :
22050 Hz × 2 canaux × (16 bits / 8) = 22050 × 2 × 2 = 88200 octets/seconde

Après (avec notre réduction par 2) :
22050 Hz × 2 canaux × (16 bits / 8) = 22050 × 2 × 2 = 88200 octets/seconde
```

**Pourquoi c'est important ?** Le lecteur audio utilise cette valeur pour synchroniser la lecture à la bonne vitesse.

#### 6c. Mise à jour de la taille des données (octets 40-43)

```c
header[40] = (unsigned char)(data_size_nouveau & 0xFF);
header[41] = (unsigned char)((data_size_nouveau >> 8) & 0xFF);
header[42] = (unsigned char)((data_size_nouveau >> 16) & 0xFF);
header[43] = (unsigned char)((data_size_nouveau >> 24) & 0xFF);
```

**Explications :**
- Encode la nouvelle taille des données (moitié de la taille originale) en Little Endian
- Le lecteur audio doit savoir exactement combien d'octets il y a à lire

**Exemple :**
```
Avant : 176400 octets
Après : 88200 octets
```

**Conséquence si on n'update pas :**
```
Le lecteur lira 176400 octets
Mais il n'y en a que 88200
→ Lecture de données corrompues, crash potentiel
```

### Étape 7 : Sauvegarde du nouveau fichier

```c
FILE *f_sortie = fopen("sortie.wav", "wb");
fwrite(header, 1, 44, f_sortie);              // En-tête modifié
fwrite(musique_reduite, 1, data_size_nouveau, f_sortie); // Données réduites

printf("Fichier sortie.wav cree avec succes !\n");
fclose(f_sortie);
```

**Étapes :**
1. Crée un nouveau fichier en mode **Write Binary** (`"wb"`)
2. Écrit les 44 octets d'en-tête **modifiés**
3. Écrit les données audio **réduites**
4. Ferme le fichier

**Résultat :** Un fichier WAV valide, lisible par n'importe quel lecteur audio, mais avec une qualité réduite.

### Étape 8 : Nettoyage de la mémoire

```c
free(musique);
free(musique_reduite);
return 0;
```

Libère la mémoire dynamiquement allouée. **C'est crucial** pour éviter une fuite mémoire !

## Comparaison avant/après

### Fichier ORIGINAL
```
Fichier : entree.wav
Fréquence : 44100 Hz
Canaux : 2
Bits/échantillon : 16
Taille données : 176400 octets
Durée (théorique) : 176400 / (44100 × 2 × 2) = 1 seconde
Qualité : CD standard
```

### Fichier RÉDUIT
```
Fichier : sortie.wav
Fréquence : 22050 Hz  (÷ 2)
Canaux : 2
Bits/échantillon : 16
Taille données : 88200 octets  (÷ 2)
Durée (théorique) : 88200 / (22050 × 2 × 2) = 1 seconde
Qualité : Réduite (son plus grave, moins de détails)
```

## Impact perceptif du sous-échantillonnage

| Aspect | Avant | Après |
|--------|-------|-------|
| **Fréquence d'échantillonnage** | 44100 Hz | 22050 Hz |
| **Fréquences audibles max (Nyquist)** | 22050 Hz | 11025 Hz |
| **Son** | Clair, détails fins | Plus grave, brouillé |
| **Taille fichier** | 10 MB (min) | 5 MB (min) |
| **Compatibilité** | CD, Streaming | Téléphone, Podcasts bas débit |

**Note Nyquist :** La fréquence maximale qu'on peut capturer = moitié de la fréquence d'échantillonnage.
- Avant : jusqu'à 22050 Hz (limite de l'audition humaine)
- Après : jusqu'à 11025 Hz (perte des aigus)

## Résumé du flux de données

```
entree.wav (176400 octets)
     ↓
[PARSING] : Récupère métadonnées
     ↓
[CHARGEMENT] : Lit tous les octets audio en mémoire
     ↓
[SOUS-ÉCHANTILLONNAGE] : Moyenne des paires, crée 88200 octets
     ↓
[UPDATE HEADER] : Change freq, byteRate, dataSize
     ↓
[ÉCRITURE] : Sauvegarde en-tête + données réduites
     ↓
sortie.wav (88200 octets) ✓
```

## Points clés de l'Étape 2

1. **Réduction : ÷ 2** Les données sont réduites à la moitié
2. **Moyenne importante** : On ne supprime pas juste la moitié, on fait une moyenne intelligente
3. **Header critique** : 3 champs DOIVENT être mis à jour (fréquence, byteRate, dataSize)
4. **Little Endian** : L'encodage doit être correct ou le fichier sera corrompu
5. **Mémoire** : Allocation/libération dynamique pour traiter des fichiers volumineux
6. **Qualité vs Taille** : Compromis conscient pour réduire la taille aux dépens de la qualité audio
