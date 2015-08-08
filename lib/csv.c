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
 * Read in a line from a comma-separated-value file.
 *
 * Parameters:
 *	input	An open stdio stream
 *	buffer	a character buffer with enough room to read the line
 *	size	sizeof(buffer)
 *	ptrs	an array of pointers to the delimited strings
 *	n	number of points
 *
 * Returns:
 *	number of values pointed to by the pointers
 *	0 on EOF
 *	-1 on any error.  Errors are printed to stderr.
 */

#include <stdio.h>
#include <strings.h>

char *myname;

int
csv_read(FILE *input, char *buffer, int size, char **ptrs, int n)
{
	int i;
	int state;
	char *p;

	do {
		(void)fgets(buffer, size, input);

		if (feof(input))
			return 0;

		if (buffer[strlen(buffer)-1] != '\n' &&
		    buffer[strlen(buffer)-1] != '\r') {
			fprintf(stderr, "%s: input line is longer than "
				"%d chars\n",
				myname, size);
			buffer[40] = '\0';
			fprintf(stderr, "  line begins: %s\n", buffer);
			fprintf(stderr, "%s: csv read aborting\n", myname);
			return -1;
		}

		
		i = 0;
		state = 1;
		for (p = buffer; *p; p++) {

			if (*p == '\n' || *p == '\r' || *p == '#') {
				*p = '\0';
				break;
			}

			if (state) {
				if (i >= n) {
					fprintf(stderr, "%s: input line has"
							" more than "
							"%d fields\n",
						myname, n);
					fprintf(stderr, "  first field is: %s\n",
						buffer);
					fprintf(stderr, "%s: csv read "
						"aborting\n",
						myname);
					return -1;
				}
				ptrs[i++] = p;
				state = 0;
			}

			if (*p == ',') {
				*p = '\0';
				state = 1;
			}
		}
	} while (i == 0);

	return i;
}
