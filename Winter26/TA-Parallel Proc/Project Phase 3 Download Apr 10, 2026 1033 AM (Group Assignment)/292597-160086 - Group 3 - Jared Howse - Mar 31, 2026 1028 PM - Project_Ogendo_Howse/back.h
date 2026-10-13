#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./src/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./src/stb_image_write.h"

#include "./src/serial.h"
#include "./src/openmp.h"
#include "./src/ompi.h"

void serial_grayscale(unsigned char *const img, unsigned char *const gray_img, const int height, const int width);
void serial_gaussian_blur(unsigned char *const in, unsigned char *out, int w, int h);
void serial_edge_detect(unsigned char *const in, unsigned char *out, int w, int h);

void omp_grayscale(unsigned char *const img, unsigned char *const gray_img, const int height, const int width);
void omp_gaussian_blur(unsigned char *const in, unsigned char *out, int w, int h);
void omp_edge_detect(unsigned char *const in, unsigned char *out, const int width, const int height);

void mpi_grayscale(unsigned char *const img, unsigned char *const gray_img, const int height, const int width, const int size, const int rank);
void mpi_gaussian_blur(unsigned char *in, unsigned char *out, const int height, const int width, const int size, const int rank);
void mpi_edge_detect(unsigned char *in, unsigned char *out, const int height, const int width, const int size, const int rank);

struct Image
{
    int width;
    int height;
    int channels;
};

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

    unsigned char *img = NULL;
    struct Image metadata;
    size_t img_size;
    unsigned char *gray_img = NULL;
    unsigned char *blur_img = NULL;
    unsigned char *edge_img = NULL;

    // Isolating memory allocation to only the host process
    if (rank == 0)
    {
        img = stbi_load("./resource/parrot.png", &metadata.width, &metadata.height, &metadata.channels, 4);

        if (img == NULL)
        {
            printf("Error in loading the image\n");
            MPI_Finalize();
            return 1;
        }

        img_size = (size_t)metadata.width * metadata.height;
        gray_img = (unsigned char *)calloc(img_size, sizeof(unsigned char));

        if (gray_img == NULL)
        {
            stbi_image_free(img);
            MPI_Finalize();
            return 1;
        }

        blur_img = (unsigned char *)calloc(img_size, sizeof(unsigned char));
        if (blur_img == NULL)
        {
            stbi_image_free(img);
            free(gray_img);
            MPI_Finalize();
            return 1;
        }

        edge_img = (unsigned char *)calloc(img_size, sizeof(unsigned char));
        if (edge_img == NULL)
        {
            stbi_image_free(img);
            free(gray_img);
            free(blur_img);
            MPI_Finalize();
            return 1;
        }
    }

    if (mode == 2)
    {
        int dims[3];
        if (rank == 0)
        {
            dims[0] = metadata.width;
            dims[1] = metadata.height;
            dims[2] = metadata.channels;
        }

        MPI_Bcast(dims, 3, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank != 0)
        {
            metadata.width = dims[0];
            metadata.height = dims[1];
            metadata.channels = dims[2];
        }
    }

    double start_time, gray_time, blur_time, edge_time;
    double glocal_elapsed, blocal_elapsed, elocal_elapsed, gmax_elapsed, bmax_elapsed, emax_elapsed;

    switch (mode)
    {
    case 0:
        // 1. Sync all MPI processes before starting
        MPI_Barrier(MPI_COMM_WORLD);
        start_time = MPI_Wtime();
        serial_grayscale(img, gray_img, metadata.height, metadata.width);
        MPI_Barrier(MPI_COMM_WORLD);
        gray_time = MPI_Wtime();
        glocal_elapsed = gray_time - start_time;
        stbi_write_png("./resource/gray_serial.png", metadata.width, metadata.height, 1, gray_img, metadata.width);

        MPI_Barrier(MPI_COMM_WORLD);
        start_time = MPI_Wtime();
        serial_gaussian_blur(gray_img, blur_img, metadata.width, metadata.height);
        MPI_Barrier(MPI_COMM_WORLD);
        blur_time = MPI_Wtime();
        blocal_elapsed = blur_time - start_time;
        stbi_write_png("./resource/blur_serial.png", metadata.width, metadata.height, 1, blur_img, metadata.width);

        MPI_Barrier(MPI_COMM_WORLD);
        start_time = MPI_Wtime();
        serial_edge_detect(blur_img, edge_img, metadata.width, metadata.height);
        MPI_Barrier(MPI_COMM_WORLD);
        edge_time = MPI_Wtime();
        elocal_elapsed = edge_time - start_time;
        stbi_write_png("./resource/edge_serial.png", metadata.width, metadata.height, 1, edge_img, metadata.width);

        if (rank == 0)
            printf("\n|============ SERIAL ============|\n");
        break;

    case 1:
        MPI_Barrier(MPI_COMM_WORLD);
        start_time = MPI_Wtime();
        omp_grayscale(img, gray_img, metadata.height, metadata.width);
        MPI_Barrier(MPI_COMM_WORLD);
        gray_time = MPI_Wtime();
        glocal_elapsed = gray_time - start_time;

        if (rank == 0)
            stbi_write_png("./resource/gray_omp.png", metadata.width, metadata.height, 1, gray_img, metadata.width);

        MPI_Barrier(MPI_COMM_WORLD);
        start_time = MPI_Wtime();
        omp_gaussian_blur(gray_img, blur_img, metadata.width, metadata.height);
        MPI_Barrier(MPI_COMM_WORLD);
        blur_time = MPI_Wtime();
        blocal_elapsed = blur_time - start_time;

        if (rank == 0)
            stbi_write_png("./resource/blur_omp.png", metadata.width, metadata.height, 1, blur_img, metadata.width);

        MPI_Barrier(MPI_COMM_WORLD);
        start_time = MPI_Wtime();
        omp_edge_detect(blur_img, edge_img, metadata.width, metadata.height);
        MPI_Barrier(MPI_COMM_WORLD);
        edge_time = MPI_Wtime();
        elocal_elapsed = edge_time - start_time;

        if (rank == 0)
            stbi_write_png("./resource/edge_omp.png", metadata.width, metadata.height, 1, edge_img, metadata.width);

        if (rank == 0)
            printf("\n|============ OPEN-MP ============|\n");
        break;

    case 2:

        MPI_Barrier(MPI_COMM_WORLD);
        start_time = MPI_Wtime();

        mpi_grayscale(img, gray_img, metadata.height, metadata.width, size, rank);

        MPI_Barrier(MPI_COMM_WORLD);
        gray_time = MPI_Wtime();
        glocal_elapsed = gray_time - start_time;
        if (rank == 0)
            stbi_write_png("./resource/gray_mpi.png", metadata.width, metadata.height, 1, gray_img, metadata.width);

        MPI_Barrier(MPI_COMM_WORLD);
        start_time = MPI_Wtime();

        mpi_gaussian_blur(gray_img, blur_img, metadata.height, metadata.width, size, rank);

        MPI_Barrier(MPI_COMM_WORLD);
        blur_time = MPI_Wtime();
        blocal_elapsed = blur_time - start_time;
        if (rank == 0)
            stbi_write_png("./resource/blur_mpi.png", metadata.width, metadata.height, 1, blur_img, metadata.width);

        MPI_Barrier(MPI_COMM_WORLD);
        start_time = MPI_Wtime();

        mpi_edge_detect(blur_img, edge_img, metadata.height, metadata.width, size, rank);

        MPI_Barrier(MPI_COMM_WORLD);
        edge_time = MPI_Wtime();
        elocal_elapsed = edge_time - start_time;
        if (rank == 0)
            stbi_write_png("./resource/edge_mpi.png", metadata.width, metadata.height, 1, edge_img, metadata.width);

        if (rank == 0)
        {
            printf("\n|============ MPI ============|\n");
            fflush(stdout);
        }
        break;

    default:
        break;
    }

    // Find the 'bottleneck' time (the slowest process)
    MPI_Reduce(&glocal_elapsed, &gmax_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&blocal_elapsed, &bmax_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&elocal_elapsed, &emax_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        printf("Grayscale runtime: %f seconds\n", gmax_elapsed);
        printf("Gaussian Blur runtime runtime: %f seconds\n", bmax_elapsed);
        printf("Edge Detection runtime: %f seconds\n", emax_elapsed);
    }

    stbi_image_free(img);
    free(gray_img);
    free(blur_img);
    free(edge_img);
    MPI_Finalize();
    return 0;
}