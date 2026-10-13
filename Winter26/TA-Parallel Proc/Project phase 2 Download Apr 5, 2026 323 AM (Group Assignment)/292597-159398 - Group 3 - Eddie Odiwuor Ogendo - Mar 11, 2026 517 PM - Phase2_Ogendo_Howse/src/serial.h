#ifndef SERIAL_H
#define SERIAL_H

void serial_grayscale(unsigned char *const img, unsigned char *const gray_img, const int height, const int width)
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // Find the starting index of the current pixel in the RGBA array
            int pixel_idx = (y * width + x) * 4;

            unsigned char r = img[pixel_idx];     // Red
            unsigned char g = img[pixel_idx + 1]; // Green
            unsigned char b = img[pixel_idx + 2]; // Blue

            unsigned char gray = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);

            // Store in the new 1-channel array
            gray_img[y * width + x] = gray;
        }
    }
}

void serial_gaussian_blur(unsigned char *const in, unsigned char *out, int w, int h)
{
    int kernel[5][5] = {
        {1, 4, 7, 4, 1},
        {4, 16, 26, 16, 4},
        {7, 26, 41, 26, 7},
        {4, 16, 26, 16, 4},
        {1, 4, 7, 4, 1}};
    int kernel_sum = 273;

    for (int y = 2; y < h - 2; y++)
    {
        for (int x = 2; x < w - 2; x++)
        {
            int sum = 0;

            // Iterate through the 5x5 neighborhood
            for (int ky = -2; ky <= 2; ky++)
            {
                for (int kx = -2; kx <= 2; kx++)
                {
                    int pixel = in[(y + ky) * w + (x + kx)];
                    sum += pixel * kernel[ky + 2][kx + 2];
                }
            }
            out[y * w + x] = (unsigned char)(sum / kernel_sum);
        }
    }
}

void serial_edge_detect(unsigned char *const in, unsigned char *out, const int width, const int height)
{
    // 3x3 Laplacian Kernel
    int kernel[3][3] = {
        {0, 1, 0},
        {1, -4, 1},
        {0, 1, 0}};

    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
        {
            int sum = 0;

            // Convolution
            for (int ky = -1; ky <= 1; ky++)
            {
                for (int kx = -1; kx <= 1; kx++)
                {
                    int pixel = in[(y + ky) * width + (x + kx)];
                    sum += pixel * kernel[ky + 1][kx + 1];
                }
            }

            int edge = abs(sum);
            out[y * width + x] = (unsigned char)(edge > 255 ? 255 : edge);
        }
    }
}

#endif // SERIAL_H