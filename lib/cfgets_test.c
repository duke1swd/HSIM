/*
 * This program tests cfgets.
 * The program uses itself as test data
6
this line read with five char buff leng
5
this line read with four char buff leng
21
xxxxxxxxxxxxxxxxxxxx
now # is the
# is the line
not \# a comment
now is the winter of our discontent
now is the winter o\f
now \
is the winter
now # \
is the winter
now \\s the winter # of our discontent
EOT
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "rsim.h"

static char lbuf[128];
char *myname;

static void
usage()
{
	fprintf(stderr, "Usage: %s < input_flie\n", myname);
	exit(1);
}

int
main(int argc, char **argv)
{
	int len;
	int lineno;

	myname = *argv;

	if (argc != 1)
		usage();

	len = sizeof lbuf;
	lineno = 0;

	while (cfgets(lbuf, len, stdin) == lbuf) {
		lineno++;
		if (isdigit(lbuf[0])) {
			len = atoi(lbuf);
			printf("len is now %d\n", len);
		} else if (strcmp(lbuf, "EOT\n") == 0)
			break;
		else
			printf("%3d: .%s.\n", lineno, lbuf);
	}
}
