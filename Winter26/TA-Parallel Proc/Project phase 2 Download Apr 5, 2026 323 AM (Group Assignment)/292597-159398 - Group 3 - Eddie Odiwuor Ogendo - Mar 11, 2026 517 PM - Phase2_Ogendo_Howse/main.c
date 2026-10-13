#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <mpi.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./src/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./src/stb_image_write.h"

#include "./src/serial.h"

void serial_grayscale(unsigned char *const img, unsigned char *const gray_img, const int height, const int width);
void serial_gaussian_blur(unsigned char *const in, unsigned char *out, int w, int h);
void serial_edge_detect(unsigned char *const in, unsigned char *out, int w, int h);

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 3)
    {
        if (rank == 0)
            printf("Usage: mpirun -np {processors} ./main {mode} {threads}\n");
        MPI_Finalize();
        return 1;
    }

    const int mode = atoi(argv[1]);
    const int threads = atoi(argv[2]);

    omp_set_num_threads(threads);

    int width, height, channels;
    unsigned char *img = stbi_load("./resource/parrot.png", &width, &height, &channels, 4);

    if (img == NULL)
    {
        printf("Error in loading the image\n");
        MPI_Finalize();
        return 1;
    }

    size_t img_size = (size_t)width * height * 4;

    size_t gray_size = (size_t)width * height;
    unsigned char *gray_img = (unsigned char *)calloc(gray_size, sizeof(unsigned char));

    if (gray_img == NULL)
    {
        stbi_image_free(img);
        MPI_Finalize();
        return 1;
    }

    size_t blur_size = gray_size;
    unsigned char *blur_img = (unsigned char *)calloc(blur_size, sizeof(unsigned char));
    if (blur_img == NULL)
    {
        stbi_image_free(img);
        free(gray_img);
        MPI_Finalize();
        return 1;
    }

    size_t edge_size = blur_size;
    unsigned char *edge_img = calloc(edge_size, sizeof(unsigned char));

    if (edge_img == NULL)
    {
        stbi_image_free(img);
        free(gray_img);
        free(blur_img);
        MPI_Finalize();
        return 1;
    }

    switch (mode)
    {
    case 0:
        serial_grayscale(img, gray_img, height, width);
        stbi_write_png("./resource/gray_serial.png", width, height, 1, gray_img, width);

        serial_gaussian_blur(gray_img, blur_img, width, height);
        stbi_write_png("./resource/blur_serial.png", width, height, 1, blur_img, width);

        serial_edge_detect(blur_img, edge_img, width, height);
        stbi_write_png("./resource/edge_serial.png", width, height, 1, edge_img, width);
        break;

    default:
        break;
    }

    stbi_image_free(img);
    free(gray_img);
    free(blur_img);
    free(edge_img);
    MPI_Finalize();
    return 0;
}