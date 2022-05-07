
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include "timing.h"
#include <stdint.h>
#include <assert.h>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
/* ref:https://github.com/nothings/stb/blob/master/stb_image.h */

#include "libbmp.h"

#ifndef _MAX_DRIVE
#define _MAX_DRIVE 3
#endif
#ifndef _MAX_FNAME
#define _MAX_FNAME 256
#endif
#ifndef _MAX_EXT
#define _MAX_EXT 256
#endif
#ifndef _MAX_DIR
#define _MAX_DIR 256
#endif
#ifndef MIN
#define MIN(a, b)    ( (a) > (b) ? (b) : (a) )
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#define int8    char
#define uint8   unsigned char
#define int16   short
#define uint16  unsigned short
#define int32   int
#define uint32  unsigned int
#define int64   long long
#define uint64  unsigned long long
#define CLIP3(x,min,max)         ( (x)< (min) ? (min) : ((x)>(max)?(max):(x)) )
char *out_img = "out_img.bmp";

uint8 inv_gamma_table[256] ={
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,
        2,2,3,3,3,3,3,4,4,4,4,5,5,5,5,6,
        6,6,7,7,7,8,8,8,9,9,9,10,10,10,11,11,
        12,12,12,13,13,14,14,15,15,16,16,17,17,18,18,19,
        19,20,20,21,22,22,23,23,24,25,25,26,26,27,28,28,
        29,30,30,31,32,33,33,34,35,36,36,37,38,39,39,40,
        41,42,43,44,44,45,46,47,48,49,50,51,51,52,53,54,
        55,56,57,58,59,60,61,62,63,64,65,66,67,68,70,71,
        72,73,74,75,76,77,78,80,81,82,83,84,86,87,88,89,
        91,92,93,94,96,97,98,100,101,102,104,105,106,108,109,110,
        112,113,115,116,117,119,120,122,123,125,126,128,129,131,132,134,
        135,137,139,140,142,143,145,147,148,150,152,153,155,157,158,160,
        162,163,165,167,169,170,172,174,176,177,179,181,183,185,187,188,
        190,192,194,196,198,200,202,204,206,208,210,212,214,216,218,220,
        222,224,226,228,230,232,234,236,238,240,242,245,247,249,251,253
};
unsigned char *loadImage(const char *filename, int *Width, int *Height, int *Channels) {
    return (stbi_load(filename, Width, Height, Channels, 0));
}
void save2BMP(uint8* input,uint16 img_width,uint16 img_height,char *file_name){

    bmp_img img_rgb;
    bmp_img_init_df (&img_rgb, img_width, img_height);
    for(uint16 y = 0;y < img_height;y++){
        for(uint16 x =0;x < img_width;x++){
            uint8 temp_r = input[3*(y*img_width+x)];
            uint8 temp_g = input[3*(y*img_width+x)+1];
            uint8 temp_b = input[3*(y*img_width+x)+2];
            bmp_pixel_init(&img_rgb.img_pixels[y][x], temp_r, temp_g, temp_b);
        }
    }
    bmp_img_write (&img_rgb, file_name);
    bmp_img_free (&img_rgb);
    return;
}

void apply_invert_gamma(uint8* input,uint8* output,uint16 img_width,uint16 img_height){

    for(uint16 y = 0;y < img_height;y++){
        for(uint16 x =0;x < img_width;x++){
            uint8 temp_r = input[3*(y*img_width+x)];
            uint8 temp_g = input[3*(y*img_width+x)+1];
            uint8 temp_b = input[3*(y*img_width+x)+2];
            output[3*(y*img_width+x)] = inv_gamma_table[temp_r];
            output[3*(y*img_width+x)+1] = inv_gamma_table[temp_g];
            output[3*(y*img_width+x)+2] = inv_gamma_table[temp_b];
        }
    }


}

int main(int argc, char **argv) {

    /*
    if (argc < 2) {
        printf("usage: %s   image \n ", argv[0]);
        printf("eg: %s   d:\\image.jpg \n ", argv[0]);

        return (0);
    }
    char *szfile = argv[1];
     */
    char *szfile = "test_img.jpg";
    int Width = 0;
    int Height = 0;
    int Channels = 0;
    unsigned char *inputImage = NULL;

    double startTime = now();
    inputImage = loadImage(szfile, &Width, &Height, &Channels);
    double nLoadTime = calcElapsed(startTime, now());
    printf("load time: %d ms.\n ", (int) (nLoadTime * 1000));

    if ((Channels != 0) && (Width != 0) && (Height != 0)) {
        uint8 *outputImg = (uint8 *) malloc(Width * Channels * Height * sizeof(uint8));
        if (inputImage) {
            memcpy(outputImg, inputImage, Width * Channels * Height*sizeof(uint8));
        } else {
            printf("load: %s fail!\n ", szfile);
        }
    //    img_process(outputImg,Width,Height);
    //    save2BMP(outputImg,Width,Height,out_img);
        uint8 *linear_img = (uint8 *) malloc(Width * Channels * Height * sizeof(uint8));

        apply_invert_gamma(outputImg,linear_img, Width,Height);
        save2BMP(linear_img,Width,Height,out_img);

        free(linear_img);
        free(outputImg);
        free(inputImage);
    }else
    {
        printf("load: %s fail!\n", szfile);
    }

    return 0;
}