#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>

#define Mil 1000000

void alarmRinging(int);

int main() {

	struct timeval tp;
	gettimeofday(&tp, NULL);
	long int const ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;

  if (signal(SIGALRM, (void (*)(int)) alarmRinging) == SIG_ERR) {
    perror("Unable to regiser signal handler");
    exit(1);
  }

	long int const ms_to_sleep = 2000 - (ms % 2000);

	struct itimerval it_val;
	it_val.it_interval.tv_sec  = 0;
	it_val.it_interval.tv_usec = 0;
	it_val.it_value.tv_sec  =  ms_to_sleep / 1000;
	it_val.it_value.tv_usec = (ms_to_sleep * 1000) % Mil;

  if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
    perror("error calling setitimer()");
    exit(1);
  }

	while (1) pause();

	return 0;
}

void alarmRinging(int signo) {
	exit(0);
}

