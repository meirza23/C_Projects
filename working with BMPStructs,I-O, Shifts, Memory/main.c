#include <stdio.h>                              /*Mirza Etka Topal*/
#include <stdlib.h>                             /*   12.03.2024  */
#include <string.h>
#include "lab4bmp.h"

/**
 * Belirtilen dosya adıyla bir görüntü dosyasını açar,
 * bellek tahsis eder ve bu bellekte Image yapısının üyelerine atar,
 * ardından bu Image'i döndürür.
 * Dosya bulunamazsa NULL döndürür.
 */
Image *imread(char *fname) {
    FILE *f = fopen(fname, "rb"); // Dosyayı aç
    Image *im;
    if (f == NULL) { // Dosya açılamazsa
        perror("dosya bulunamadı!"); // Hata mesajını yazdır
        return NULL;
    }

    // Başlık bilgilerini oku
    uint32_t offset, hsize, width, height;
    uint16_t nplanes, nbits;
    fseek(f, 10, SEEK_SET);
    fread(&offset, 4, 1, f); //2.Haftanın pdf'ten
    fread(&hsize, 4, 1, f);
    fread(&width, 4, 1, f);
    fread(&height, 4, 1, f);
    fread(&nplanes, 2, 1, f);
    fread(&nbits, 2, 1, f);

    // Esnek dizi üyesi dahil Image yapısı için bellek tahsis et
    im = (Image*)malloc(sizeof(Image)+ (width * height * sizeof(Pixel)));
    if (im == NULL) { // Bellek tahsis edilemezse
        fclose(f); // Dosyayı kapat
        perror("Bellek tahsisi başarısız!"); // Hata mesajını yazdır
        return NULL;
    }

    // Image yapısı üyelerine değerler atama
    im->offset = offset;
    im->headersize = hsize;
    im->width = width;
    im->height = height;
    im->nplanes = nplanes;
    im->nbits = nbits;

    // Piksel verilerini oku
    fseek(f, im->offset, SEEK_SET);
    for(int i = (im->height)-1; i >= 0; i--){//2.slayttan aldım
       for(int j = 0; j < im->width; j++){
            fread(&(im->pixels[i*im->width+j]),4,1,f);
       }
   }

    

    fclose(f); // Dosyayı kapat

    return im; // Image yapısını döndür
}

/** Görüntü belleğini serbest bırakır ve dosyayı kapatır */
int imclose(Image *im) {
    free(im); // Belleği serbest bırak
    return 0;
}

/** Karakter c'yi piksel p içine gizler, değiştirilmiş pikseli döndürür */
int hideinpixel(Pixel *p, unsigned char c) {
    p->b = (p->b & 0xFC) | ((c >> 6) & 0x03);
    p->g = (p->g & 0xFC) | ((c >> 4) & 0x03);
    p->r = (p->r & 0xFC) | ((c >> 2) & 0x03);
    p->a = (p->a & 0xFC) | (c & 0x03);
    return 0;
}

/**
 * Verilen mesajı im->pixels piksellerine hideinpixel() kullanarak gizler
 */
int hidemsg(Image *im, unsigned char *msg) {
    int msg_len = strlen((char *)msg);
    int index = 0;
    for (int i = 0; i < im->width * im->height && index < msg_len; i++) {
        Pixel current_pixel = im->pixels[i]; // Geçici bir kopya oluştur
        hideinpixel(&current_pixel, msg[index]);
        im->pixels[i] = current_pixel; // Kopyayı orijinal yapının üzerine yaz
        index++;
    }
    return 0;
}

/**
 * Pikseldeki gizli karakteri okur ve döndürür:
 * initial(left most) 2  bit in p.b for 00000011->initial bits 00
 * following 2 bit in p.g
 * following 2 bit in p.r
 * following 2 bit in p.a
 */
char readcharinpixel(Pixel *p) {
    unsigned char c = 0;
    c |= (p->b & 0x03) << 6;
    c |= (p->g & 0x03) << 4;
    c |= (p->r & 0x03) << 2;
    c |= (p->a & 0x03);
    return c;
}

/**
 * im içindeki gizli karakterleri msg[] içine okur ve döndürür,
 * readcharinpixel() kullanır, döndürülen karakter sayısını döndürür
 */
int readmsg(Image *im, unsigned char msg[], int maxlength) {
    int index = 0;
    for (int i = 0; i < im->width * im->height && index < maxlength; i++) {
        Pixel current_pixel = im->pixels[i]; // Geçici bir kopya oluştur
        msg[index] = readcharinpixel(&current_pixel);
        index++;
    }
    return index;
}

/**
 * Pikselleri yazdırır
 */
int printpixel(Pixel *p) {
    printf("r:%d-g:%d-b:%d-a:%d\n", p->r, p->g, p->b, p->a);
    return 0;
}

int main(void) {
    unsigned char *msg = (unsigned char *)"hello";
    char fname[] = "cute-animals32bit.bmp";
    Image *im = imread(fname);

    if (im == NULL) {
        printf("Hata: Görüntü dosyası açılamadı veya okunamadı.\n");
        return 1;
    }

    hidemsg(im, msg);
    printf("%.6s\n", (char *)msg);

    imclose(im);

    return 0;//
}