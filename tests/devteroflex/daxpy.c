#define N (1024*1024)

int xs[N];
int ys[N];

void daxpy(int size, int a) {
    for(int i = 0; i < size; i++) {
        ys[i] = a * xs[i] + ys[i];
    }
}

int main() {
    for(int i = 0; i < N; i++) {
        xs[i] = ys[i] = i;
    }

    int a = 5;

    daxpy(N, a);
    return ys[0];
}
