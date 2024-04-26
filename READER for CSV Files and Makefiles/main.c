#include <stdio.h>                                    /* Mirza Etka Topal */
#define MAX 2048                                      /*    03.03.2024    */

int read_xline(FILE *stream, int lineno, char buffer[]);
int read_xcol(const char *row, int colno, char buffer[]);
const char *fname = "Food_contents_2019.csv";

int main() {
    int col, row;
    printf("Satir ve Sutunu giriniz:");
    scanf("%d %d", &row, &col);

    int target_col = col + 1;   
    int target_row = row + 1; 
    char buffer[MAX];
    char line[MAX];
    FILE *stream = fopen(fname, "r");       /* Dosyayı aç */

    int length = read_xline(stream, target_row, buffer);        /* Hedef satırı oku */
    length = read_xcol(buffer, target_col, line);               /* Hedef sütunu oku */
    printf("Bulunan deger: %s\n", line);

    fclose(stream);         /* Dosyayı kapat */
    return 0;
}
