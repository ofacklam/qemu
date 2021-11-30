#include <stddef.h>
#include <sys/time.h>

#define Mil 1000000

int main() {
	
	struct timeval tp;
	gettimeofday(&tp, NULL);
	long int ms = tp.tv_sec * Mil + tp.tv_usec;
	long int const next_full_2_seconds = ms - (ms % 2*Mil) + 2*Mil;

	do {
		gettimeofday(&tp, NULL);
		ms = tp.tv_sec * Mil + tp.tv_usec;
	} while (!(ms >= next_full_2_seconds));

	return 0;
}

