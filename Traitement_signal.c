#include <stdio.h>
#include <stdlib.h>

int main() {
    // --- ÉTAPE 1 : LECTURE ---
    FILE *f_entree = fopen("son/son1.wav", "rb");
    if (f_entree == NULL) {
        printf("Erreur : Impossible d'ouvrir le fichier .wav\n");
        return 1;
    }

    unsigned char header[44];
    fread(header, 1, 44, f_entree);

    int canaux = header[22] | (header[23] << 8); 
    int freq_originale = header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
    int bits_per_sample = header[34] | (header[35] << 8);
    int data_size_original = header[40] | (header[41] << 8) | (header[42] << 16) | (header[43] << 24);

    unsigned char *musique = malloc(data_size_original);
    fread(musique, 1, data_size_original, f_entree);
    fclose(f_entree);

    // --- ÉTAPE 2 : SOUS-ÉCHANTILLONNAGE ---
    int nouvelle_freq = freq_originale / 2;
    int data_size_nouveau = data_size_original / 2;
    unsigned char *musique_travail = malloc(data_size_nouveau);

    for (int i = 0; i < data_size_nouveau; i++) {
        int moyenne = (musique[2 * i] + musique[2 * i + 1]) / 2;
        musique_travail[i] = (unsigned char)moyenne;
    }

    // --- ÉTAPE 3 : NORMALISATION ---
    // 1. Trouver l'amplitude maximale (l'écart max par rapport au silence de 128)
    int max_ecart = 0;
    for (int i = 0; i < data_size_nouveau; i++) {
        int amplitude = abs(musique_travail[i] - 128); 
        if (amplitude > max_ecart) {
            max_ecart = amplitude;
        }
    }

    // 2. Appliquer le gain
    if (max_ecart > 0) {
        // On calcule le multiplicateur pour que max_ecart devienne 127 (le max en 8 bits)
        float gain = 127.0 / max_ecart;
        for (int i = 0; i < data_size_nouveau; i++) {
            int valeur_centree = musique_travail[i] - 128;
            int nouvelle_valeur = (int)(valeur_centree * gain) + 128;
            
            // Sécurité pour ne pas dépasser 0 ou 255
            if (nouvelle_valeur > 255) nouvelle_valeur = 255;
            if (nouvelle_valeur < 0) nouvelle_valeur = 0;
            
            musique_travail[i] = (unsigned char)nouvelle_valeur;
        }
    }

    // --- MISE À JOUR DU HEADER ---
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

    // --- SAUVEGARDE ---
    FILE *f_sortie = fopen("son/son1sortie2.wav", "wb");
    fwrite(header, 1, 44, f_sortie);
    fwrite(musique_travail, 1, data_size_nouveau, f_sortie);
    
    fclose(f_sortie);
    free(musique);
    free(musique_travail);
    printf("Traitement de l'etape 3 termine !\n");

    return 0;
}