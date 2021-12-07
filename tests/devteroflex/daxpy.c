
volatile int xs[1024*1024];
volatile int ys[1024*1024];

void daxpy(int size, int a) {
    for(int i = 0; i < size; i++) {
        ys[i] = a * xs[i] + ys[i];
    }
}

int main() {
    int a = 5;

    daxpy(1024*1024, a);
    return ys[0];
}
