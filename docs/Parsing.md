# Parsing.c - ÉTAPE 1 : L'Analyse du fichier WAV

## Objectif général (Étape 1 du TP)

L'**étape 1 du TP** porte sur le **Parsing** (l'analyse). Son but est de "découper" votre fichier WAV pour séparer la **carte d'identité** (l'en-tête) de la **musique elle-même** (les données audio).

Le programme `Parsing.c` est une implémentation simple de cette étape : il lit et affiche les métadonnées critiques d'un fichier audio WAV.

## Concepts fondamentaux

### 1. L'Octet (Byte)
- C'est l'unité de base de tout fichier
- Un octet = 8 bits = une valeur entre **0 et 255**
- Un fichier entier n'est qu'une longue suite d'octets

### 2. Le Header (En-tête)
- Les **44 premiers octets** du fichier WAV
- Ils NE contiennent PAS de musique
- Ils contiennent la **"carte d'identité"** du son :
  - À quelle vitesse le son a-t-il été enregistré ? (fréquence d'échantillonnage)
  - C'est mono ou stéréo ? (nombre de canaux)
  - Combien de bits de précision pour chaque échantillon ?
  - Quelle est la taille totale des données audio ?

### 3. Little Endian (Format de stockage)
C'est une convention particulière pour stocker les nombres qui utilisent plusieurs octets.

**Exemple intuif :** Si vous voulez écrire "123" en Little Endian, vous écrivez les chiffres à l'envers : "321"

**En binaire :** Un nombre sur 4 octets `[a][b][c][d]` stocké en Little Endian signifie :
- L'octet `a` est le moins significatif (le "3")
- L'octet `d` est le plus significatif (le "1")

**Formule de combinaison :**
```
nombre = a | (b << 8) | (c << 16) | (d << 24)
```
- `a` reste tel quel
- `b` est décalé de 8 bits vers la gauche (multiplié par 256)
- `c` est décalé de 16 bits (multiplié par 65536)
- `d` est décalé de 24 bits (multiplié par 16777216)

## Fonctionnement détaillé du Parsing

### Étape 1 : Ouverture du fichier

```c
FILE *fichier = fopen("son/son1.wav", "rb");
if (fichier == NULL) {
    printf("Erreur : Impossible d'ouvrir le fichier.\n");
    return 1;
}
```

**Explications :**
- `fopen("son/son1.wav", "rb")` ouvre le fichier `son1.wav` situé dans le dossier `son/`
- Mode `"rb"` = **Read Binary** (Lecture Binaire)
  - **"r"** = lecture (read), pas écriture
  - **"b"** = mode binaire, lit les octets bruts sans transformation
- La vérification `if (fichier == NULL)` s'assure que le fichier a bien pu être ouvert
  - Si le fichier n'existe pas ou n'est pas accessible, on affiche une erreur et on quitte

### Étape 2 : Allocation et lecture de l'en-tête

```c
unsigned char header[44];
fread(header, sizeof(unsigned char), 44, fichier);
```

**Explications :**
- `unsigned char header[44]` : crée un tableau de 44 octets non signés
  - Type `unsigned char` = valeurs de 0 à 255
  - Index 0 à 43 (soit 44 éléments)
- `fread(header, sizeof(unsigned char), 44, fichier)` :
  - Lit dans le fichier
  - Lit des blocs de taille `sizeof(unsigned char)` (1 octet)
  - Lit exactement 44 blocs (donc 44 octets)
  - Stocke le résultat dans le tableau `header`

**Résultat :** Le tableau `header` contient maintenant les 44 premiers octets du fichier

### Étape 3 : Extraction des métadonnées

Le fichier WAV suit une structure standardisée. Les informations importantes se trouvent toujours aux mêmes positions (offsets) :

#### Structure complète du header (44 octets)

| Octets | Information | Description |
|--------|-------------|-------------|
| 0-3 | "RIFF" | Identifiant RIFF (toujours "RIFF") |
| 4-7 | Taille du fichier - 8 | Taille totale en octets |
| 8-11 | "WAVE" | Format (toujours "WAVE") |
| 12-15 | "fmt " | Sous-chunk format |
| 16-19 | Taille du sous-chunk | Généralement 16 |
| 20-21 | Format audio | 1 = PCM (compression) |
| **22-23** | **Nombre de canaux** | 1 = mono, 2 = stéréo |
| **24-27** | **Fréquence d'échantillonnage** | Ex: 44100, 48000 Hz |
| 28-31 | Byte rate | Fréquence × Canaux × (Bits/8) |
| 32-33 | Block align | Canaux × (Bits/8) |
| **34-35** | **Bits par échantillon** | Ex: 16 bits (CD quality) |
| 36-39 | "data" | Marque le début des données audio |
| **40-43** | **Taille des données audio** | Nombre d'octets contenant le son |

#### Extraction dans le code :

```c
int canaux = header[22];
```
- `header[22]` contient le nombre de canaux
- Pour la majorité des fichiers, c'est 1 (mono) ou 2 (stéréo)
- Pas besoin de combiner avec Little Endian car c'est sur 1 octet

```c
int freq = header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
```
- La fréquence occupe 4 octets (24, 25, 26, 27)
- **Application du Little Endian :**
  - Prendre `header[24]` (le plus petit octet)
  - Ajouter `header[25]` × 256
  - Ajouter `header[26]` × 65536
  - Ajouter `header[27]` × 16777216
- **Résultat typique :** 44100 Hz (CD quality) ou 48000 Hz (audio professionnel)

```c
int bits = header[34];
```
- Le nombre de bits par échantillon (sur 2 octets, mais les valeurs standards sont sur 1 octet)
- Typiquement : 16 bits (CD audio) ou 8 bits (téléphone)

### Étape 4 : Affichage des résultats

```c
printf("--- Infos du fichier ---\n");
printf("Canaux : %d\n", canaux);
printf("Frequence : %d Hz\n", freq);
printf("Bits : %d bits\n", bits);
```

Affiche de manière lisible les trois informations principales du fichier audio.

### Étape 5 : Fermeture du fichier

```c
fclose(fichier);
```
Libère les ressources associées à la lecture du fichier.

## Exemple concret

Supposons qu'on analyse un fichier audio CD classique enregistré en stéréo :

```
--- Infos du fichier ---
Canaux : 2
Frequence : 44100 Hz
Bits : 16 bits
```

**Signification :**
- **2 canaux** = stéréo (gauche et droite)
- **44100 Hz** = 44100 échantillons par seconde (le standard CD)
- **16 bits** = 2^16 = 65536 niveaux de précision différents
- **Débit binaire total** = 44100 × 2 × (16/8) = **176 400 octets/seconde**
- **Durée d'1 minute** = 176400 × 60 ≈ **10.6 MB**

## Points clés du Parsing

1. **Mode binaire obligatoire** : `"rb"` lit les octets exacts, pas des caractères texte
2. **Little Endian critique** : Sans la bonne combinaison, vous lirez des nombres complètement faux
3. **Positions fixes** : Les offsets (22, 24, 34) sont toujours les mêmes dans un WAV standard
4. **Lecture seule** : Cette étape NE modifie PAS le fichier, juste l'analyse
5. **Fondation** : Le Parsing est essentiel pour comprendre le fichier avant de le transformer (étapes 2-5)
