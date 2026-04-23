#include <stdio.h>
#include <stdlib.h>
#include <math.h> // Indispensable pour sin()

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

int main() {
    // --- ÉTAPE 1 : LECTURE DU FICHIER SOURCE ---
    FILE *f_entree = fopen("son/son1.wav", "rb");
    if (f_entree == NULL) {
        printf("Erreur : Impossible d'ouvrir le fichier .wav\n");
        return 1;
    }

    unsigned char header[44];
    fread(header, 1, 44, f_entree);

    int canaux_orig = header[22] | (header[23] << 8); 
    int freq_orig = header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
    int bits = header[34] | (header[35] << 8);
    int data_size_orig = header[40] | (header[41] << 8) | (header[42] << 16) | (header[43] << 24);

    unsigned char *musique = malloc(data_size_orig);
    fread(musique, 1, data_size_orig, f_entree);
    fclose(f_entree);

    // --- ÉTAPE 2, 3, 4 : TRAITEMENTS (Résumé pour la sortie finale) ---
    // Pour cet exemple final, on va simplement générer le nouveau son pur
    // et l'ajouter à la suite de la musique traitée.

    // --- ÉTAPE 5 : GÉNÉRATION D'UN "LA" 440Hz (1 seconde) ---
    int duree_sec = 1;
    int nb_echantillons_synth = freq_orig * duree_sec * canaux_orig;
    unsigned char *synthese = malloc(nb_echantillons_synth);

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

    // --- CRÉATION DU FICHIER FINAL (MUSIQUE + SYNTHÈSE) ---
    int data_size_total = data_size_orig + nb_echantillons_synth;
    
    // Mise à jour de la taille dans le header
    header[40] = (unsigned char)(data_size_total & 0xFF);
    header[41] = (unsigned char)((data_size_total >> 8) & 0xFF);
    header[42] = (unsigned char)((data_size_total >> 16) & 0xFF);
    header[43] = (unsigned char)((data_size_total >> 24) & 0xFF);

    // Mise à jour de la taille totale du fichier (Taille Data + 36 octets)
    int file_size = data_size_total + 36;
    header[4] = (unsigned char)(file_size & 0xFF);
    header[5] = (unsigned char)((file_size >> 8) & 0xFF);
    header[6] = (unsigned char)((file_size >> 16) & 0xFF);
    header[7] = (unsigned char)((file_size >> 24) & 0xFF);

    FILE *f_sortie = fopen("son/son1resultat_final.wav", "wb");
    fwrite(header, 1, 44, f_sortie);           // Header
    fwrite(musique, 1, data_size_orig, f_sortie); // Musique originale
    fwrite(synthese, 1, nb_echantillons_synth, f_sortie); // Le "Bip" généré
    
    fclose(f_sortie);
    free(musique);
    free(synthese);

    printf("Succès ! Le fichier 'resultat_final.wav' contient la musique suivie d'un bip de 1s.\n");
    return 0;
}