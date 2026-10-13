#include <stdio.h>

#define N 1000003

void stage1_saxpy(const float *a, const float *x, const float *y, float *out, int start, int end){
    if (start < 0 || end < start )
        return;
    for (int i=start; i < N && i<end; i++){
        out[i] = a[i]*x[i]*y[i];
    }
}

void stage2_prefix_like(float *out, int start, int end){
    if (start < 1 || end < start )
        return;
    for (int i=start; i < N && i < end; i++){
        out[i] = out[i] + out[i-1];
    }
}
