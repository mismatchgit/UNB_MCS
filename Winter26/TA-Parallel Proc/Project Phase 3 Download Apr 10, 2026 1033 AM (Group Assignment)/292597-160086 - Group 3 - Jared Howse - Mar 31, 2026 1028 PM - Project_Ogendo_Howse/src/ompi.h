#ifndef OMPI_H
#define OMPI_H

void mpi_grayscale(unsigned char *const img, unsigned char *const gray_img, const int height, const int width, const int size, const int rank)
{
    int chunks = width * height;
    int base_part = chunks / size;
    int remainder = chunks % size;

    int *counts = (int *)malloc(size * sizeof(int));
    int *displ = (int *)malloc(size * sizeof(int));
    int *g_counts = (int *)malloc(size * sizeof(int));
    int *g_displ = (int *)malloc(size * sizeof(int));

    int offset = 0;
    int g_offset = 0;
    for (int i = 0; i < size; i++)
    {
        // Ranks get 1 extra pixel
        int current_chunk_size = base_part + (i < remainder ? 1 : 0);
        counts[i] = current_chunk_size * 4; // 4 channels rgba
        displ[i] = offset;
        offset += counts[i];

        // Use g_counts and g_displ in MPI_Gatherv
        g_counts[i] = current_chunk_size;
        g_displ[i] = g_offset;
        g_offset += g_counts[i];
    }

    // Each rank allocates only what it needs based on its specific count
    unsigned char *img_buf = (unsigned char *)malloc(counts[rank]);
    unsigned char *gray_buf = (unsigned char *)malloc(g_counts[rank]);

    MPI_Scatterv(img, counts, displ, MPI_UNSIGNED_CHAR, img_buf, counts[rank], MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    for (int i = 0; i < g_counts[rank]; i++)
    { // Local index for 4-channel input
        int in_idx = i * 4;
        gray_buf[i] = (unsigned char)(0.299 * img_buf[in_idx] + 0.587 * img_buf[in_idx + 1] + 0.114 * img_buf[in_idx + 2]);
    }

    MPI_Gatherv(gray_buf, g_counts[rank], MPI_UNSIGNED_CHAR, gray_img, g_counts, g_displ, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    free(counts);
    free(displ);
    free(g_counts);
    free(g_displ);
    free(img_buf);
    free(gray_buf);
}

void mpi_gaussian_blur(unsigned char *in, unsigned char *out, const int h, const int w, const int size, const int rank)
{
    int base_rows = h / size;
    int remainder = h % size;

    int *send_counts = (int *)malloc(size * sizeof(int));
    int *send_displs = (int *)malloc(size * sizeof(int));
    int *recv_counts = (int *)malloc(size * sizeof(int));
    int *recv_displs = (int *)malloc(size * sizeof(int));

    int current_row = 0;
    for (int i = 0; i < size; i++)
    {
        int rows_this_rank = base_rows + (i < remainder ? 1 : 0);
        recv_counts[i] = rows_this_rank * w;
        recv_displs[i] = current_row * w;

        // Halo Logic: Subtract 2 for 'above', add 2 for 'below'
        int start_row = (i == 0) ? 0 : current_row - 2;
        int end_row = (i == size - 1) ? current_row + rows_this_rank : current_row + rows_this_rank + 2;

        send_counts[i] = (end_row - start_row) * w;
        send_displs[i] = start_row * w;
        current_row += rows_this_rank;
    }

    unsigned char *local_in = (unsigned char *)malloc(send_counts[rank]);
    unsigned char *local_out = (unsigned char *)malloc(recv_counts[rank]);

    MPI_Scatterv(in, send_counts, send_displs, MPI_UNSIGNED_CHAR,
                 local_in, send_counts[rank], MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    int kernel[5][5] = {{1, 4, 7, 4, 1},
                        {4, 16, 26, 16, 4},
                        {7, 26, 41, 26, 7},
                        {4, 16, 26, 16, 4},
                        {1, 4, 7, 4, 1}};

    int kernel_sum = 273;
    int rows_to_process = recv_counts[rank] / w;

    for (int y = 0; y < rows_to_process; y++)
    {
        for (int x = 0; x < w; x++)
        {
            // Index of local buffer (skipping 2 halo rows if not rank 0)
            int buffer_y = y + ((rank == 0) ? 0 : 2);
            int global_y = (recv_displs[rank] / w) + y;

            // Boundary safety: 2-pixel margin for 5x5 kernel
            if (global_y < 2 || global_y >= h - 2 || x < 2 || x >= w - 2)
            {
                local_out[y * w + x] = local_in[buffer_y * w + x]; // Keep original or set to 0
                continue;
            }

            int sum = 0;
            for (int ky = -2; ky <= 2; ky++)
            {
                for (int kx = -2; kx <= 2; kx++)
                {
                    sum += local_in[(buffer_y + ky) * w + (x + kx)] * kernel[ky + 2][kx + 2];
                }
            }
            local_out[y * w + x] = (unsigned char)(sum / kernel_sum);
        }
    }

    MPI_Gatherv(local_out, recv_counts[rank], MPI_UNSIGNED_CHAR, out, recv_counts, recv_displs, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // Cleanup
    free(send_counts);
    free(send_displs);
    free(recv_counts);
    free(recv_displs);
    free(local_in);
    free(local_out);
}

void mpi_edge_detect(unsigned char *in, unsigned char *out, const int height, const int width, const int size, const int rank)
{
    int base_rows = height / size;
    int remainder = height % size;

    int *send_counts = (int *)malloc(size * sizeof(int));
    int *send_displs = (int *)malloc(size * sizeof(int));
    int *recv_counts = (int *)malloc(size * sizeof(int));
    int *recv_displs = (int *)malloc(size * sizeof(int));

    int current_row = 0;
    for (int i = 0; i < size; i++)
    {
        int rows_for_this_rank = base_rows + (i < remainder ? 1 : 0);

        // Gatherv Metadata (The actual result size)
        recv_counts[i] = rows_for_this_rank * width;
        recv_displs[i] = current_row * width;

        // Scatterv Metadata (Includes the Halo/Overlap)
        int start_row = (i == 0) ? 0 : current_row - 1;
        int end_row = (i == size - 1) ? current_row + rows_for_this_rank : current_row + rows_for_this_rank + 1;
        int num_rows_with_halo = end_row - start_row;

        send_counts[i] = num_rows_with_halo * width;
        send_displs[i] = start_row * width;

        current_row += rows_for_this_rank;
    }

    // Allocate local buffer to hold the chunk + halo rows
    unsigned char *local_in = (unsigned char *)malloc(send_counts[rank]);
    unsigned char *local_out = (unsigned char *)malloc(recv_counts[rank]);

    MPI_Scatterv(in, send_counts, send_displs, MPI_UNSIGNED_CHAR,
                 local_in, send_counts[rank], MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // Perform Laplacian on the local chunk
    // If not Rank 0, 'actual' data starts at row 1 of local_in
    int rows_to_process = recv_counts[rank] / width;

    // 3x3 Laplacian Kernel
    int kernel[3][3] = {
        {0, 1, 0},
        {1, -4, 1},
        {0, 1, 0}};

    for (int y = 0; y < rows_to_process; y++)
    {
        for (int x = 0; x < width; x++)
        {

            // Map local 'y' to the buffer index including the halo offset
            int buffer_y = y + ((rank == 0) ? 0 : 1);

            // Global Boundary Check: Skip the very top and very bottom of the WHOLE image
            int global_y = (recv_displs[rank] / width) + y;
            if (global_y == 0 || global_y == height - 1 || x == 0 || x == width - 1)
            {
                local_out[y * width + x] = 0;
                continue;
            }

            int sum = 0;
            for (int ky = -1; ky <= 1; ky++)
            {
                for (int kx = -1; kx <= 1; kx++)
                {
                    // Access neighbors using the buffer_y which includes our halo row
                    unsigned char pixel = local_in[(buffer_y + ky) * width + (x + kx)];
                    sum += pixel * kernel[ky + 1][kx + 1];
                }
            }

            int edge = abs(sum);
            local_out[y * width + x] = (unsigned char)(edge > 255 ? 255 : edge);
        }
    }

    MPI_Gatherv(local_out, recv_counts[rank], MPI_UNSIGNED_CHAR,
                out, recv_counts, recv_displs, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    free(send_counts);
    free(send_displs);
    free(recv_counts);
    free(recv_displs);
    free(local_in);
    free(local_out);
}

#endif // OMPI_H