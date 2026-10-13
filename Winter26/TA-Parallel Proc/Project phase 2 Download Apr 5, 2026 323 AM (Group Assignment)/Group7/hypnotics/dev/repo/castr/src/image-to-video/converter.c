#include "../image-to-video.h"

#include <stdio.h>
#include <stdlib.h>

int convert_to(const char *out, const int N){
    char cmd[512];

    snprintf(cmd, sizeof(cmd), "ffmpeg -y -framerate 60 -i frames/%%05d.ppm -c:v libx264 -pix_fmt yuv420p %s", out);

    return system(cmd);
}



void write_frame(const int step, const int *dataset, const int N, const int K){
    char filename[256];
    snprintf(filename, sizeof(filename), "frames/%05d.ppm", step);

    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("fopen");
        return;
    }

    fprintf(f, "P6\n%d %d\n255\n", N, N);

    for (int i = 0; i < N * N; i++) {

        int s = dataset[i];

        unsigned char r = (s * 53) % 256;
        unsigned char g = (s * 97) % 256;
        unsigned char b = (s * 193) % 256;

        fwrite(&r, 1, 1, f);
        fwrite(&g, 1, 1, f);
        fwrite(&b, 1, 1, f);
    }

    fclose(f);
}
