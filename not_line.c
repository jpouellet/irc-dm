#include <stdio.h>
#include <unistd.h>

int
main()
{
	printf("no \\n");
	fflush(stdout);
	sleep(1);
	printf("\n");
	fflush(stdout);
	sleep(1);
	printf("line1\nline2\n");
	fflush(stdout);
	sleep(1);
	printf("asdf");
	fflush(stdout);
	sleep(1);
	printf("blah");
	sleep(1);
	return 0;
}
