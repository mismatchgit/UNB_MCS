#include <stdio.h>
#include <stdlib.h>

#define CHANNELS 3  //Red, Green, Blue

typedef struct {
    int width;
    int height;
    unsigned char *data;
} Image;

Image * imageUpscale()
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

    for(int y = height - 1; y >= 0; y--)
    {
        for(int x = 0; x < width; x++)
        {
            int i = (y * width + x) * CHANNELS;

            unsigned char b = fgetc(readFile);
            unsigned char g = fgetc(readFile);
            unsigned char r = fgetc(readFile);

            img->data[i] = r;
            img->data[i+1] = g;
            img->data[i+2] = b;
        }

        fseek(readFile, padSize, SEEK_CUR);
    }
    fclose(readFile);
    return foundImg;
}

//Taken directly from Lab3
Image* create_image(int w, int h) 
{
    Image *img = (Image*)malloc(sizeof(Image));
    img->width = w;
    img->height = h;
    img->data = (unsigned char*)malloc(w * h * CHANNELS);
    return img;
}

//Taken directly from Lab3
void writeBMP(const char *filename, Image *img) 
{
    FILE *f = fopen(filename, "wb");
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
    for (int y = h - 1; y >= 0; y--) {
    for (int x = 0; x < w; x++) {
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
    int height = atoi(argv[1]);
    int width = atoi(argv[2]);

    Image *input = loadPhoto("input.bnp", width, height);
    Image *output = imageUpscale(input, width, height);

    writeBMP("projectHelloWorld.bmp", source);

    free(input->data);
    free(output->data);
    free(input);
    free(output);

    return 0;
}