/*
 * cfgets
 *
 * This function acts like fgets(3), but
 *  - comments are allowed.  '#' introduces a comment and runs till end of line
 *  - the sequence "\#" is mapped to '#' and does not introduce a comment.
 *  - '\' followed by anything other than '#' or '\n' goes into the buffer
 *  - lines that are blank, have only white space and comments are ignored
 *  - lines may be continued by ending them with '\'.  Lines that end in a
 *  	comment cannot be continued
 */

#include <stdio.h>
#include "rsim.h"

#define	STATE_0		0	// starting state
#define	STATE_BACKSLASH	1	// found a backslash.
#define	STATE_COMMENT	2	// found a comment, seeking '\n'

char *
cfgets(char *buffer, int size, FILE *input)
{
	int c;
	char *p;
	unsigned char state;

	state = STATE_0;
	p = buffer;

	// defensive programming
	if (size < 2)
		return NULL;
	// make room for the null terminator
	size -= 1;

	while ((c = fgetc(input)) != EOF) {
	    switch (state) {
	    default:	// never!
	    case STATE_0:
		switch (c) {
		case '\\':
			state = STATE_BACKSLASH;
			break;
		case '#':
			state = STATE_COMMENT;
			break;
		case '\n':
			*p++ = c;
			goto done;
		default:
			*p++ = c;
			if (p - buffer >= size)
				goto done;
			break;
		}
		break;

	    case STATE_COMMENT:
		if (c == '\n')
			goto done;
		break;

	    case STATE_BACKSLASH:
		switch (c) {
		
		// for most characters, "\x" goes into the buffer as "\x".
		default:
			*p++ = '\\';
			if (p - buffer >= size) {
				ungetc(c, input);
				goto done;
			}
			// note fall thru

		// for the characters in the following "case" statements, "\x" goes into the buffer as 'x'.
		case '#':
		case '\\':
			*p++ = c;
			if (p - buffer >= size)
				goto done;
			// note fall thru

		// for backslash followed by newline, nothing goes into the buffer.
		case '\n':
			state = STATE_0;
			break;
		}
	    }
	}
	if (p == buffer)
		return NULL;
    done:
	*p = '\0';

	return buffer;
}
