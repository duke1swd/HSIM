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
 * Parse the liquid fuel csv file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "liquid_fuel.h"
#include "rsim.h"

/*
 * Define the liquid fuel data file
 */
#define	LIQUID_FUEL	"liquidfuel.csv"

static char *columns[] ={
	"Fuel",
	"Density",
	"cpropep",
};

#define	NCOL	((sizeof columns)/(sizeof columns[0]))


#define	FUEL				0
#define	LIQUID_FUEL_DENSITY		1
#define	LIQUID_FUEL_CPROPEP		2

extern char *myname;

int
liquid_fuel_data(char *liquid_fuel_name, struct liquid_fuel_data_s *fp)
{
	int j;
	FILE *input;
	char buffer[256];
	char *ptrs[NCOL];

	if (!liquid_fuel_name) {
		fprintf(stderr, "%s: no fuel specified\n",
			myname);
		return 1;
	}

	input = fopen(LIQUID_FUEL, "r");
	if (input == NULL) {
		fprintf(stderr, "%s: cannot open data file %s for reading.\n",
			myname, LIQUID_FUEL);
		perror("open");
		return 1;
	}

	if (csv_read(input, buffer, sizeof buffer, ptrs, NCOL) == 0) {
		fprintf(stderr, "%s: data file %s is empty\n",
			myname, LIQUID_FUEL);
		return 1;
	}

	for (j = 0; j < NCOL; j++) {
		if (ptrs[j] == (char *)0) {
			fprintf(stderr, "%s: error in %s.  "
					"Column %d is blank\n",
					myname,
					LIQUID_FUEL,
					j + 1);
			exit(1);
		}
				
		if (strcmp(ptrs[j], columns[j]) != 0) {
		    fprintf(stderr, "%s: bad column heading in "
					"data file %s\n",
			myname, LIQUID_FUEL);
		    fprintf(stderr, "\tExpected \'%s\' got \'%s\' "
					"in column %d\n",
			columns[j], ptrs[j], j);
		    return 1;
		}
	}

	while (csv_read(input, buffer, sizeof buffer, ptrs, NCOL))
		if (strcasecmp(ptrs[FUEL], liquid_fuel_name) == 0)
			goto found;

	/* The supplied fuel is not a liquid fuel */
	fclose(input);
	return 1;

    found:
	fclose(input);
	if (fp) {
		fp->fuel_density = atof(ptrs[LIQUID_FUEL_DENSITY]);
		fp->cpropep = atoi(ptrs[LIQUID_FUEL_CPROPEP]);
	}

	return 0;
}
