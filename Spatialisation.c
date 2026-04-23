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

    int canaux_orig = header[22] | (header[23] << 8); 
    int freq_orig = header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
    int bits = header[34] | (header[35] << 8);
    int data_size_orig = header[40] | (header[41] << 8) | (header[42] << 16) | (header[43] << 24);

    unsigned char *musique = malloc(data_size_orig);
    fread(musique, 1, data_size_orig, f_entree);
    fclose(f_entree);

    // --- ÉTAPE 2 & 3 (Simplifiées ici pour se concentrer sur l'étape 4) ---
    // On garde la fréquence d'origine pour cet exemple de spatialisation
    
    // --- ÉTAPE 4 : PASSAGE AU 2.1 (3 CANAUX) ---
    // Si on a 2 canaux, le nombre d'échantillons total est data_size_orig.
    // En ajoutant un 3ème canal, la taille augmente de 50% (on passe de 2 à 3).
    int nb_frames = data_size_orig / 2; // Une "frame" = [L] + [R]
    int data_size_21 = nb_frames * 3; 
    unsigned char *musique_21 = malloc(data_size_21);

    int j = 0;
    for (int i = 0; i < data_size_orig; i += 2) {
        unsigned char gauche = musique[i];
        unsigned char droite = musique[i+1];
        unsigned char caisson = (gauche + droite) / 2; // Moyenne pour le LFE

        musique_21[j++] = gauche;   // Canal L
        musique_21[j++] = droite;   // Canal R
        musique_21[j++] = caisson;  // Canal LFE
    }

    // --- MISE À JOUR DU HEADER POUR LE 2.1 ---
    header[22] = 3; // On passe à 3 canaux
    header[23] = 0;

    // Mise à jour de la taille (DataSize)
    header[40] = (unsigned char)(data_size_21 & 0xFF);
    header[41] = (unsigned char)((data_size_21 >> 8) & 0xFF);
    header[42] = (unsigned char)((data_size_21 >> 16) & 0xFF);
    header[43] = (unsigned char)((data_size_21 >> 24) & 0xFF);

    // Mise à jour du ByteRate (Freq * Canaux * OctetsParEchantillon)
    int nouveau_byte_rate = freq_orig * 3 * (bits / 8);
    header[28] = (unsigned char)(nouveau_byte_rate & 0xFF);
    header[29] = (unsigned char)((nouveau_byte_rate >> 8) & 0xFF);
    header[30] = (unsigned char)((nouveau_byte_rate >> 16) & 0xFF);
    header[31] = (unsigned char)((nouveau_byte_rate >> 24) & 0xFF);

    // --- SAUVEGARDE ---
    FILE *f_sortie = fopen("son/son1sortie3.wav", "wb");
    fwrite(header, 1, 44, f_sortie);
    fwrite(musique_21, 1, data_size_21, f_sortie);
    
    fclose(f_sortie);
    free(musique);
    free(musique_21);
    printf("Fichier son1sortie3 cree avec succes !\n");

    return 0;
}