#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./src/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./src/stb_image_write.h"

#include "./src/serial.h"
#include "./src/openmp.h"
#include "./src/ompi.h"

struct Image
{
    int width, height, channels;
};

int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 4)
    {
        if (rank == 0)
            printf("Usage: %s <0:Serial|1:OMP|2:MPI> <threads> <path>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int mode = atoi(argv[1]);
    int threads = atoi(argv[2]);
    char *path = argv[3];

    struct Image meta;
    unsigned char *img = NULL;

    if (rank == 0)
    {
        img = stbi_load(path, &meta.width, &meta.height, &meta.channels, 4);
        if (!img)
        {
            printf("Error: Could not load image %s\n", path);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    int dims[2];
    if (rank == 0)
    {
        dims[0] = meta.width;
        dims[1] = meta.height;
    }
    MPI_Bcast(dims, 2, MPI_INT, 0, MPI_COMM_WORLD);
    meta.width = dims[0];
    meta.height = dims[1];

    unsigned char *gray = (rank == 0) ? malloc(meta.width * meta.height) : NULL;
    unsigned char *blur = (rank == 0) ? malloc(meta.width * meta.height) : NULL;
    unsigned char *edge = (rank == 0) ? malloc(meta.width * meta.height) : NULL;

    double start, end;
    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();

    if (mode == 0 && rank == 0)
    {
        serial_grayscale(img, gray, meta.height, meta.width);
        serial_gaussian_blur(gray, blur, meta.width, meta.height);
        serial_edge_detect(blur, edge, meta.width, meta.height);
    }
    else if (mode == 1 && rank == 0)
    {
        omp_set_num_threads(threads);
        omp_grayscale(img, gray, meta.height, meta.width);
        omp_gaussian_blur(gray, blur, meta.width, meta.height);
        omp_edge_detect(blur, edge, meta.width, meta.height);
    }
    else if (mode == 2)
    {
        mpi_grayscale(img, gray, meta.height, meta.width, size, rank);
        mpi_gaussian_blur(gray, blur, meta.height, meta.width, size, rank);
        mpi_edge_detect(blur, edge, meta.height, meta.width, size, rank);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();

    if (rank == 0)
    {
        // Output format: MODE_TIME: [seconds]
        printf("TIME: %f\n", end - start);

        // Save results to verify correctness (commented out for testing)
        stbi_write_png("./resource/output_edge.png", meta.width, meta.height, 1, edge, meta.width);

        stbi_image_free(img);
        free(gray);
        free(blur);
        free(edge);
    }

    MPI_Finalize();
    return 0;
}
