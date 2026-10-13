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

//invert_parallel function

void invert_parallel(Image *image){
    //calculate the number of bytes to loop into
    int total_bytes = image->width * image->height * CHANNELS;

    #pragma omp parallel for 
    for (int i=0 ; i<total_bytes; i++){
        image->data[i]=250-image->data[i];
    }
}

void blur_parallel (Image *src, Image *new){

    int width = src->width;
    int height = src->height;
    
    #pragma omp parallel for

    for (int y=1; y<height -1; y++){
        for ( int x=1; x < width-1; x++ ){
            
            int index = (y * width + x) * CHANNELS;
            //fpr each channel 
            for (int c=0; c<CHANNELS; c++ ){
                int sum =0;
                //Handle the 3*3  getting the neighbours
                for (int dy=-1; dy<=1; dy++){
                    for (int dx=-1;dx<=1; dx++){
                        //sum of the pixels 
                        int n = ((x + dx) + (y + dy) * width ) * CHANNELS + c;
                        sum += src->data[n];
                    }
                }

                new->data[index+c]=sum/9;
            }
        }

    }

}



int main() {
 Image *source = create_image(WIDTH, HEIGHT);
 generate_noisy_image(source);
 writeBMP("original.bmp", source);

 // TODO: Call your filters here...
 //invert the original.bmp
 invert_parallel(source);
 //generate it
 writeBMP("inverted.bmp", source);

 //reset to Noisy iamge 
 generate_noisy_image(source);

 //creat a buffer for double buffering 
 omp_set_num_threads(8);
 Image *buffer = create_image(WIDTH,HEIGHT);

// We swap Source and Buffer every iteration so the output of one becomes input of next
int nthr = omp_get_max_threads();

printf("The total number of threads are %d \n", nthr);
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

  // 4. Save result (It is in 'source' because loop ended on odd iteration 19 -> back to source)
 writeBMP("blurred_heavy.bmp", source);


 free(buffer->data); free(buffer);
 free(source->data);
 free(source);
 return 0;
}

//export OMP_NUM_THREADS=8
//reset OMP_NUM_THREADS