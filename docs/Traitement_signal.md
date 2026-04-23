# Traitement_signal.c - ÉTAPE 3 : Traitement du Signal (Normalisation)

## Objectif général (Étape 3 du TP)

L'**étape 3 du TP** porte sur le **traitement du signal** (traitement mathématique du son). L'objectif est de manipuler les nombres du son pour :
- **Détecter la saturation** : Trouver les points où le son "tape" contre les limites
- **Normaliser** : Augmenter le volume global du son de manière intelligente sans le distordre

Le programme `Traitement_signal.c` implémente la **normalisation** : il cherche le point le plus fort du son et augmente proportionnellement tout le reste pour que le son soit le plus fort possible sans jamais saturer.

## Concepts fondamentaux

### 1. Saturation

La saturation est un phénomène audio où le son est trop fort et "tape" contre le plafond des valeurs possibles.

**Exemple en 8 bits :**
```
Plage de valeurs : 0 à 255
Silence : 128 (centre)
Son trop fort : au lieu de 200, on force 255 (saturation)
Résultat : distorsion, bruit, perte d'information
```

### 2. Amplitude

L'amplitude est la "force" du son, mesurée comme l'écart par rapport au silence (128).

**Formule :**
```
Amplitude = |valeur - 128|

Exemple :
- Valeur 128 → Amplitude 0 (silence)
- Valeur 200 → Amplitude 72 (son modéré)
- Valeur 50 → Amplitude 78 (son modéré, côté négatif)
- Valeur 255 → Amplitude 127 (maximum, saturation)
```

### 3. Normalisation

La normalisation est le processus qui :
1. Trouve l'amplitude maximale du fichier
2. Calcule un multiplicateur (gain) pour que cette amplitude devienne 127 (le maximum sans saturer)
3. Applique ce gain à **tout** le son

**Résultat :** Le son est plus fort, avec plus de "punch", mais sans distorsion.

**Analogue visuel :** C'est comme augmenter le contraste d'une image en tirant les niveaux de gris vers les extrêmes (noir et blanc).

## Fonctionnement détaillé

### Étape 1 : Lecture du fichier (Parsing)

```c
FILE *f_entree = fopen("son/son1.wav", "rb");
unsigned char header[44];
fread(header, 1, 44, f_entree);

int canaux = header[22] | (header[23] << 8);
int freq_originale = header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
int bits_per_sample = header[34] | (header[35] << 8);
int data_size_original = header[40] | (header[41] << 8) | (header[42] << 16) | (header[43] << 24);

unsigned char *musique = malloc(data_size_original);
fread(musique, 1, data_size_original, f_entree);
fclose(f_entree);
```

Comme dans les étapes précédentes, on récupère les métadonnées et on charge tous les octets audio.

### Étape 2 : Sous-échantillonnage (prétraitement)

```c
int nouvelle_freq = freq_originale / 2;
int data_size_nouveau = data_size_original / 2;
unsigned char *musique_travail = malloc(data_size_nouveau);

for (int i = 0; i < data_size_nouveau; i++) {
    int moyenne = (musique[2 * i] + musique[2 * i + 1]) / 2;
    musique_travail[i] = (unsigned char)moyenne;
}
```

On applique le sous-échantillonnage (ÉTAPE 2) comme préparation. Cela réduit le volume de données à traiter.

### Étape 3 : Normalisation du signal

#### Phase 1 : Trouver l'amplitude maximale

```c
int max_ecart = 0;
for (int i = 0; i < data_size_nouveau; i++) {
    int amplitude = abs(musique_travail[i] - 128);
    if (amplitude > max_ecart) {
        max_ecart = amplitude;
    }
}
```

**Explications :**
- Pour chaque échantillon, calcule l'écart par rapport au centre (128)
- Utilise `abs()` pour ignorer le signe (amplitude = valeur absolue)
- Garde en mémoire le **maximum** trouvé

**Exemple de trace :**
```
Fichier : [50][130][100][200][120][150]
Amplitudes : |50-128|=78  |130-128|=2  |100-128|=28  |200-128|=72  |120-128|=8  |150-128|=22
Max trouvé : 78
```

#### Phase 2 : Calculer le gain

```c
if (max_ecart > 0) {
    float gain = 127.0 / max_ecart;
    // ...
}
```

**Formule :**
```
Gain = 127 / max_amplitude
```

**Explication :**
- 127 est le maximum d'amplitude possible en 8 bits sans saturer
- Si le maximum actuel est 78, le gain = 127 / 78 ≈ 1.628
- On va multiplier tous les amplitudes par 1.628

**Exemple :**
```
max_ecart = 78
gain = 127 / 78 = 1.628

Ancien point fort : 200 (amplitude 72)
Nouveau : 128 + (200-128) × 1.628 = 128 + 72 × 1.628 ≈ 128 + 117 = 245

Ancien point faible : 50 (amplitude 78)
Nouveau : 128 + (50-128) × 1.628 = 128 + (-78) × 1.628 ≈ 128 - 127 = 1
```

#### Phase 3 : Appliquer le gain à chaque échantillon

```c
for (int i = 0; i < data_size_nouveau; i++) {
    int valeur_centree = musique_travail[i] - 128;
    int nouvelle_valeur = (int)(valeur_centree * gain) + 128;
    
    // Sécurité pour ne pas dépasser 0 ou 255
    if (nouvelle_valeur > 255) nouvelle_valeur = 255;
    if (nouvelle_valeur < 0) nouvelle_valeur = 0;
    
    musique_travail[i] = (unsigned char)nouvelle_valeur;
}
```

**Étapes :**
1. **Centrer** la valeur : soustraire 128
2. **Appliquer le gain** : multiplier par le facteur calculé
3. **Re-centrer** : ajouter 128 pour revenir dans la plage 0-255
4. **Limiter** (clipping) : s'assurer qu'on ne dépasse pas 0 ou 255

**Trace complète :**
```
Valeur originale : 200
Centrer : 200 - 128 = 72
Gain : 72 × 1.628 ≈ 117.2
Re-centrer : 117.2 + 128 ≈ 245
Vérifier : 245 < 255 ✓
Résultat : 245
```

### Étape 4 : Mise à jour de l'en-tête

```c
header[24] = (unsigned char)(nouvelle_freq & 0xFF);
header[25] = (unsigned char)((nouvelle_freq >> 8) & 0xFF);
header[26] = (unsigned char)((nouvelle_freq >> 16) & 0xFF);
header[27] = (unsigned char)((nouvelle_freq >> 24) & 0xFF);

int nouveau_byte_rate = nouvelle_freq * canaux * (bits_per_sample / 8);
header[28] = (unsigned char)(nouveau_byte_rate & 0xFF);
header[29] = (unsigned char)((nouveau_byte_rate >> 8) & 0xFF);
header[30] = (unsigned char)((nouveau_byte_rate >> 16) & 0xFF);
header[31] = (unsigned char)((nouveau_byte_rate >> 24) & 0xFF);

header[40] = (unsigned char)(data_size_nouveau & 0xFF);
header[41] = (unsigned char)((data_size_nouveau >> 8) & 0xFF);
header[42] = (unsigned char)((data_size_nouveau >> 16) & 0xFF);
header[43] = (unsigned char)((data_size_nouveau >> 24) & 0xFF);
```

**Trois mises à jour :**
1. **Fréquence** (octets 24-27) : elle a changé lors du sous-échantillonnage
2. **ByteRate** (octets 28-31) : il dépend de la nouvelle fréquence
3. **DataSize** (octets 40-43) : la taille des données a réduit

### Étape 5 : Sauvegarde

```c
FILE *f_sortie = fopen("son/son1sortie2.wav", "wb");
fwrite(header, 1, 44, f_sortie);
fwrite(musique_travail, 1, data_size_nouveau, f_sortie);

fclose(f_sortie);
free(musique);
free(musique_travail);
```

Écrit le fichier final avec header modifié et données normalisées.

## Visualisation de la normalisation

```
AVANT NORMALISATION
│     ╱╲      ╱╲
│    ╱  ╲    ╱  ╲
├─  ╱────╲──╱────╲──  (128 = silence)
│  ╱      ╲╱      ╲
│╱                 ╲

Amplitude max trouvée : 72 (loin du maximum théorique 127)
Volume sonore : faible, audio "mou"

APRÈS NORMALISATION (gain × 1.78)
│  ╱╲            ╱╲
│ ╱  ╲          ╱  ╲
├╱────╲────────╱────╲  (128 = silence)
│      ╲      ╱      ╲╱
│       ╲────╱

Amplitude max : 127 (utilisé le maximum possible)
Volume sonore : optimal, "punchy" sans distorsion
```

## Cas limites

### Fichier déjà saturé
```
Fichier : [255][254][255][200][255]
Max amplitude : 127 (pour 255)
Gain : 127 / 127 = 1.0

Résultat : aucun changement (déjà au maximum)
```

### Fichier trop silencieux
```
Fichier : [126][128][129][127][128]
Max amplitude : 2 (très silencieux)
Gain : 127 / 2 = 63.5

Résultat : amplification massive (× 63.5)
Nouveaux échantillons : [126, 128, 130, 127, 128] × 63.5
```

### Fichier complètement silencieux
```
Fichier : [128][128][128][128][128]
Max amplitude : 0 (silence absolu)

Vérification : if (max_ecart > 0) → FAUX
Résultat : pas de traitement (évite division par zéro)
```

## Points clés de l'Étape 3

1. **Détection** : Trouver l'amplitude maximale du fichier
2. **Calcul du gain** : 127 / max_amplitude
3. **Application proportionnelle** : Multiplier tous les écarts par le gain
4. **Sécurité** : Vérifier les limites 0-255
5. **Sans distorsion** : Le gain est calculé pour juste atteindre le maximum
6. **Mathématiquement simple** : Une boucle et quelques multiplications
7. **Effet perceptif** : Le son devient plus "vivant" et présent