# Mixage.c - ÉTAPE 5b : Mixage (Fusion de signaux)

## Objectif général (Étape 5b du TP)

L'**étape 5b du TP** porte sur le **mixage** : la fusion intelligente de plusieurs signaux audio en un seul.

L'objectif est de combiner un fichier audio existant avec un signal généré (synthèse) en les **mélangeant** proportionnellement, plutôt que de les concaténer simplement (comme en étape 5a).

Le programme `Mixage.c` implémente ce mélange : il génère un bip de 880 Hz et le **mélange** avec les 2 premières secondes de la musique originale, créant un effet de "superposition" des deux sons.

## Concepts fondamentaux

### 1. Mixage audio

Le mixage est le processus de **combiner plusieurs signaux** en un seul en les **additionnant proportionnellement**.

**Formule simple :**
```
Signal_mixé = (Signal_1 + Signal_2) / 2
```

Cela crée un son où on entend simultanément les deux sources.

### 2. Proportions de mélange

Les deux signaux peuvent ne pas avoir le même "poids" dans le mélange.

**Formule générale :**
```
Signal_mixé = (α × Signal_1 + β × Signal_2)
où α + β = 1 (pour préserver le volume global)
```

**Exemple :**
```
Mélange à parts égales : α = 0.5, β = 0.5
Mélange 75/25 : α = 0.75, β = 0.25
Mélange 30/70 : α = 0.30, β = 0.70
```

### 3. Saturation lors du mixage

Un piège courant : deux signaux forts mélangés peuvent créer une **saturation**.

**Exemple :**
```
Signal_1[i] = 200 (fort)
Signal_2[i] = 200 (fort)
Somme : 200 + 200 = 400 (> 255, dépassement!)
Moyen de l'éviter : diviser par 2 → (200 + 200) / 2 = 200 ✓
```

## Fonctionnement détaillé

### Étape 1 : Lecture du fichier source

```c
FILE *f_entree = fopen("son/son1.wav", "rb");
unsigned char header[44];
fread(header, 1, 44, f_entree);

int freq = header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
int data_size = header[40] | (header[41] << 8) | (header[42] << 16) | (header[43] << 24);

unsigned char *musique = malloc(data_size);
fread(musique, 1, data_size, f_entree);
fclose(f_entree);
```

Lecture standard du fichier audio.

### Étape 2 : Boucle de mixage

```c
float freq_bip = 880.0; // Bip plus aigu

for (int i = 0; i < data_size; i++) {
    // On génère le bip uniquement pour les 2 premières secondes
    if (i < freq * 2) {
        double t = (double)i / freq;
        unsigned char bip = (unsigned char)(128 + 127 * sin(2.0 * M_PI * freq_bip * t));
        
        // MIXAGE : (Original + Bip) / 2
        int mix = (musique[i] + bip) / 2;
        musique[i] = (unsigned char)mix;
    }
}
```

#### Analyse détaillée

**Boucle générale :**
```c
for (int i = 0; i < data_size; i++)
```
Itère sur **chaque octet** du fichier audio.

**Condition de durée :**
```c
if (i < freq * 2)
```
Le bip n'est généré que pendant les 2 premières secondes :
- `freq` = 44100 Hz
- `freq * 2` = 88200 octets
- Ces 88200 octets = 2 secondes d'audio (en mono 8-bit)

**Génération du bip :**
```c
double t = (double)i / freq;
unsigned char bip = (unsigned char)(128 + 127 * sin(2.0 * M_PI * freq_bip * t));
```

Génère une onde sinus à 880 Hz (fréquence plus aigu que 440 Hz).
- Identique à l'étape 5a, mais avec fréquence = 880 Hz au lieu de 440 Hz

**Mixage proprement dit :**
```c
int mix = (musique[i] + bip) / 2;
musique[i] = (unsigned char)mix;
```

Combines :
1. `musique[i]` : échantillon original du fichier
2. `bip` : synthèse générée

**Division par 2** : Assure que la somme ne dépasse pas 255

### Étape 3 : Sauvegarde du résultat

```c
FILE *f_sortie = fopen("son/son1_final.wav", "wb");
fwrite(header, 1, 44, f_sortie);
fwrite(musique, 1, data_size, f_sortie);

fclose(f_sortie);
free(musique);
```

**Point important :** L'en-tête ne change **pas** car :
- Même fréquence d'échantillonnage
- Même nombre de canaux
- Même nombre d'octets totaux
- Même durée

On modifie juste les **valeurs** des octets, pas la **structure** du fichier.

## Comparaison : Concaténation vs Mixage

### ÉTAPE 5a : Concaténation (Generation_ondes.c)

```
Fichier original : 88200 octets (2 secondes de musique)
Synthèse générée : 44100 octets (1 seconde de La 440Hz)
Résultat final : 132300 octets (3 secondes totales)

Expérience à l'écoute :
[2 secondes de musique originale] → [1 seconde de bip pur]
```

**Visualisation :**
```
Temporelle :
Musique :  [═════════════════════════════════════════════════════]
Bip :                                                      [═════]
Résultat : [═════════════════════════════════════════════════════][═════]
```

### ÉTAPE 5b : Mixage (Mixage.c)

```
Fichier original : 88200 octets (2 secondes de musique)
Synthèse générée : 44100 octets (1 seconde de La 880Hz, mais mélangée)
Résultat final : 88200 octets (2 secondes, TAILLE IDENTIQUE)

Expérience à l'écoute :
[2 secondes de musique + bip mélangés] (un seul son)
```

**Visualisation :**
```
Temporelle :
Musique :  [═════════════════════════════════════════════════════]
Bip :      [═════════════════════════════════════════════════════]
Résultat : [███████████████████████████████████████════════════] (mélangé pour 2s)
```

## Exemple numérique complet

### Donnée d'entrée

```
Musique original (16 premiers octets) :
[140][100][150][120][130][110][145][115][135][105][155][125][140][100][150][120]

Fréquence : 44100 Hz
```

### Génération du bip

```
Pour i = 0, 1, 2, ..., 87999 (2 secondes) :
  t = i / 44100
  bip[i] = 128 + 127 × sin(2π × 880 × t)

À 880 Hz, 2 secondes = 880 × 2 = 1760 cycles
(beaucoup plus rapide que la musique)
```

### Mixage (premiers 8 octets)

| i | Musique | Bip (880Hz) | Mix | Résultat |
|---|---------|-------------|-----|----------|
| 0 | 140 | 128 | (140+128)/2 | 134 |
| 1 | 100 | 145 | (100+145)/2 | 122 |
| 2 | 150 | 160 | (150+160)/2 | 155 |
| 3 | 120 | 110 | (120+110)/2 | 115 |
| 4 | 130 | 135 | (130+135)/2 | 132 |
| 5 | 110 | 150 | (110+150)/2 | 130 |
| 6 | 145 | 120 | (145+120)/2 | 132 |
| 7 | 115 | 130 | (115+130)/2 | 122 |

### Résultat

```
Fichier original  : [140][100][150][120][130][110][145][115]...
Fichier mixé      : [134][122][155][115][132][130][132][122]...

Le fichier a la même taille, mais les valeurs sont modifiées
```

## Pourquoi 880 Hz et non 440 Hz ?

```
À 880 Hz (octave plus haut) :
- Le bip est plus aigu, plus perceptible dans le mélange
- Moins de confusions avec les fréquences graves de la musique
- Plus facile à entendre "au-dessus" de la musique

À 440 Hz :
- Risque que les graves de la musique noient le bip
- Mélange moins distinct
```

## Notion de perte d'information

Contrairement à la concaténation, le mixage est **irréversible** :

```
Concaténation :
Original + Synthèse → [Original][Synthèse]
Récupération : on peut extraire Original et Synthèse séparément

Mixage :
Original + Synthèse → Mix
Récupération : IMPOSSIBLE d'extraire Original ou Synthèse purs
(on a "écrasé" les valeurs originales)
```

## Contrôle des proportions

Dans ce programme, les deux signaux sont au **50/50** :
```c
int mix = (musique[i] + bip) / 2;
```

Pour une proportion différente, on pourrait faire :
```c
// 75% musique, 25% bip
int mix = (musique[i] * 3 + bip) / 4;

// 10% musique, 90% bip
int mix = (musique[i] + bip * 9) / 10;

// Mais attention : vérifier que la somme ne dépasse pas 255
```

## Points clés de l'Étape 5b

1. **Formule** : `Mix = (Signal_1 + Signal_2) / 2`
2. **Synthèse en temps réel** : Le bip est généré à la volée
3. **Durée partielle** : Bip seulement sur 2 secondes (pas 3 comme étape 5a)
4. **Fréquence plus haute** : 880 Hz pour distinguer du reste
5. **Même taille** : Fichier d'entrée = fichier de sortie
6. **Irréversible** : Impossible de séparer les signaux après mixage
7. **Protection contre saturation** : Division par 2 pour rester dans [0, 255]
8. **Résultat perceptif** : Superposition des deux sons au lieu de succession