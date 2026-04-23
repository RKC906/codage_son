#include <stdio.h>
#include <stdlib.h>

int main() {
    // 1. Ouvrir le fichier
    FILE *fichier = fopen("son/son1.wav", "rb");
    if (fichier == NULL) {
        printf("Erreur : Impossible d'ouvrir le fichier.\n");
        return 1;
    }

    // 2. Créer un tableau pour stocker l'en-tête (44 octets)
    unsigned char header[44];
    
    // 3. Lire les 44 premiers octets
    fread(header, sizeof(unsigned char), 44, fichier);

    // 4. Extraire les informations (en tenant compte du Little Endian)
    // On combine les octets pour recréer les nombres
    int canaux = header[22]; 
    // Pour la fréquence, on combine 4 octets (24, 25, 26, 27)
    int freq = header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
    int bits = header[34];

    printf("--- Infos du fichier ---\n");
    printf("Canaux : %d\n", canaux);
    printf("Frequence : %d Hz\n", freq);
    printf("Bits : %d bits\n", bits);

    fclose(fichier);
    return 0;
}