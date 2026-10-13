#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define WIDTH 1920
#define HEIGHT 1080
#define CHANNELS 3 // R, G, B

// Data Structure: A flattened 1D array representing a 2D image
typedef struct {
    int width;
    int height;
    unsigned char *data; // 1D array: R, G, B, R, G, B...
} Image;

// Helper: Allocate memory for an image
Image* create_image(int w, int h) {
    Image *img = (Image*)malloc(sizeof(Image));
    img->width = w;
    img->height = h;
    img->data = (unsigned char*)malloc(w * h * CHANNELS);
    return img;
}

// Helper: Generate a "test pattern" (Gradient) so we don't need external files
// Helper: Generate a gradient WITH RANDOM NOISE
void generate_test_image(Image *img) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int index = (y * img->width + x) * CHANNELS;
            
            // 1. Create the smooth gradient background
            img->data[index] = (unsigned char)(x % 255);
            img->data[index+1] = (unsigned char)(y % 255);
            img->data[index+2] = 128;

            // 2. Add "Salt and Pepper" Noise (Random white/black spots)
            // This makes the image look "dirty" so the blur has something to clean up.
            int noise = rand() % 100;
            if (noise < 5) { // 5% chance of noise
                // Make pixel white
                img->data[index] = 255; 
                img->data[index+1] = 255; 
                img->data[index+2] = 255;
            }
        }
    }
}

// Helper: Save image as standard BMP (Windows Bitmap)
// This allows you to open the file on any lab computer without installing viewers.
void writeBMP(const char *filename, Image *img) {
    FILE *f = fopen(filename, "wb");
    if (!f) { perror("File open failed"); return; }
    int w = img->width, h = img->height;
    unsigned char fileHeader[14] = { 'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0 };
    unsigned char infoHeader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0 };
    int padSize  = (4 - (w * 3) % 4) % 4;
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
    double start = omp_get_wtime();
    
    // TODO: Calculate total bytes in the image (width * height * 3)
    long total_bytes = img->width * img->height * CHANNELS;

    // TODO: Add OpenMP Pragma here
    #pragma omp parallel for
    for (long i = 0; i < total_bytes; i++) {
        img->data[i] = 255 - img->data[i];
    }
    
    double end = omp_get_wtime();
    printf("Invert Time: %f seconds\n", end - start);
}

void blur_parallel(Image *src, Image *dst) {
    double start = omp_get_wtime();
    int w = src->width;
    int h = src->height;

    // TODO: Parallelize the outer loops. 
    // HINT: Use 'collapse(2)' to combine Y and X loops into one large task pool.
    #pragma omp parallel for collapse(2)
    for (int y = 1; y < h - 1; y++) {       // Skip top/bottom borders
        for (int x = 1; x < w - 1; x++) {   // Skip left/right borders
            
            // Loop over 3 channels (Red, Green, Blue)
            for (int c = 0; c < 3; c++) {
                int sum = 0;
                
                // 3x3 Stencil: Visit neighbors (dy=-1 to 1, dx=-1 to 1)
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        
                        // MAPPING FORMULA: (Row * Width + Col) * 3 + Channel
                        int index = ((y + dy) * w + (x + dx)) * 3 + c;
                        
                        sum += src->data[index];
                    }
                }
                
                // Write Average to DESTINATION
                int out_index = (y * w + x) * 3 + c;
                dst->data[out_index] = sum / 9;
            }
        }
    }
    
    double end = omp_get_wtime();
    printf("Blur Time: %f seconds\n", end - start);
}

int main() {
    // 1. Setup
    Image *source = create_image(WIDTH, HEIGHT);
    generate_test_image(source);
    writeBMP("original.bmp", source); // Save baseline to check validity
    
    // 2. We will implement filters here...
    invert_parallel(source);
    writeBMP("inverted.bmp", source);

    // Reset source (OPTIONAL: re-generate if you want to blur the original, not the inverted)
    generate_test_image(source);

    Image *dest = create_image(WIDTH, HEIGHT);
    
    blur_parallel(source, dest);
    writeBMP("blurred.bmp", dest);
    
    free(dest->data); free(dest);
    
    // Cleanup
    free(source->data);
    free(source);
    return 0;
}