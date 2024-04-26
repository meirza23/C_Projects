/**
 * DO NOT CHANGE TYPES, VARIABLES DECLARATIONS
 *  AND FUNCTIONS PROTOTYPES
 */

#include <inttypes.h>
/** type for a 32 bit pixel, little endian*/
typedef union {
    uint32_t pixel;
    struct {
        uint8_t b : 8;      
        uint8_t g : 8;      
        uint8_t r : 8;
        uint8_t a : 8;      /* a or x in argb, xrgb   */
    } __attribute__((packed));
} Pixel;

/** struct type for image data*/
typedef struct image {
    int offset;     /* starting address in image file.              */
    int headersize; /* header size                                  */
    int width;      /* image width in pixels                        */
    int height;     /* image height in pixels                       */
    int nplanes;    /* number of color planes-1                     */
    int nbits;      /* 32-24-16 etc                                 */
    Pixel pixels[];   /* pointer to image, remaining part of bmp data */
} __attribute__((packed)) Image;
