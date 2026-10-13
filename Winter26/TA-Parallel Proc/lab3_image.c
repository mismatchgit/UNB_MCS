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

void invert_parallel(Image *img) {
    int total_pixels = img->width * img->height;
    
    // Parallelize over all pixels using a flat 1D loop (best for cache & load balance)
    #pragma omp parallel for
    for (int i = 0; i < total_pixels; i++) {
        int idx = i * CHANNELS;  // 0,3,6,... for RGB start of each pixel
        
        img->data[idx + 0] = 255 - img->data[idx + 0];  // Red   → 255 - R
        img->data[idx + 1] = 255 - img->data[idx + 1];  // Green → 255 - G
        img->data[idx + 2] = 255 - img->data[idx + 2];  // Blue  → 255 - B
    }
    
    printf("Image inverted (parallel) – white noise should now be black dots.\n");
}

void blur_parallel(Image *source, Image *dest) {
    // Make sure dest has the same size
    if (dest->width != source->width || dest->height != source->height) {
        printf("Error: Source and destination image dimensions must match!\n");
        return;
    }

    int width = source->width;
    int height = source->height;

    // We only blur interior pixels (skip 1-pixel border on all sides)
    #pragma omp parallel for collapse(2) schedule(static)
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int center_idx = (y * width + x) * CHANNELS;

            // We'll accumulate sum for each channel
            int sum_r = 0, sum_g = 0, sum_b = 0;

            // 3×3 neighborhood
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int ny = y + dy;
                    int nx = x + dx;
                    int idx = (ny * width + nx) * CHANNELS;

                    sum_r += source->data[idx + 0];
                    sum_g += source->data[idx + 1];
                    sum_b += source->data[idx + 2];
                }
            }

            // Average = sum / 9, store in destination
            dest->data[center_idx + 0] = (unsigned char)(sum_r / 9);
            dest->data[center_idx + 1] = (unsigned char)(sum_g / 9);
            dest->data[center_idx + 2] = (unsigned char)(sum_b / 9);
        }
    }

    // Optional: copy the border pixels unchanged from source to dest
    // (so the whole image is valid, just unblurred on edges)
    #pragma omp parallel for
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Only copy border pixels (the ones we skipped above)
            if (y == 0 || y == height-1 || x == 0 || x == width-1) {
                int idx = (y * width + x) * CHANNELS;
                dest->data[idx + 0] = source->data[idx + 0];
                dest->data[idx + 1] = source->data[idx + 1];
                dest->data[idx + 2] = source->data[idx + 2];
            }
        }
    }

    printf("Box blur (3x3) applied in parallel – interior pixels smoothed, borders copied.\n");
}
int main() {
    Image *source = create_image(WIDTH, HEIGHT);
    // generate_noisy_image(source);
    // writeBMP("original.bmp", source);
    // // TODO: Call your filters here...
    // invert_parallel(source);
    // writeBMP("inverted_parallel.bmp", source);
    // free(source->data);
    // free(source);
    // return 0;
    // 1. Reset source to the noisy original
    generate_noisy_image(source);
    // 2. Create a second buffer for Double Buffering
    Image *buffer = create_image(WIDTH, HEIGHT);
    // 3. Run the "Ping-Pong" loop
    // We swap Source and Buffer every iteration so the output of one becomes input of next
    double start = omp_get_wtime();
    for (int i = 0; i < 20; i++) {
        if (i % 2 == 0) {
            blur_parallel(source, buffer); // Read Source -> Write Buffer
        } else {
            blur_parallel(buffer, source); // Read Buffer -> Write Source
        }
    }
    double end = omp_get_wtime();
    printf("Heavy Blur Time: %f seconds\n", end - start);
    // 4. Save result (It is in 'source' because loop ended on odd iteration 19 -> back to source)
    writeBMP("blurred_heavy.bmp", source);
    free(buffer->data); free(buffer);
}