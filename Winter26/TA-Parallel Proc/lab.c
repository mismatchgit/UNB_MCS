#include <stdlib.h>
#include <stdio.h>


typedef struct {
  int start,end;
} Range;

void stage1_saxpy(const float *a, const float *x, const float *y, float *out, Range range) {
  for (int i = range.start; i < range.end; i++) {
    out[i] = a[i] * x[i] + y[i];
  }
}

void stage2_prefix_like(float *out, Range range) {
  for (int i = range.start; i < range.end; i++) {
    out[i] = out[i] + out[i - 1];
  }
}

int main(void) {
  printf("Hello World!\n");
  return 0;
}
