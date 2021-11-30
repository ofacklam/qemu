#include <stdio.h>

// 	a b
// 	c d
typedef struct {
	int a, b, c, d;
} matrix4x4_t;

const matrix4x4_t identity = (matrix4x4_t) {
	1, 0,
	0, 1
};

const matrix4x4_t fib_mat = (matrix4x4_t) { 
	1, 1,
	1, 0
};

matrix4x4_t mult(matrix4x4_t A, matrix4x4_t B) {
	return (matrix4x4_t) {
		A.a*B.a + A.b*B.c,		A.a*B.b + A.b*B.d,
		A.c*B.a + A.d*B.c,    A.c*B.b + A.d*B.d
	};
}

matrix4x4_t mat_pow(matrix4x4_t A, unsigned int n) {
	// INVARIANT: correct result == A^n * R;
	matrix4x4_t R = identity;
	while (n != 0) {
		if (n&1) {
			R = mult(A, R);
			n -= 1;
		} else {
			A = mult(A, A);
			n /= 2;
		}
	}
	return R;
}

int fib(unsigned n) {
	if (n == 0) return 0;
	
	matrix4x4_t ret = mat_pow(fib_mat, n);
	return ret.a;
}

int main() {
	for (int i = 0; i < 20; i++)
		printf("%d\n", fib(i));

	return 0;
}

