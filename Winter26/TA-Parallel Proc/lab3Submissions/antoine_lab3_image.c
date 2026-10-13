#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

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

// Save as BMP (Windows Bitmap) so you can view it without installing software.
void writeBMP(const char *filename, Image *img) {

    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("File open failed");
        return;
    }

    int w = img->width, h = img->height;

    // Standard BMP Header
    unsigned char fileHeader[14] = { 'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0 };
    unsigned char infoHeader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0 };

    int padSize = (4 - (w * 3) % 4) % 4;
    int size = 54 + (w * 3 + padSize) * h;

    fileHeader[2] = (unsigned char)(size);
    fileHeader[3] = (unsigned char)(size>>8);
    fileHeader[4] = (unsigned char)(size>>16);
    fileHeader[5] = (unsigned char)(size>>24);
    infoHeader[4] = (unsigned char)(w);
    infoHeader[5] = (unsigned char)(w>>8);
    infoHeader[6] = (unsigned char)(w>>16);
    infoHeader[7] = (unsigned char)(w>>24);
    infoHeader[8] = (unsigned char)(h);
    infoHeader[9] = (unsigned char)(h>>8);
    infoHeader[10] = (unsigned char)(h>>16);
    infoHeader[11] = (unsigned char)(h>>24);

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

Image *copy_image(const Image *source);

void invert_parallel(Image *souce);
void blur_parallel(Image *out, Image *source);

int main(int argc, char **argv) {
    // Optional: run like ./matrix_lab 6
    if (argc >= 2) {
        int t = atoi(argv[1]);
        if (t > 0) omp_set_num_threads(t);
    }

    printf("thread num=%d\n", omp_get_max_threads());

    Image *source = create_image(WIDTH, HEIGHT);

    generate_noisy_image(source);
    writeBMP("original.bmp", source);

    // TODO: Call your filters here...
    Image *invert_image = copy_image(source);
    invert_parallel(invert_image);

    writeBMP("inverted.bmp", invert_image);

    Image *buffer= copy_image(source);
    double start = omp_get_wtime();
    for (int i=0; i<20; i++){
        if (i%2 == 0){
            blur_parallel(source, buffer);
        }
        else {
            blur_parallel(buffer, source);
        }
    }
    double end = omp_get_wtime();

    printf("Heavy Blur time: %f seconds\n", end-start);
    writeBMP("blurred_heavy.bmp", source);

    free(source->data);
    free(source);

    free(invert_image->data);
    free(invert_image);

    free(buffer->data);
    free(buffer);
    return 0;
}

Image *copy_image(const Image *source){
    Image *new = malloc(sizeof(Image));
    new->height = source->height;
    new->width = source->width;
    new->data = malloc(source->width * source->height * CHANNELS);

    if (new->data == NULL){
        fprintf(stderr, "Error: failed to allocate memory for the new image\n");
        exit(EXIT_FAILURE);
    }

    memcpy(new->data, source->data, source->width * source->height * CHANNELS);

    return new;
}

void invert_parallel(Image *source){
    int max = source->width * source->height;

    #pragma omp parallel for default(none) shared(source, max)
    for (int i=0; i<max; i++){
        int index = i*CHANNELS;
        source->data[index] = 255 - source->data[index];
        source->data[index+1] = 255 - source->data[index+1];
        source->data[index+2] = 255 - source->data[index+2];
    }
}

#define INDEX(x, y) ((y)*(WIDTH) + (x))*CHANNELS

void blur_parallel(Image *dest, Image *source){
    const int width = source->width;
    const int height = source->height;
    unsigned char *in = source->data;
    unsigned char *out = dest->data;

    #pragma omp parallel for default(none) shared(out, in, width, height)
    for (int y=0; y<height; y++){
        for (int x=0; x<width; x++){
            if (y == 0 || y == height-1 || x == 0 || x == width-1){
                out[INDEX(x, y)] = in[INDEX(x, y)];
            }
            else {
                int points[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

                points[0] = INDEX(x-1, y-1);
                points[1] = INDEX(x, y-1);
                points[2] = INDEX(x+1, y-1);
                points[3] = INDEX(x-1, y);
                points[4] = INDEX(x, y);
                points[5] = INDEX(x+1, y);
                points[6] = INDEX(x-1, y+1);
                points[7] = INDEX(x, y+1);
                points[8] = INDEX(x+1, y+1);

                int r = 0, g = 0, b = 0;
                for (int i=0; i<9; i++){
                    r += in[points[i]];
                    g += in[points[i]+1];
                    b += in[points[i]+2];
                }

                r /= 9;
                g /= 9;
                b /= 9;

                out[INDEX(x, y)] = r;
                out[INDEX(x, y)+1] = g;
                out[INDEX(x, y)+2] = b;
            }
        }
    }
}

