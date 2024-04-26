#include <stdlib.h>                             /* Mirza Etka Topal */
#include <stdio.h>                              /*    03.03.2024    */
#include <string.h>
#define MAX 2048

int read_xline(FILE *stream, int lineno, char buffer[]) {
    int current_row = 2;
    int length = 0;
    char line[MAX];            // Okunan satırı depolamak için bir dizi

    rewind(stream);            // Dosyayı başa sarmak için

    while (fgets(line, MAX, stream) != NULL) {    // Dosyanın sonuna kadar satırları oku
        if (current_row == lineno) {             // İstenilen satıra ulaşıldı mı kontrol et
            strncpy(buffer, line, MAX - 1);      // Satırı kopyala
            buffer[MAX - 1] = '\0';
            length = strlen(buffer);
            break;
        }
        current_row++;
    }

    rewind(stream);

    return length;      // Okunan satırın uzunluğunu döndür
}

int read_xcol(const char *row, int colno, char buffer[]) {
    int current_col = 2;             // Dosyanın başlığının altından başlar
    int i = 0;                       // Hedef sütundaki karakter dizisi dizini
    int inside_quotes = 0;           // Alıntı içinde mi kontrolü

    while (*row != '\0') {
        if (*row == '"') {          // Eğer karakter çift tırnaksa
            inside_quotes = !inside_quotes;       // Alıntı içinde mi kontrolünü tersine çevir
        } else if (*row == ',' && !inside_quotes) {
            current_col++;
        } else if (current_col == colno) {
            buffer[i++] = *row;     // Karakteri hedef dizide depola ve indeksi arttır
        }
        row++;
    }

    buffer[i] = '\0';            // Karakter dizisini sonlandır

    return i;
}
