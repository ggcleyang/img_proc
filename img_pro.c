
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
char *out_img = "gtm_gamma_img.bmp";

#define ASYMMETRY_LUT_SIZE 65
#define ISP_DRC_DATA_MAX    ((1<<8) - 1)
typedef struct hiISP_DRC_S
{
    uint8   u8Asymmetry;
    uint8   u8BrightEnhance;
    //uint32  au32AsymmetryLUT[ASYMMETRY_LUT_SIZE];
    uint8  au8AsymmetryLUT[ASYMMETRY_LUT_SIZE];

} ISP_DRC_S;

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
uint8 gamma_045_table[256] = {
        0,21,28,34,39,43,47,50,53,56,59,62,64,66,69,71,
        73,75,77,79,81,83,84,86,88,89,91,93,94,96,97,99,
        100,101,103,104,105,107,108,109,111,112,113,114,115,117,118,119,
        120,121,122,123,124,126,127,128,129,130,131,132,133,134,135,136,
        137,138,139,140,140,141,142,143,144,145,146,147,148,149,149,150,
        151,152,153,154,155,155,156,157,158,159,159,160,161,162,163,163,
        164,165,166,166,167,168,169,169,170,171,172,172,173,174,175,175,
        176,177,177,178,179,179,180,181,182,182,183,184,184,185,186,186,
        187,188,188,189,190,190,191,191,192,193,193,194,195,195,196,196,
        197,198,198,199,200,200,201,201,202,203,203,204,204,205,206,206,
        207,207,208,208,209,210,210,211,211,212,212,213,214,214,215,215,
        216,216,217,217,218,219,219,220,220,221,221,222,222,223,223,224,
        224,225,225,226,227,227,228,228,229,229,230,230,231,231,232,232,
        233,233,234,234,235,235,236,236,237,237,238,238,239,239,240,240,
        241,241,242,242,242,243,243,244,244,245,245,246,246,247,247,248,
        248,249,249,250,250,250,251,251,252,252,253,253,254,254,255,255
};
void GenerateAsymmetry(ISP_DRC_S *pstDrc)
{
    uint32 Asymmetry = pstDrc->u8Asymmetry;
    uint32 SecondPole = pstDrc->u8BrightEnhance;

    double x  = ((double)(Asymmetry)+1)/257 * 2 - 1;
    int ai = (int)(0.5 + 255 * (1- 1/(1000*x*x*x) + x - ((x >= 0)*2)));
    double as = fabs((double)(ai) / 255);
    double dp = (double)(SecondPole) / 255;
    //printf("Asymmetry_value:%f,Asymmetry_gamma:%f\n",as,dp);
    int ii;
    for (ii = 0 ; ii <ASYMMETRY_LUT_SIZE ; ++ii)
    {
        if(ai >= 0)
        {
            x = (double)(ii) / (ASYMMETRY_LUT_SIZE - 1);
        }
        else
        {
            x = (double)((ASYMMETRY_LUT_SIZE - 1) - ii) / (ASYMMETRY_LUT_SIZE - 1);
        }
        int   y   =   (int)((dp+(1-dp)*pow((fabs(1-dp-x)/dp),   3))   * (x*(as+1)/(as+x) ) * ISP_DRC_DATA_MAX + 0.5);
//        if(ii==33){
//            printf("dp:%f\n",dp);
//            printf("x:%f\n",x);
//            printf("y:%d\n",y);
//        }
        y = y < 0 ? 0 : y > ISP_DRC_DATA_MAX ? ISP_DRC_DATA_MAX : y;
        if (ai < 0)
        {
            y = ISP_DRC_DATA_MAX - y;
        }
        pstDrc->au8AsymmetryLUT[ii] = y;
    }
}

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
void single2BMP(uint8* input,uint16 img_width,uint16 img_height,char *file_name){

    bmp_img img_rgb;
    bmp_img_init_df (&img_rgb, img_width, img_height);
    for(uint16 y = 0;y < img_height;y++){
        for(uint16 x =0;x < img_width;x++){
            uint8 temp_value = input[y*img_width+x];
            bmp_pixel_init(&img_rgb.img_pixels[y][x], temp_value, temp_value, temp_value);
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
    return;
}
void rgb2gray(uint8* input,uint8* output,uint16 img_width,uint16 img_height){

     //rgb to intensity,get intensity img
     //intensity= 1/61*(R*20+G*40+B)
    for(uint16 y = 0;y < img_height;y++){
        for(uint16 x =0;x < img_width;x++){
            uint8 temp_r = input[3*(y*img_width+x)];
            uint8 temp_g = input[3*(y*img_width+x)+1];
            uint8 temp_b = input[3*(y*img_width+x)+2];
            output[y*img_width+x] = CLIP3(floor(1.0/61 *(temp_r*20+temp_g*40+temp_b)),0,255);
        }
    }

    return;
}
uint8 linear_inter(uint8 x0,uint8 y0,uint8 x1,uint8 y1,float x){

     if(x0==x1){
         return y0;
     }
     float k = (float)(y1-y0)/(x1-x0);
    //printf("linear_inter K:%f\n",k);
    uint8 result = CLIP3(y0 + k*(x-x0),0,255);
     return result;
}
void simple_gtm(uint8* input,uint8* output,uint16 img_width,uint16 img_height,uint8 *lut){

    uint8 *intensity_in = (uint8 *) malloc(img_width * img_height * sizeof(uint8));
    uint8 *intensity_out = (uint8 *) malloc(img_width * img_height * sizeof(uint8));

    //rgb to intensity,get intensity img
    //char *gray_in = "gray_in.bmp";
    //char *gray_out = "gray_out.bmp";
    rgb2gray(input,intensity_in,img_width,img_height);
    //single2BMP(intensity_in,img_width,img_height,gray_in);
    for(uint16 y = 0;y < img_height;y++){
        for(uint16 x =0;x < img_width;x++){
            uint8 temp_Y = intensity_in[y*img_width+x];
            uint8 index_Y = temp_Y/4;
            float float_Y = (float)temp_Y/4;
            intensity_out[y*img_width+x] = linear_inter(index_Y,lut[index_Y],index_Y+1,lut[index_Y+1],float_Y);
            //intensity_out[y*img_width+x] = gamma_045_table[temp_Y];
        }
    }
    //single2BMP(intensity_out,img_width,img_height,gray_out);
    // rgb * scale factor
    for(uint16 y = 0;y < img_height;y++){
        for(uint16 x =0;x < img_width;x++){
            float scale;
            if(intensity_in[y*img_width+x] == 0){
                scale = 1.0;
            }
            else
            {
                scale =  (float)intensity_out[y*img_width+x]/intensity_in[y*img_width+x];
            }

            //printf("scale:%f\n",scale);
            uint16 temp_R = input[3*(y*img_width+x)] * scale;
            uint16 temp_G = input[3*(y*img_width+x)+1] * scale;
            uint16 temp_B = input[3*(y*img_width+x)+2] * scale;

            output[3*(y*img_width+x)] =CLIP3(temp_R,0,255);
            output[3*(y*img_width+x)+1] =CLIP3(temp_G,0,255);
            output[3*(y*img_width+x)+2] =CLIP3(temp_B,0,255);
        }
    }


    free(intensity_in);
    free(intensity_out);

    return;
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
    char *szfile = "linear_img.bmp";
    int Width = 0;
    int Height = 0;
    int Channels = 0;
    unsigned char *inputImage = NULL;


    ISP_DRC_S* ispDrcS;
    ispDrcS = (ISP_DRC_S*) malloc(sizeof(ISP_DRC_S));
    ispDrcS->u8Asymmetry = 30;//[1 30] //smaller,dark area enhance stronger
    ispDrcS->u8BrightEnhance = 210;//[150 210] //250 //bigger ,global enhance stronger
    for(uint8 i =0;i<ASYMMETRY_LUT_SIZE;i++){
        ispDrcS->au8AsymmetryLUT[i] = 0;
    }

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
        uint8 *gtm_img = (uint8 *) malloc(Width * Channels * Height * sizeof(uint8));

    //    apply_invert_gamma(outputImg,linear_img, Width,Height);
        GenerateAsymmetry(ispDrcS);
        simple_gtm(outputImg,gtm_img,Width,Height,ispDrcS->au8AsymmetryLUT);
        save2BMP(gtm_img,Width,Height,out_img);

        free(gtm_img);
        free(outputImg);
        free(inputImage);
    }else
    {
        printf("load: %s fail!\n", szfile);
    }
    free(ispDrcS);
    return 0;
}