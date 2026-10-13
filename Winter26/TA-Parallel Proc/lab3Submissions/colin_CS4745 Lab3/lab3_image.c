#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define WIDTH 1920
#define HEIGHT 1080
#define CHANNELS 3 // Red, Green, Blue

typedef struct {
    int width;
    int height;
    unsigned char *data; // The 1D array of pixel data
} Image;

// Allocates memory for a new image
Image* create_image(int w, int h) {
Image *img = (Image*)malloc(sizeof(Image));
    img->width = w;
    img->height = h;
    img->data = (unsigned char*)malloc(w * h * CHANNELS);
    return img;
}

// Generates a smooth gradient with random "salt and pepper" noise.
// We add noise so we can visually see the Blur filter working later.
void generate_noisy_image(Image *img) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int index = (y * img->width + x) * CHANNELS;

            // Background Gradient
            img->data[index] = (unsigned char)(x % 255); // Red
            img->data[index+1] = (unsigned char)(y % 255); // Green
            img->data[index+2] = 128; // Blue

            // Add Noise (Random white specks)
            if ((rand() % 100) < 5) { // 5% chance
                img->data[index] = 255;
                img->data[index+1] = 255;
                img->data[index+2] = 255;
            }
        }
    }
}

void invert_parallel(Image *img){
    #pragma omp parallel
    {
        int total = omp_get_num_threads();
        int id = omp_get_thread_num();
        double chunk = img->height/total;
        int remainder = img->height%total;
        int start_row = id*chunk;
        int end_row = start_row+chunk;
        if(id==total-1){end_row = end_row+remainder;}
        for(int y = start_row; y < end_row; y++){
            for(int x = 0; x < img->width; x++){
                int index = (y * img->width + x) * CHANNELS;

                img->data[index] = 255-img->data[index]; // Red
                img->data[index+1] = 255-img->data[index+1]; // Green
                img->data[index+2] = 255-img->data[index+2]; // Blue
            }
        }
    }
}

// Used to make blur_parallel less messy
int averagePixel(Image *img, int index){
    int average = img->data[index-(img->width-3)*CHANNELS]+img->data[index-(img->width)*CHANNELS]+img->data[index-(img->width+3)*CHANNELS]; //height-1
    average = average+img->data[index-3]+img->data[index]+img->data[index+3]; //height
    average = average+img->data[index+(img->width-3)*CHANNELS]+img->data[index+(img->width)*CHANNELS]+img->data[index+(img->width+3)*CHANNELS]; //height+1
    return average/9;
}

void blur_parallel(Image *img, Image *buf){
    #pragma omp parallel
    {
        int total = omp_get_num_threads();
        int id = omp_get_thread_num();
        double chunk = img->height/total;
        int remainder = img->height%total;
        int start_row = id*chunk;
        int end_row = start_row+chunk;
        if(id==0){start_row = start_row+1;}
        if(id==total-1){end_row = end_row+remainder-1;}
        for(int y = start_row; y < end_row; y++){
            for(int x = 0; x < img->width; x++){
                int index = (y * img->width + x) * CHANNELS;
                
                buf->data[index] = averagePixel(img, index); // Red
                buf->data[index+1] = averagePixel(img, index+1); // Green
                buf->data[index+2] = averagePixel(img, index+2); // Blue
            }
        }
    }
}

// Save as BMP (Windows Bitmap) so you can view it without installing software.
void writeBMP(const char *filename, Image *img) {
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

int main(int argc, char **argv) {
    if (argc >= 2) {
        int t = atoi(argv[1]);
        if (t > 0) omp_set_num_threads(t);
    }

    Image *source = create_image(WIDTH, HEIGHT);
    generate_noisy_image(source);
    writeBMP("original.bmp", source);
    
    // TODO: Call your filters here...
    invert_parallel(source);
    writeBMP("inverted.bmp", source);

    generate_noisy_image(source);

    Image *buffer = create_image(WIDTH, HEIGHT);

    double start = omp_get_wtime();
    for (int i = 0; i < 20; i++) {
        if (i % 2 == 0) {
            blur_parallel(source, buffer); // Read Source -> Write Buffer
        } 
        else {
            blur_parallel(buffer, source); // Read Buffer -> Write Source
        }
    }

    double end = omp_get_wtime();
    printf("Heavy Blur Time: %f seconds\n", end - start);

    writeBMP("blurred_heavy.bmp", source);

    free(buffer->data); 
    free(buffer);

    free(source->data);
    free(source);
    return 0;
}