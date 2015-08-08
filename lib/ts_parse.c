/*
  This file is a portion of Hsim 0.1
 
  Hsim is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published
  by the Free Software Foundation; either version 2 of the License,
  or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */

/*
 * THE PARSER.
 *
 * This reads a newline terminated string
 * from the supplied stdio FILE and returns
 * it parsed into substrings according to the rules.
 *
 * All messages are read using this routine.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ts_parse.h"

#define	DEFAULT_BUFFER_SIZE	512
#define	MAX_BUFFER_SIZE		(512*16)

static int linecounter;

/*
 * Initialization.
 */
void
ts_parse_init()
{
	linecounter =0;
}

/*
 * Fatal Error Exit.
 */
void
ts_fatal(char *msg)
{
	extern char *myname;

	fprintf(stderr, "%s: Fatal Error on line %d: %s\n",
		myname, linecounter, msg);
	exit(1);
}

/*
 * Make the string portion of the buffer twice as big.
 * Assumes there are no valid pointers (yet).
 */
static void
realloc_buffer(struct ts_parsed_s *buffer)
{
	int old_size;

	old_size = buffer->mem_size;
	buffer->mem_size *= 2;
	buffer->mem = realloc(buffer->mem, buffer->mem_size);
	if (buffer->mem == (char *)0)
		ts_fatal("malloc failure");
	bzero(buffer->mem + old_size, old_size);
}

void
ts_parse_free(struct ts_parsed_s *buffer)
{
	free((char *)(buffer->words));
	free((char *)(buffer->mem));
	free((char *)buffer);
}

struct ts_parsed_s *
ts_parse(FILE *in, struct ts_parsed_s *buffer)
{
	int c;
	int bytes_read;
	int count;
	int state, oldstate;
	char *p;
	char *q;
	char **pp;
	char *last_p;

	linecounter++;

	/*
	 * If we don't have a buffer, get one. 
	 */
	if (buffer == NULL) {
		buffer = (struct ts_parsed_s *)
			malloc(sizeof (struct ts_parsed_s));
		if (buffer == NULL)
			ts_fatal("malloc failure");
		buffer->mem = malloc(DEFAULT_BUFFER_SIZE);
		if (buffer->mem == NULL)
			ts_fatal("malloc failure");

		buffer->mem_size = DEFAULT_BUFFER_SIZE;
		buffer->words = (char **)0;
	}

	/*
	 * Loop, reading in the line, however long it may be.
	 * This is the right place to process continuation lines,
	 * but continuations lines are not yet implemented.
	 */
    read_again:
	bytes_read = 0;
	bzero(buffer->mem, buffer->mem_size);
	for (;;) {
		fgets(buffer->mem + bytes_read,
			buffer->mem_size - bytes_read, in);
		if (feof(in) || ferror(in))
			return NULL;
		c = buffer->mem[buffer->mem_size - 2];
		if (c == '\n' || c == '\0')
			break;	/* got the entire line. */
		bytes_read = buffer->mem_size - 1;
		realloc_buffer(buffer);
	}
	
	/*
	 * First Pass.
	 *
	 * Loop through the buffer, null terminating the strings.
	 * Count the strings to make sure we have enough space
	 * for the pointers.
	 * End of line is a double null character.
	 */

	/* First make sure there is room for the double NULL terminator. */
	if (c == '\n')
		realloc_buffer(buffer);
	
	count = 0;
	state = 0;  /* states 0: nada,
			      1: working on a string
			      2: working on a quoted string
			      3: working on a '\' escaped character
		     */
	oldstate = 0; /* only used by state 3 */
	last_p = buffer->mem;
	for (p = buffer->mem; *p; p++) {
		if (*p == '\n') {
			if (state == 3) {
				/* line ends with '\', probably
				 * an continuation line.  NYI,
				 * so ignore it.
				 */
				p--;
			}
			if (state != 0)
				last_p = p;
			break;
		} else if (state == 3) {
			if (oldstate == 0) {
				count++;
				state = 1;
			} else 
				state = oldstate;
		} else if (*p == '\\') {
			oldstate = state;
			state = 3;
		} else if (*p == ' ' || *p == '\t' || *p == '\r') {
			if (state == 1) {
				*p = '\0';
				last_p = p;
				state = 0;
			}
		} else if (*p == '"') {
			if (state == 0) {
				state = 2;
				count++;
			} else if (state == 2) {
				state = 1;
			} else if (state == 1) {
				state = 2;
			}
		} else if (*p == '#') {
			if (state != 0)
				last_p = p;
			break;
		} else {
			if (state == 0) {
				count++;
				state = 1;
			}
		}
	}
	*last_p++ = '\0';
	*last_p = '\0';

	/*
	 * If we didn't get any valid strings, try again.
	 * This is how we ignore comments and blank lines.
	 */
	if (count == 0)
		goto read_again;

	/*
	 * Second Pass.
	 *
	 * Loop through assigning pointers.
	 */

	/* First allocate room for the pointers. */
	buffer->words = (char **)realloc((char *)(buffer->words),
		(count + 1) * (sizeof (char *)));
	if (buffer->words == (char **)0)
		ts_fatal("malloc failure");

	state = 0;	/* 0 = nothing yet
			 * 1 = working on a string
			 * 2 = found one null.
			 */
	pp = buffer->words;
	for (p = buffer->mem; ; p++) {
		if (*p) {
			if (state != 1)
				*pp++ = p;
			state = 1;
		} else {
			if (state == 2)
				break;
			state = 2;
		}
	}
	*pp = (char *)0;

	if (pp - buffer->words != count)
		ts_fatal("INTERNAL ERROR: ts_parse pass 2");

	/*
	 * Third Pass.
	 *
	 * Canonicalize each string:
	 *   - Strip leading whitespace
	 *   - Pull out quotes
	 *   - Process escapes
	 */
	for (pp = buffer->words; *pp; pp++) {
		/* copy from p to q */
		q = *pp;
		state = 0;  /* states 0: nada,
				      1: working on a string
				      2: working on a quoted string
				      3: working on a '\' escaped character
			     */
		oldstate = 0; /* only used by state 3 */
		for (p = *pp; *p; p++) {
			if (state == 3) {
				*q++ = *p;
				if (oldstate == 0)
					state = 1;
				else 
					state = oldstate;
			} else if (*p == '\\') {
				oldstate = state;
				state = 3;
			} else if (*p == ' ' || *p == '\t' || *p == '\r') {
				if (state != 0)
					*q++ = *p;
			} else if (*p == '"') {
				if (state == 0 || state == 1) {
					state = 2;
				} else if (state == 2) {
					state = 1;
				}
			} else {
				*q++ = *p;
				if (state == 0)
					state = 1;
			}
		}
		*q = '\0';
	}

	return buffer;
}

#ifdef DEBUG
char *myname;

int
main(int argc, char **argv)
{
	char **pp;
	struct ts_parsed_s *buffer;

	myname = *argv;

	buffer = NULL;

	for (;;) {
		buffer = ts_parse(stdin, buffer);
		if (feof(stdin))
			break;
	
		printf("\nCanonicalized Strings are:\n");
		for (pp = buffer->words; *pp; pp++)
			printf("\t.%s.\n", *pp);
	}
	return 0;
}
#endif
