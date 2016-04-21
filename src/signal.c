#include "signal.h"

#if defined(_CAP_WINDOWS)
// Nothing todo
#else

sighandler_t
signal(int sig, sighandler_t handler) {
	struct sigaction newdisp, prevdisp;

	newdisp.sa_handler = handler;
	sigemptyset(&newdisp.sa_mask);
	
#ifdef OLD_SIGNAL
	newdisp.sa_flags = SA_RESETHAND | SA_NODEFER;
#else
	newdisp.sa_flags = SA_RESTART;
#endif

	if (sigaction(sig, &newdisp, &prevdisp) == -1) {
		return SIG_ERR;
	}

	return prevdisp.sa_handler;
}

#endif

/**************
* signal test *
**************/

#if defined(_TEST_SIGNAL)
void
handler(int sig) {
	if (sig == SIGINT) {
		printf("SIGINT\n");
		signal(sig, handler);
	} else {
		printf("sig[%d]\n", sig);
	}

	fflush(stdout);
}

int
main(int argc, char* argv[]) {
	signal(SIGINT, handler);

	for (;;) {
		getchar();
	}

    return 0;
}
#endif
