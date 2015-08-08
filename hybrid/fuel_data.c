/*
  This file is a portion of Hsim 0.4
 
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
 * This routine is an access method for fuel.csv
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fuel.h"
#include "rsim.h"

extern char *myname;

/*
 * fuel.csv definitions
 */
#define	FUEL	"fuel.csv"

static char *columns[] ={
	"fuel",
	"N",
	"A",
	"K",
	"Density",
	"cpropep",
};

#define	NCOL	((sizeof columns)/(sizeof columns[0]))

#define	FUEL_NAME	0
#define	FUEL_N		1
#define	FUEL_A		2
#define	FUEL_K		3
#define	FUEL_DENSITY	4
#define FUEL_CPROPEP	5

int
fuel_data(char *fuel_name, struct fuel_data_s *fp)
{
	int j;
	FILE *input;
	char buffer[256];
	char *ptrs[NCOL];

	if (!fuel_name) {
		fprintf(stderr, "%s: no fuel specified\n",
			myname);
		return 1;
	}

	input = fopen(FUEL, "r");
	if (input == NULL) {
		fprintf(stderr, "%s: cannot open data file %s for reading.\n",
			myname, FUEL);
		perror("open");
		return 1;
	}

	if (csv_read(input, buffer, sizeof buffer, ptrs, NCOL) == 0) {
		fprintf(stderr, "%s: data file %s is empty\n",
			myname, FUEL);
		return 1;
	}

	for (j = 0; j < NCOL; j++) {
		if (ptrs[j] == (char *)0) {
			fprintf(stderr, "%s: error in fuel.csv.  "
					"Column %d is blank\n",
					myname,
					j + 1);
			exit(1);
		}
				
		if (strcmp(ptrs[j], columns[j]) != 0) {
		    fprintf(stderr, "%s: bad column heading in "
					"data file %s\n",
			myname, FUEL);
		    fprintf(stderr, "\tExpected \'%s\' got \'%s\' "
					"in column %d\n",
			columns[j], ptrs[j], j);
		    return 1;
		}
	}

	while (csv_read(input, buffer, sizeof buffer, ptrs, NCOL))
		if (strcasecmp(ptrs[0], fuel_name) == 0)
			goto found;

	/* The supplied fuel is not a solid fuel */
	fclose(input);
	return 2;

    found:
	fclose(input);
	if (fp) {
		fp->fuel_n = atof(ptrs[FUEL_N]);
		fp->fuel_a = atof(ptrs[FUEL_A]);
		fp->fuel_k = atof(ptrs[FUEL_K]);
		fp->fuel_density = atof(ptrs[FUEL_DENSITY]);
		fp->cpropep = atoi(ptrs[FUEL_CPROPEP]);
	}

	return 0;
}
