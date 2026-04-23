# Generation_ondes.c - ÉTAPE 5a : Synthèse d'onde (Génération de signal)

## Objectif général (Étape 5a du TP)

L'**étape 5a du TP** porte sur la **synthèse d'onde** : la génération de son à partir de formules mathématiques au lieu de lire un fichier existant.

L'objectif est de créer une seconde de son **pur** et **musical** en utilisant une onde **sinusoïdale** à une fréquence bien définie. Le programme génère une note musicale (La, 440 Hz) et l'ajoute à la fin d'une musique existante.

Le programme `Generation_ondes.c` implémente cette synthèse : il génère une onde sinus pure à 440 Hz, puis concatène cette onde avec le fichier audio original.

## Concepts fondamentaux

### 1. L'onde sinusoïdale

Une onde sinusoïdale est la forme d'onde la plus simple et la plus "pure" que l'on puisse générer. Elle est définie par la formule :

$$y(t) = A \sin(2\pi f t + \phi)$$

Où :
- **A** = Amplitude (volume)
- **f** = Fréquence (Hz, cycles par seconde)
- **t** = Temps (secondes)
- **φ** = Phase (décalage, généralement 0)

**Exemple visuel :**
```
Amplitude 127, Fréquence 10 Hz, sur 1 seconde

│   ╱╲      ╱╲
│  ╱  ╲    ╱  ╲
├─╱────╲──╱────╲── (y = 128)
│╱      ╲╱      ╲
│                ╲
0s  0.1s  0.2s  0.3s (10 cycles en 1s = 10 Hz)
```

### 2. Représentation numérique

Un fichier audio numérique est **une discrétisation** de cette courbe continue :
- Au lieu d'avoir une valeur pour chaque instant infinitésimal
- On a une valeur tous les **1 / fréquence d'échantillonnage** secondes

**Exemple :**
```
Fréquence d'échantillonnage : 44100 Hz
Intervalle d'échantillonnage : 1 / 44100 ≈ 22.7 microsecondes

Pour générer 1 seconde :
Nombre d'échantillons = 44100 × 1 = 44100
```

### 3. La note "La" (440 Hz)

En musique, le "La" standard (A4) a une fréquence de **440 Hz**.

C'est la note de référence pour l'accordage des instruments.

**Caractéristiques :**
- Fréquence : 440 Hz
- Durée : 1 seconde (par défaut dans ce programme)
- Amplitude : 127 (maximum en 8-bit, centrée autour de 128)

## Fonctionnement détaillé

### Étape 1 : Lecture du fichier source

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

Lecture standard du fichier audio original (parsing).

### Étape 2 : Préparation de la synthèse

```c
int duree_sec = 1;
int nb_echantillons_synth = freq_orig * duree_sec * canaux_orig;
unsigned char *synthese = malloc(nb_echantillons_synth);
```

**Calculs :**
- **duree_sec** = 1 seconde de synthèse
- **nb_echantillons_synth** = nombre total d'octets à générer
  - Formule : `Fréquence × Durée × Nombre de canaux`
  - Exemple : 44100 × 1 × 2 = 88200 octets

**Raison du "× canaux_orig"** : Si le fichier original est stéréo (2 canaux), on doit générer le même son sur les 2 canaux.

### Étape 3 : Génération de l'onde sinusoïdale

```c
float freq_note = 440.0; // Note "La"
for (int i = 0; i < freq_orig * duree_sec; i++) {
    // Calcul de l'onde sinusoïdale
    double t = (double)i / freq_orig;
    unsigned char valeur = (unsigned char)(128 + 127 * sin(2.0 * M_PI * freq_note * t));

    // On remplit tous les canaux avec la même onde (Mono ou Stéréo)
    for (int c = 0; c < canaux_orig; c++) {
        synthese[i * canaux_orig + c] = valeur;
    }
}
```

#### Calcul détaillé

**Boucle externe :** Itère sur chaque **échantillon** temporel
```c
for (int i = 0; i < freq_orig * duree_sec; i++)
```
- `i` va de 0 à 44100 (pour 1 seconde à 44100 Hz)

**Temps continu :**
```c
double t = (double)i / freq_orig;
```
- Convertit l'indice `i` en temps réel (secondes)
- Exemple : `i = 22050` → `t = 22050 / 44100 = 0.5` secondes

**Formule sinusoïdale :**
```c
unsigned char valeur = (unsigned char)(128 + 127 * sin(2.0 * M_PI * freq_note * t));
```

Décomposition :
- **`2.0 * M_PI * freq_note * t`** = Phase en radians
  - `2π` = conversion de cycles à radians
  - `440 * t` = nombre de cycles à 440 Hz au temps `t`
  - Exemple : `t = 0.5s` → `440 * 0.5 = 220 cycles` → phase = `2π × 220 = ...` radians
  
- **`sin(...)`** = valeur du sinus (entre -1 et 1)

- **`127 * sin(...)`** = amplitude (entre -127 et 127)

- **`128 + ...`** = centre à 128 (format 8-bit unsigned)
  - -127 devient 1
  - 0 devient 128 (silence)
  - +127 devient 255

**Exemple de trace :**
```
i = 0 :
  t = 0 / 44100 = 0
  phase = 2π × 440 × 0 = 0
  sin(0) = 0
  valeur = 128 + 127 × 0 = 128 (silence)

i = 22050 (demi-seconde) :
  t = 22050 / 44100 = 0.5
  phase = 2π × 440 × 0.5 ≈ 1382.3 radians
  sin(1382.3) ≈ -1
  valeur = 128 + 127 × (-1) = 1 (minimum)

i = 44100 (1 seconde) :
  t = 1.0
  phase = 2π × 440 × 1 ≈ 2764.6 radians
  sin(2764.6) ≈ 0
  valeur = 128 + 127 × 0 = 128 (silence)
```

#### Remplir les canaux

```c
for (int c = 0; c < canaux_orig; c++) {
    synthese[i * canaux_orig + c] = valeur;
}
```

Si le fichier est **stéréo** (2 canaux), on duplique la même onde sur les 2 canaux :
```
Synthèse générée : [128][128][...] (mono)
Stéréo final : [128][128][128][128][...] (L=128, R=128, L=128, R=128, ...)
```

### Étape 4 : Préparation de la sauvegarde (fusion)

```c
int data_size_total = data_size_orig + nb_echantillons_synth;

// Mise à jour de la taille dans le header
header[40] = (unsigned char)(data_size_total & 0xFF);
header[41] = (unsigned char)((data_size_total >> 8) & 0xFF);
header[42] = (unsigned char)((data_size_total >> 16) & 0xFF);
header[43] = (unsigned char)((data_size_total >> 24) & 0xFF);

// Mise à jour de la taille totale du fichier
int file_size = data_size_total + 36;
header[4] = (unsigned char)(file_size & 0xFF);
header[5] = (unsigned char)((file_size >> 8) & 0xFF);
header[6] = (unsigned char)((file_size >> 16) & 0xFF);
header[7] = (unsigned char)((file_size >> 24) & 0xFF);
```

**Calculs :**
- **data_size_total** : fusion musique originale + synthèse générée
- **file_size** : taille totale du fichier = data_size_total + 36
  - Les 36 octets correspondent à l'en-tête WAV moins les 8 premiers (taille RIFF)

**Mise à jour de deux champs :**
1. **Octets 4-7** : Taille du fichier RIFF (pour structure générale WAV)
2. **Octets 40-43** : Taille des données audio (pour lecteur)

### Étape 5 : Sauvegarde du fichier final

```c
FILE *f_sortie = fopen("son/son1resultat_final.wav", "wb");
fwrite(header, 1, 44, f_sortie);                      // En-tête modifié
fwrite(musique, 1, data_size_orig, f_sortie);        // Musique originale
fwrite(synthese, 1, nb_echantillons_synth, f_sortie); // Synthèse (La 440Hz)

fclose(f_sortie);
free(musique);
free(synthese);
```

**Ordre dans le fichier final :**
```
Octets 0-43 : En-tête WAV (modifié)
Octets 44 à 44+data_size_orig-1 : Musique originale
Octets 44+data_size_orig à fin : Synthèse (1 seconde de La)
```

**Résultat à l'écoute :** On entend d'abord la musique originale, puis un bip pur (la 440 Hz) pendant 1 seconde.

## Mathématiques de la synthèse

### Cycle complet

Une onde sinus complète = 2π radians = 1 cycle

```
À 440 Hz, en 1 seconde :
- 440 cycles complets
- Chaque cycle = 2π radians
- Phase totale = 440 × 2π ≈ 2764.6 radians
```

### Fréquence de Nyquist

Pour générer une onde de 440 Hz, il faut un **minimum** de 880 échantillons par seconde (théorème de Nyquist).

```
Théorème : Fréquence max = fréquence d'échantillonnage / 2

Exemple :
- Fréquence d'échantillonnage : 44100 Hz
- Max audible : 44100 / 2 = 22050 Hz
- 440 Hz << 22050 Hz ✓ (bien en dessous, synthèse précise)
```

### Validation de la synthèse

Nombre d'échantillons pour générer N cycles d'une fréquence f :
```
Échantillons = fréquence_échantillonnage × (N / f)

Exemple (440 Hz, 44100 Hz d'échantillonnage, 1 seconde) :
Échantillons = 44100 × 1 = 44100
Cycles = 44100 / 44100 × 440 = 440 cycles ✓
```

## Cas pratiques

### Génération d'autres notes

```c
// Do (C4) = 261.63 Hz
float freq_note = 261.63;

// Mi (E4) = 329.63 Hz
float freq_note = 329.63;

// Si (B4) = 493.88 Hz
float freq_note = 493.88;
```

### Durées différentes

```c
// 2 secondes
int duree_sec = 2;
int nb_echantillons_synth = freq_orig * 2 * canaux_orig;

// 0.5 secondes
int duree_sec = 0.5;
int nb_echantillons_synth = freq_orig * 0.5 * canaux_orig;
```

## Points clés de l'Étape 5a

1. **Formule sinus** : $y(t) = 128 + 127 \sin(2\pi f t)$
2. **Conversion i → t** : `t = i / freq_orig`
3. **440 Hz** : Note "La" de référence musicale
4. **Interleaving** : Dupliquer sur chaque canal si stéréo
5. **Header critique** : Augmentation de la taille totale
6. **Concaténation** : Musique originale + synthèse
7. **Mathématiquement pure** : Une onde sinus n'a que la fréquence désirée