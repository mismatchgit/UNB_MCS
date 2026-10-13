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
            img->data[index]     = (unsigned char)(x % 255); // Red
            img->data[index + 1] = (unsigned char)(y % 255); // Green
            img->data[index + 2] = 128;                      // Blue

            // Add Noise (Random white specks)
            if ((rand() % 100) < 5) { // 5% chance
                img->data[index]     = 255;
                img->data[index + 1] = 255;
                img->data[index + 2] = 255;
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

    int w = img->width;
    int h = img->height;

    // Standard BMP Header
    unsigned char fileHeader[14] = {
        'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0
    };
    unsigned char infoHeader[40] = {
        40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0
    };

    int padSize = (4 - (w * 3) % 4) % 4;
    int size = 54 + (w * 3 + padSize) * h;

    fileHeader[2] = (unsigned char)(size);
    fileHeader[3] = (unsigned char)(size >> 8);
    fileHeader[4] = (unsigned char)(size >> 16);
    fileHeader[5] = (unsigned char)(size >> 24);

    infoHeader[4]  = (unsigned char)(w);
    infoHeader[5]  = (unsigned char)(w >> 8);
    infoHeader[6]  = (unsigned char)(w >> 16);
    infoHeader[7]  = (unsigned char)(w >> 24);
    infoHeader[8]  = (unsigned char)(h);
    infoHeader[9]  = (unsigned char)(h >> 8);
    infoHeader[10] = (unsigned char)(h >> 16);
    infoHeader[11] = (unsigned char)(h >> 24);

    fwrite(fileHeader, 1, 14, f);
    fwrite(infoHeader, 1, 40, f);

    unsigned char padding[3] = {0, 0, 0};

    for (int y = h - 1; y >= 0; y--) {
        for (int x = 0; x < w; x++) {
            int i = (y * w + x) * 3;

            // BMP expects BGR format (Blue, Green, Red)
            fputc(img->data[i + 2], f);
            fputc(img->data[i + 1], f);
            fputc(img->data[i], f);
        }
        fwrite(padding, 1, padSize, f);
    }

    fclose(f);
    printf("Saved %s\n", filename);
}

void blur_parallel(Image *buffer, Image *img) {
    int stride = WIDTH * CHANNELS;

    #pragma omp parallel for num_threads(8)
    for (int y = 1; y < HEIGHT - 1; y++) {
        for (int x = 1; x < WIDTH - 1; x++) {
            int r = 0, g = 0, b = 0;

            // Central index for the current pixel
            int current = y * stride + x * CHANNELS;

            // Iterate through the 3x3 neighborhood
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int neighbor_idx = current + (ky * stride) + (kx * CHANNELS);
                    r += img->data[neighbor_idx];
                    g += img->data[neighbor_idx + 1];
                    b += img->data[neighbor_idx + 2];
                }
            }

            buffer->data[current] = r / 9;
            buffer->data[current + 1] = g / 9;
            buffer->data[current + 2] = b / 9;
        }
    }
}






void invert_parallel(Image *img){
    // we can just use this regular opm parallel for because we do not need to do anything different for different chunks
    #pragma omp parallel for
    for (int index = 0; index < WIDTH*HEIGHT * CHANNELS; index++) {
        // Background Gradient
        img->data[index] = 255 - img->data[index]; // Re
        
    }
}

int main() {
    Image *source = create_image(WIDTH, HEIGHT);

    generate_noisy_image(source);
    writeBMP("original.bmp", source);
    invert_parallel(source);
    writeBMP("inverted.bmp", source);
    // TODO: Call your filters here...

    // 1. Reset source to the noisy original
    generate_noisy_image(source);

    // 2. Create a second buffer for Double Buffering
    Image *buffer = create_image(WIDTH, HEIGHT);

    // 3. Run the "Ping-Pong" loop
    // We swap Source and Buffer every iteration so the output of one becomes input of next
    double start = omp_get_wtime();

    for (int i = 0; i < 20; i++) {
        if (i % 2 == 0) {
            blur_parallel(buffer, source);
              // Read Source -> Write Buffer
        } else {
            blur_parallel(source, buffer);  // Read Buffer -> Write Source
        }
    }

    blur_parallel(source, buffer);

    double end = omp_get_wtime();
    printf("Heavy Blur Time: %f seconds\n", end - start);

    // 4. Save result
    // (It is in 'source' because loop ended on odd iteration 19 -> back to source)
    writeBMP("blurred_heavy.bmp", source);

    free(buffer->data);
    free(buffer);


    free(source->data);
    free(source);

    return 0;
}
