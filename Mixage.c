#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

int main() {
    // --- 1. LECTURE DU FICHIER ---
    FILE *f_entree = fopen("son/son1.wav", "rb");
    if (!f_entree) return 1;

    unsigned char header[44];
    fread(header, 1, 44, f_entree);

    int freq = header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
    int data_size = header[40] | (header[41] << 8) | (header[42] << 16) | (header[43] << 24);

    unsigned char *musique = malloc(data_size);
    fread(musique, 1, data_size, f_entree);
    fclose(f_entree);

    // --- 5 & 6. GÉNÉRATION ET MIXAGE ---
    // On va créer un bip et le MIXER sur les 1eres secondes de la musique
    float freq_bip = 880.0; // Un bip plus aigu
    
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

    // --- SAUVEGARDE FINALE ---
    FILE *f_sortie = fopen("son/son1_final.wav", "wb");
    fwrite(header, 1, 44, f_sortie); // Le header ne change pas car la taille est identique
    fwrite(musique, 1, data_size, f_sortie);
    
    fclose(f_sortie);
    free(musique);

    printf("TP Terminé ! Le fichier 'son1_final.wav' contient le mixage.\n");
    return 0;
}