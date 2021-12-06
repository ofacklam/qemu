#define N 1000

int A[N][N], B[N][N], C[N][N];

int main() {

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			A[i][j] = i+j;
			B[i][j] = i*j;
		}
	}

	for (int j = 0; j < N; j++) {
		for (int i = 0; i < N; i++) {
			for (int k = 0; k < N; k++) {
				C[i][j] += A[i][k] * C[k][j];
			}
		}
	}

	return 0;
}

