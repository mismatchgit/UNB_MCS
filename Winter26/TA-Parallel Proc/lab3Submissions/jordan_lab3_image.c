#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#define WIDTH 1920
#define HEIGHT 1080
#define CHANNELS 3 // Red, Green, Blue

typedef struct 
{
    int width;
    int height;
    unsigned char *data; // The 1D array of pixel data
} Image;

// Allocates memory for a new image
Image* create_image(int w, int h) 
{
    Image *img = (Image*)malloc(sizeof(Image));
    img->width = w;
    img->height = h;
    img->data = (unsigned char*)malloc(w * h * CHANNELS);
    return img;
}

// Generates a smooth gradient with random "salt and pepper" noise.
// We add noise so we can visually see the Blur filter working later.
void generate_noisy_image(Image *img) 
{
    for (int y = 0; y < img->height; y++) 
    {
        for (int x = 0; x < img->width; x++) 
        {
            int index = (y * img->width + x) * CHANNELS;
            // Background Gradient
            img->data[index] = (unsigned char)(x % 255); // Red
            img->data[index+1] = (unsigned char)(y % 255); // Green
            img->data[index+2] = 128; // Blue
            // Add Noise (Random white specks)
            if ((rand() % 100) < 5)  // 5% chance
            {
                img->data[index] = 255;
                img->data[index+1] = 255;
                img->data[index+2] = 255;
            }
        }
    }
}

// Save as BMP (Windows Bitmap) so you can view it without installing software.
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

int invert_parallel(Image *img)
{
    double start3 = omp_get_wtime();

    #pragma omp parallel for
    for (int i = 0; i < img->height * img->width * CHANNELS; i++) 
    {
        img->data[i] = 255 - img->data[i];
    }
    double end3 = omp_get_wtime();
    double timeStep3 = end3-start3;
    printf("Invert Parallel processing time taken: %f\n", timeStep3);
}

int blur_parallel(Image *source, Image *dest)
{
    double start4 = omp_get_wtime();

    //For every pixel
    omp_set_num_threads(1);
    #pragma omp parallel for
    for (int j = 1; j < ((source->height-1)); j ++)
    {
        for (int i = 1; i < ((source->width-1)); i ++)
        {

            int avgRed = 0;
            int avgGreen = 0;
            int avgBlue = 0;
            //For 3x3 Average Colors
            for (int k = -1; k <= 1; k++)
            {
                for (int l = -1; l <= 1; l++)
                {
                    avgRed += (source->data[(((j+l)*((source->width))+k+i) * CHANNELS)]);
                    avgGreen += (source->data[(((j+l)*((source->width))+k+i) * CHANNELS) + 1]);
                    avgBlue += (source->data[(((j+l)*((source->width))+k+i) * CHANNELS) + 2]);
                }
            }
            dest->data[((j*(source->width))+i) * CHANNELS]= ((avgRed)/9);
            dest->data[((j*(source->width))+i) * CHANNELS +1] = ((avgGreen)/9);
            dest->data[((j*(source->width))+i) * CHANNELS +2]= ((avgBlue)/9);
        }
    }


    double end4 = omp_get_wtime();
    double timeStep4 = end4-start4;
    printf("Single Blur processing time taken: %f\n", timeStep4);
}


int main() 
{
    Image *source = create_image(WIDTH, HEIGHT);
    generate_noisy_image(source);
    writeBMP("original.bmp", source);

    Image * source2 = create_image(WIDTH, HEIGHT);
    memcpy(source2->data, source->data, source->height * source->width * CHANNELS);    
    
    Image * destination2 = create_image(WIDTH, HEIGHT);
    memcpy(destination2->data, source->data, source->height * source->width * CHANNELS);    

    Image * source3 = create_image(WIDTH, HEIGHT);
    memcpy(source3->data, source->data, source->height * source->width * CHANNELS);


    // TODO: Call your filters here...
    invert_parallel(source);    
    writeBMP("inverted.bmp", source);

    blur_parallel(source2, destination2);
    writeBMP("blurred.bmp", destination2);

    //PART 5

    // 1. Reset source to the noisy original
    generate_noisy_image(source3);                                  // <----  I created a new copy of source, just to be safe.
    // 2. Create a second buffer for Double Buffering
    Image *buffer = create_image(WIDTH, HEIGHT);
    // 3. Run the "Ping-Pong" loop
    // We swap Source and Buffer every iteration so the output of one becomes input of next
    double start = omp_get_wtime();
    for (int i = 0; i < 20; i++) 
    {
        if (i % 2 == 0) 
        {
            blur_parallel(source, buffer); // Read Source -> Write Buffer
        } 
        else 
        {
            blur_parallel(buffer, source); // Read Buffer -> Write Source
        }
    }
    double end = omp_get_wtime();

    printf("Heavy Blur Time: %f seconds\n", end - start);
    // 4. Save result (It is in 'source' because loop ended on odd iteration 19 -> back to source)
    writeBMP("blurred_heavy.bmp", source);

    free(source->data);
    free(source);
    free(source2->data);
    free(source2);
    free(source3->data);
    free(source3);
    free(destination2->data);
    free(destination2);
    free(buffer->data);
    free(buffer);
    return 0;
}