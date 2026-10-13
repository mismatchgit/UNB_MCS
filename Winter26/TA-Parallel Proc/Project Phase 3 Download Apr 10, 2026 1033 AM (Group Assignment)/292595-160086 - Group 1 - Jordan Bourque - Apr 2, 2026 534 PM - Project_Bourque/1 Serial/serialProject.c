#include <stdio.h>
#include <stdlib.h>

#define CHANNELS 3  //Red, Green, Blue
#define alpha 1.2

typedef struct {
    int width;
    int height;
    unsigned char *data;
} Image;

//Taken directly from Lab3
Image* create_image(int w, int h) 
{
    Image *img = (Image*)malloc(sizeof(Image));
    img->width = w;
    img->height = h;
    img->data = (unsigned char*)malloc(w * h * CHANNELS);
    return img;
}

Image * imageContrast(Image * input) //Mode 0
{
    Image * output = create_image(input -> width, input -> height);

    for(int y = 0; y < input -> height; y++)
    {
        for(int x = 0; x < input -> width; x++)
        {
            int n = (y * input -> width + x) * CHANNELS;

            for(int i = 0; i < CHANNELS; i++)
            {
                int pixel = input -> data[n + i];
                int newPixel = (int) ((alpha * (pixel - 128)) + 128);

                if (newPixel < 0) 
                {newPixel = 0;}
                if (newPixel > 255)
                {newPixel = 255;}

                output -> data[n + i] = (unsigned char) newPixel;
            }
        }
    }
    return output;
}

Image * imageBlur(Image * input) //Mode 1
{
    Image * output = create_image(input -> width, input -> height);

    int kernel[3][3] ={ {1, 2, 1},
                        {2, 4, 2},
                        {1, 2, 1} };

    int kernelSum = 16;

    for(int i = 0; i < input->width * input->height * CHANNELS; i++)
    {
        output->data[i] = input->data[i];
    }

    for(int y1 = 1; y1 < input -> height - 1; y1++)
    {
        for(int x1 = 1; x1 < input -> width - 1; x1++)
        {
            for(int c = 0; c < CHANNELS; c++)
            {
                int sum = 0;
                for(int y2 = -1; y2 <= 1; y2++)
                {
                    for(int x2 = -1; x2 <= 1; x2++)
                    {
                        int px = x1 + x2;
                        int py = y1 + y2;

                        int n = ((py * input -> width + px) * CHANNELS + c);

                        sum += input -> data[n] * kernel[y2 + 1][x2 + 1];
                    }
                }
                int outIndex = (y1 * input -> width + x1) * CHANNELS + c;
                output -> data[outIndex] = (unsigned char) (sum / kernelSum);
            }
        }
    }
    return output;
}

Image * imageUpscale(Image * input) //Mode 2
{
    int newWidth = input -> width * 2;
    int newHeight = input -> height * 2;

    Image * output = create_image(newWidth, newHeight);

    for(int y = 0; y < input -> height; y++)
    {
        for(int x = 0; x < input -> width; x++)
        {
            int n = (y * input -> width + x) * CHANNELS;

            int newX = x * 2;
            int newY = y * 2;

            int pix1 = (newY * newWidth + newX) * CHANNELS;
            int pix2 = (newY * newWidth + newX + 1) * CHANNELS;
            int pix3 = ((newY + 1) * newWidth + newX) * CHANNELS;
            int pix4 = ((newY + 1) * newWidth + newX + 1) * CHANNELS;

            for(int i = 0; i < CHANNELS; i++)
            {
                output->data[pix1 + i] = input->data[n + i];
                output->data[pix2 + i] = input->data[n + i];
                output->data[pix3 + i] = input->data[n + i];
                output->data[pix4 + i] = input->data[n + i];
            }
        }
    }
    return output;
}

Image* loadPhoto(const char *filename, int width, int height)
{
    FILE * readFile = fopen(filename, "rb");

    if(!readFile)
    {
        perror("File open failed");
        exit(1);
    }

    Image * foundImg = create_image(width, height);

    fseek(readFile, 54, SEEK_SET);

    int padSize = (4 - (width * 3) % 4) % 4;

    for(int y = height - 1; y >= 0; y--)
    {
        for(int x = 0; x < width; x++)
        {
            int i = (y * width + x) * CHANNELS;

            unsigned char b = fgetc(readFile);
            unsigned char g = fgetc(readFile);
            unsigned char r = fgetc(readFile);

            foundImg->data[i] = r;
            foundImg->data[i+1] = g;
            foundImg->data[i+2] = b;
        }

        fseek(readFile, padSize, SEEK_CUR);
    }
    fclose(readFile);
    return foundImg;
}

//Taken directly from Lab3
void writeBMP(const char *filename, Image *img) 
{
    FILE * f = fopen(filename, "wb");
    if (!f) { perror("File open failed"); return; }
    int w = img->width, h = img->height;

    // Standard BMP Header
    unsigned char fileHeader[14] = { 'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0 };
    unsigned char infoHeader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0 };

    int padSize = (4 - (w * 3) % 4) % 4;
    int size = 54 + (w * 3 + padSize) * h;

    fileHeader[2] = (unsigned char)(size); fileHeader[3] = (unsigned char)(size>>8);
    fileHeader[4] = (unsigned char)(size>>16); fileHeader[5] = (unsigned char)(size>>24);
    infoHeader[4] = (unsigned char)(w); infoHeader[5] = (unsigned char)(w>>8);
    infoHeader[6] = (unsigned char)(w>>16); infoHeader[7] = (unsigned char)(w>>24);
    infoHeader[8] = (unsigned char)(h); infoHeader[9] = (unsigned char)(h>>8);
    infoHeader[10] = (unsigned char)(h>>16); infoHeader[11] = (unsigned char)(h>>24);

    fwrite(fileHeader, 1, 14, f);
    fwrite(infoHeader, 1, 40, f);

    unsigned char padding[3] = {0,0,0};
    for (int y = h - 1; y >= 0; y--) 
    {
    for (int x = 0; x < w; x++) 
    {
    int i = (y * w + x) * 3;
    // BMP expects BGR format (Blue, Green, Red)
    fputc(img->data[i+2], f); fputc(img->data[i+1], f); fputc(img->data[i], f);
    }
    fwrite(padding, 1, padSize, f);
    }
    fclose(f);
    printf("Saved %s\n", filename);
}

int main (int argc, char * argv[])
{

    if (argc < 4)
    {
        printf("Please enter width, height, and mode. \nModes: \n0 = contrast adjustment,\n1 = gaussian blur\n2 = image upscale.");
        return 1;
    }

    int height = atoi(argv[1]);
    int width = atoi(argv[2]);
    int mode = atoi(argv[3]);

    Image *input = loadPhoto("input.bmp", width, height);
    Image *output;

    switch(mode)
    {
        case 0:
            output = imageContrast(input);
            break;

        case 1:
            output = imageBlur(input);
            break;

        case 2:
            output = imageUpscale(input);
            break;

        default:
            printf("Mode = 0, 1, or 2.");
            return 1;
    }

    writeBMP("projectHelloWorld.bmp", output);

    free(input->data);
    free(output->data);
    free(input);
    free(output);

    return 0;
}