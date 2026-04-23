#include <stdio.h>
#include <stdlib.h>

int main() {
    // 1. OUVERTURE DU FICHIER SOURCE
    // "rb" signifie Read Binary (Lecture Binaire)
    FILE *f_entree = fopen("son/son1.wav", "rb");
    if (f_entree == NULL) {
        printf("Erreur : Impossible d'ouvrir le fichier .wav\n");
        return 1;
    }

    // 2. LECTURE DE L'EN-TÊTE (44 octets) [cite: 174, 182]
    unsigned char header[44];
    fread(header, 1, 44, f_entree);

    // Extraction des informations importantes [cite: 6, 179, 180, 181]
    int canaux = header[22] | (header[23] << 8); 
    int freq_originale = header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
    int bits_per_sample = header[34] | (header[35] << 8);
    int data_size_original = header[40] | (header[41] << 8) | (header[42] << 16) | (header[43] << 24); /* cite: 187 */

    printf("Frequence originale : %d Hz\n", freq_originale);
    printf("Canaux : %d\n", canaux);

    // 3. CHARGEMENT DES DONNÉES AUDIO (DATA CHUNK) [cite: 7, 186]
    unsigned char *musique = malloc(data_size_original);
    fread(musique, 1, data_size_original, f_entree);
    fclose(f_entree);

    // 4. ÉTAPE 2 : SOUS-ÉCHANTILLONNAGE (Division par 2) [cite: 8, 9]
    // On divise la fréquence par 2, donc on aura 2 fois moins de données
    int nouvelle_freq = freq_originale / 2;
    int data_size_nouveau = data_size_original / 2;
    unsigned char *musique_reduite = malloc(data_size_nouveau);

    for (int i = 0; i < data_size_nouveau; i++) {
        // On prend deux échantillons successifs et on fait la moyenne 
        // musique[2*i] est l'échantillon actuel, musique[2*i+1] est le suivant
        int moyenne = (musique[2 * i] + musique[2 * i + 1]) / 2;
        musique_reduite[i] = (unsigned char)moyenne;
    }

    // 5. MISE À JOUR DE L'EN-TÊTE POUR LE NOUVEAU FICHIER [cite: 10, 20]
    // Il faut changer la fréquence (offset 24)
    header[24] = (unsigned char)(nouvelle_freq & 0xFF);
    header[25] = (unsigned char)((nouvelle_freq >> 8) & 0xFF);
    header[26] = (unsigned char)((nouvelle_freq >> 16) & 0xFF);
    header[27] = (unsigned char)((nouvelle_freq >> 24) & 0xFF);

    // Il faut changer le ByteRate (Freq * Canaux * OctetsParEchantillon) [cite: 33, 184]
    int nouveau_byte_rate = nouvelle_freq * canaux * (bits_per_sample / 8);
    header[28] = (unsigned char)(nouveau_byte_rate & 0xFF);
    header[29] = (unsigned char)((nouveau_byte_rate >> 8) & 0xFF);
    header[30] = (unsigned char)((nouveau_byte_rate >> 16) & 0xFF);
    header[31] = (unsigned char)((nouveau_byte_rate >> 24) & 0xFF);

    // Il faut changer la taille des données (offset 40) [cite: 20, 187]
    header[40] = (unsigned char)(data_size_nouveau & 0xFF);
    header[41] = (unsigned char)((data_size_nouveau >> 8) & 0xFF);
    header[42] = (unsigned char)((data_size_nouveau >> 16) & 0xFF);
    header[43] = (unsigned char)((data_size_nouveau >> 24) & 0xFF);

    // 6. SAUVEGARDE DU RÉSULTAT [cite: 4, 21]
    FILE *f_sortie = fopen("son/son1sortie.wav", "wb"); // "wb" pour Write Binary
    fwrite(header, 1, 44, f_sortie);            // On écrit l'en-tête modifié
    fwrite(musique_reduite, 1, data_size_nouveau, f_sortie); // On écrit le son réduit
    
    printf("Fichier son1sortie.wav cree avec succes dans le dossier son!\n");

    // Nettoyage de la mémoire
    fclose(f_sortie);
    free(musique);
    free(musique_reduite);

    return 0;
}