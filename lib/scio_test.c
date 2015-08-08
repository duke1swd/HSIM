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
 * Test routine for scio.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ts_parse.h"
#include "scio.h"

/*
 * Define my input parameters.
 */
#define	MAX_HEIGHTS	5
double	heights[MAX_HEIGHTS];
int n_heights;

#define	MAX_VOLUMES	2
double	volumes[MAX_VOLUMES];
int n_volumes;

#define	MAX_NAMES	2
char *	names[MAX_NAMES];
int n_names;

struct scio_input_parameter_s inputs[] = {
{"height", LENGTH, 0, heights, MAX_HEIGHTS, &n_heights},
{"volume", VOLUME, 0, volumes, MAX_VOLUMES, &n_volumes},
{"name",   STRING, 0, names,   MAX_NAMES,   &n_names},
};

#define	N_INPUT_PARAMS	((sizeof inputs) / (sizeof inputs[0]))

char *myname;

void
print_inputs()
{
	int i;

	printf("Allowed inputs are:\n");

	for (i = 0; i < N_INPUT_PARAMS; i++)
		printf("\t%s\n", inputs[i].name);
}

void
print_values()
{
	int i, k;

	for (i = 0; i < N_INPUT_PARAMS; i++)
	    if (*(inputs[i].nvp)) {
	    	printf("%s: ", inputs[i].name);
		for (k = 0; k < *(inputs[i].nvp); k++)
		    if (inputs[i].unit == STRING)
			printf(" %s", ((char **)(inputs[i].vp))[k]);
		    else
			printf(" %lf", ((double *)(inputs[i].vp))[k]);
		printf("\n");
	    }
}

int
main(int argc, char **argv)
{
	struct ts_parsed_s *buffer;
	myname = *argv;

	print_inputs();

	ts_parse_init();
	scio_init(inputs, N_INPUT_PARAMS);
	buffer = NULL;
	while ((buffer = ts_parse(stdin, buffer))) {
		scio_input_line(buffer);
	}
	scio_term();
	print_values();
	return 0;
}
