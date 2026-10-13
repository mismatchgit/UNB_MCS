#include <stdlib.h>
#include <stdio.h>

#define N 1000003 // note: not divisible by 4
#define P 4

void stage1(const float *a, const float *x, 
	const float *y, float *out, const int START, const int END) {

	for (int i = START; i < END; i++) {
		out[i] = a[i] * x[i] + y[i];
	}
}


void exeStg1(const float *a, const float *x, const float *y, float *out, int proc_num){
	int chunk_size = N/P;
	int end = chunk_size*(proc_num+1);
	int start = end-chunk_size;

	if(proc_num == (P-1))
		end += N%P;
	
	stage1(a, x, y, out, start, end);
}

void stage2_prefix_like(float *out) {
	for (int i = 1; i < N; i++) {
		out[i] = out[i] + out[i - 1];
	}
}

int main(){
	return 0;
}