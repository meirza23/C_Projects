/*DO NOT CHANGE THIS FILE
BU DOSYADA DEGISIKLIK YAPMAYINIZ,
DEGISIKLIK YAPANLARA DIREK 0 VERILECEKTIR
*/
#define main studentmain
#include "main.c"
#undef main

#undef NDEBUG
#include <assert.h>

char *fname = "cute-animals32bit.bmp";

int testimread() {
    Image *im = imread(fname);
    assert(im != NULL);
    assert(im->height == 488);
    assert(im->width == 800);
    assert(im->nplanes == 1);
    assert(im->nbits == 32);
    imclose(im);
    printf("success");
    return 0;
}
int testprintpixel(Pixel p) {
    printf("r:%d-g:%d-b:%d-a:%d ", p.r, p.g, p.b, p.a);
    return 0;
}
Pixel testpixels(int i, int j) {
    Image *im = imread(fname);
    Pixel p;

    p = im->pixels[i * im->width + j];
    testprintpixel(p);

    return p;
}

int testhideinpixel1() {
    Pixel p;
    p.pixel = 0x00000000;
    hideinpixel(&p, (char)1);
    assert(p.b == 0);
    assert(p.g == 0);
    assert(p.r == 0);
    assert(p.a == 1);
    printf("success");
    return 0;
}
int testhideinpixel2() {
    Pixel p;
    p.pixel = 0x02010301;

    hideinpixel(&p, (char)1);
    assert(p.b == 0);
    assert(p.g == 0);
    assert(p.r == 0);
    assert(p.a == 1);  // 0000010->00000001
    printf("success");
    return 0;
}
int testhideinpixel3() {
    Pixel p;
    p.pixel = 0x00000000;
    hideinpixel(&p, (char)0b10101010);
    assert(p.b == 2);
    assert(p.g == 2);
    assert(p.r == 2);
    assert(p.a == 2);
    printf("success");
    return 0;
}

int testreadcharinpixel() {
    Pixel p;
    p.pixel = 0x00000000;
    char c = readcharinpixel(&p);
    assert(c == (char)0);
    // printf("%c", c);
    printf("success");
    return c;
}

int testreadcharinpixel2() {
    Pixel p;
    p.pixel = 0x02020202;
    char c = readcharinpixel(&p);
    assert(c == (char)0b10101010);
    // printf("%c", c);
    printf("success");
    return c;
}

int testreadcharinpixel3() {
    Pixel p;
    p.pixel = 0x40536271;
    char c = readcharinpixel(&p);
    assert(c == (char)0b01101100);
    // printf("%c", c);
    printf("success");
    return c;
}

int testhidemsg() {
    char *fname = "cute-animals32bit.bmp";
    Image *im = imread(fname);

    unsigned char c[101]={'\0'};
    for (int i = 0; i < 100; i++) {
        c[i] = (rand() % 100)+1;
    }
    hidemsg(im, c);
    unsigned char mask = (unsigned char)0x03;

    for (int i = 0; i < 100; i++) {
        unsigned char first = im->pixels[i].b & mask;
        unsigned char second = im->pixels[i].g & mask;
        unsigned char third = im->pixels[i].r & mask;
        unsigned char fourth = im->pixels[i].a & mask;
        unsigned char r = first << 6 | second << 4 | third << 2 | fourth;
        assert(r == c[i]);
    }
    printf("success");
    return 0;
}

int testreadmsg() {
    char *fname = "cute-animals32bit.bmp";
    Image *im = imread(fname);

    unsigned char c[101] = {'\0'};
    for (int i = 0; i < 100; i++) {
        c[i] = (rand() % 100)+1;
    }
    hidemsg(im, c);

    unsigned char c2[100];
    readmsg(im, c2, 100);
    for (int i = 0; i < 100; i++) {
        assert(c[i] == c2[i]);
    }
    printf("success");
    return 0;
}

int main() {
    int test = 0;
    scanf("%d", &test);
    switch (test) {
        case 0:
            testimread();
            break;
        case 1: {
            int i = 0, j = 0;
            scanf("%d,%d", &i, &j);
            testpixels(i, j);

            break;
        }
        case 2:
            testhideinpixel1();
            break;
        case 3:
            testhideinpixel2();
            break;
        case 4:
            testhideinpixel3();
            break;
        case 5:
            testreadcharinpixel();
            break;
        case 6:
            testreadcharinpixel2();
            break;
        case 7:
            testreadcharinpixel3();
            break;
        case 8:
            testhidemsg();
            break;
        case 9:
            testreadmsg();
            break;
        default:
            break;
    }
}