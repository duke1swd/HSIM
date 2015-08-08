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
 * THE PARSER
 */

/* A parsed line of something */

struct ts_parsed_s {
	char **words;	/* null terminated array of pointers */
	char *mem;	/* the allocated memory for this */
	unsigned mem_size; /* how big is this buffer? */
};

/* Entry points */

	/*
	 * Parses a line from the FILE.
	 * buffer may be null, in which case a new struct is returned.
	 * Otherwise, returns the buffer.
	 * If there was an error or EOF on the input, then no
	 * words will be returned.
	 */
struct ts_parsed_s *ts_parse(FILE *in, struct ts_parsed_s *buffer);

	/*
	 * Frees a buffer that was returned by ts_parse().
	 */
void ts_parse_free(struct ts_parsed_s *buffer);

	/*
	 * Initialization.
	 */
void ts_parse_init();
