#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#define WIDTH 1920
#define HEIGHT 1080
#define CHANNELS 3 // Red, Green, Blue
typedef struct {
  int width;
  int height;
  unsigned char *data; // The 1D array of pixel data
} Image;

// Allocates memory for a new image
Image *create_image(int w, int h) {
  Image *img = (Image *)malloc(sizeof(Image));
  img->width = w;
  img->height = h;
  img->data = (unsigned char *)malloc(w * h * CHANNELS);
  return img;
}

// Generates a smooth gradient with random "salt and pepper" noise.
// We add noise so we can visually see the Blur filter working later.
void generate_noisy_image(Image *img) {
  for (int y = 0; y < img->height; y++) {
    for (int x = 0; x < img->width; x++) {
      int index = (y * img->width + x) * CHANNELS;
      // Background Gradient
      img->data[index] = (unsigned char)(x % 255);     // Red
      img->data[index + 1] = (unsigned char)(y % 255); // Green
      img->data[index + 2] = 128;                      // Blue
      // Add Noise (Random white specks)
      if ((rand() % 100) < 5) { // 5% chance
        img->data[index] = 255;
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
  int w = img->width, h = img->height;
  // Standard BMP Header
  unsigned char fileHeader[14] = {'B', 'M', 0, 0,  0, 0, 0,
                                  0,   0,   0, 54, 0, 0, 0};
  unsigned char infoHeader[40] = {40, 0, 0, 0, 0, 0, 0,  0,
                                  0,  0, 0, 0, 1, 0, 24, 0};
  int padSize = (4 - (w * 3) % 4) % 4;
  int size = 54 + (w * 3 + padSize) * h;
  fileHeader[2] = (unsigned char)(size);
  fileHeader[3] = (unsigned char)(size >> 8);
  fileHeader[4] = (unsigned char)(size >> 16);
  fileHeader[5] = (unsigned char)(size >> 24);
  infoHeader[4] = (unsigned char)(w);
  infoHeader[5] = (unsigned char)(w >> 8);
  infoHeader[6] = (unsigned char)(w >> 16);
  infoHeader[7] = (unsigned char)(w >> 24);
  infoHeader[8] = (unsigned char)(h);
  infoHeader[9] = (unsigned char)(h >> 8);
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

int inline img_index(int x, int y) { return (x * WIDTH + y) * CHANNELS; }

void invert(Image *img) {
  double dawn = omp_get_wtime();
  for (int i = 0; i < (WIDTH * HEIGHT * 3); ++i) {
    img->data[i] = 255 - img->data[i];
  }
  double dusk = omp_get_wtime();
  printf("Invertion: time elapsed: %04fs\n", dusk - dawn);
}

void invert_parallel(Image *img) {
  double dawn = omp_get_wtime();
#pragma omp parallel for
  for (int i = 0; i < (WIDTH * HEIGHT * 3); ++i) {
    img->data[i] = 255 - img->data[i];
  }
  double dusk = omp_get_wtime();
  printf("Invertion: time elapsed: %04fs\n", dusk - dawn);
}

void blur(Image *img, Image *buf) {
  for (int i = 1; i < HEIGHT - 1; ++i) {
    for (int j = 1; j < WIDTH - 1; ++j) {
      for (int r = 0; r < CHANNELS; ++r) {
        buf->data[img_index(i, j) + r] =
            (img->data[img_index(i, j) + r] +
             img->data[img_index(i + 1, j) + r] +
             img->data[img_index(i, j + 1) + r] +
             img->data[img_index(i + 1, j + 1) + r] +
             img->data[img_index(i - 1, j) + r] +
             img->data[img_index(i, j - 1) + r] +
             img->data[img_index(i - 1, j - 1) + r] +
             img->data[img_index(i + 1, j - 1) + r] +
             img->data[img_index(i - 1, j + 1) + r]) /
            9;
      }
    }
  }
}

void blur_parallel(Image *img, Image *buf) {
#pragma omp parallel for default(none) shared(buf, img)
  for (int i = 1; i < HEIGHT - 1; ++i) {
    for (int j = 1; j < WIDTH - 1; ++j) {
      for (int r = 0; r < CHANNELS; ++r) {
        buf->data[img_index(i, j) + r] =
            (img->data[img_index(i, j) + r] +
             img->data[img_index(i + 1, j) + r] +
             img->data[img_index(i, j + 1) + r] +
             img->data[img_index(i + 1, j + 1) + r] +
             img->data[img_index(i - 1, j) + r] +
             img->data[img_index(i, j - 1) + r] +
             img->data[img_index(i - 1, j - 1) + r] +
             img->data[img_index(i + 1, j - 1) + r] +
             img->data[img_index(i - 1, j + 1) + r]) /
            9;
      }
    }
  }
}

int main() {
  Image *source = create_image(WIDTH, HEIGHT);
  generate_noisy_image(source);
  writeBMP("3/original.bmp", source);

  // 3
  invert(source);

  // 4
  generate_noisy_image(source);
  Image *buf = create_image(WIDTH, HEIGHT);

  double dawn = omp_get_wtime();
  for (int i = 0; i < 20; ++i) {
    if (i % 2 == 0) {
      blur(source, buf);
    } else {
      blur(buf, source);
    }
  }
  double dusk = omp_get_wtime();
  printf("Blur: time elapsed: %04fs\n", dusk - dawn);

  printf("----------Parallel----------\n");
  generate_noisy_image(source);

  invert_parallel(source);
  writeBMP("3/inverted.bmp", source);

  // 4
  generate_noisy_image(source);
  generate_noisy_image(buf);

  dawn = omp_get_wtime();
  for (int i = 0; i < 20; ++i) {
    if (i % 2 == 0) {
      blur_parallel(source, buf);
    } else {
      blur_parallel(buf, source);
    }
  }
  dusk = omp_get_wtime();
  printf("Blur: time elapsed: %04fs\n", dusk - dawn);

  writeBMP("3/blured.bmp", source);

  free(buf->data);
  free(buf);
  free(source->data);
  free(source);
  return 0;
}
