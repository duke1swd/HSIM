/*
  This file is a portion of Hsim 0.2
 
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
 * Test the n2o_thermo routines.
 */

#include <stdio.h>
#include <stdlib.h>
#include "ts_parse.h"
#include "scio.h"
#include "state.h"
#include "linkage.h"

#define	P_UNIT	"psi"
#define	T_UNIT	"F"
#define	D_UNIT	"g/cc"
#define	E_UNIT	"kJ/kg"

int nt;
double	t[20];

struct scio_input_parameter_s scio_input = {
	"temp",
	TEMPERATURE,
	REQUIRED,
	t,
	sizeof t / sizeof t[0],
	&nt,
};
	

char *myname;

static void
doit()
{
	int i;
	double temp;
	struct ts_parsed_s *input_buffer;
	FILE *input;

	constants_init();
	n2o_thermo_init();

	ts_parse_init();
	scio_init(&scio_input, 1);

	/*
	 * Read until EOF
	 */
	input = stdin;
	input_buffer = NULL;

	for (;;) {
		input_buffer = ts_parse(input, input_buffer);
		if (!input_buffer)
			break;
		scio_input_line(input_buffer);
	}

	scio_term();
	printf("%d temperatures loaded\n", nt);
	printf("\n");

	printf("  TEMP    PRESS   ");
	printf("V DENS  L DENS  ");
	printf("V ENER  L ENER  ");
	printf(" Cp/Cv");
	printf("\n");

	printf("  Deg %1s   %3s   ", T_UNIT, P_UNIT);
	printf(" %6s  %6s ", D_UNIT, D_UNIT);
	printf(" %6s  %6s ", E_UNIT, E_UNIT);
	printf("\n");

	for (i = 0; i < nt; i++) {
		n2o_thermo_error = 0;

		temp = t[i];

		printf("%7.1f  ",
			scio_convert(temp, TEMPERATURE, T_UNIT));
		printf("%6.1f  ",
			scio_convert(saturation_pressure(temp),
				PRESSURE, P_UNIT));
		printf("%6.1f  ",
			scio_convert(vapor_density(temp),
				DENSITY, D_UNIT));
		printf("%6.1f  ",
			scio_convert(liquid_density(temp),
				DENSITY, D_UNIT));
		printf("%6.1f  ", vapor_energy(temp)/1000.);
		printf("%6.1f  ", liquid_energy(temp)/1000.);
		printf("%6.1f  ", cpcv(temp));

		printf("\n");

		if (n2o_thermo_error) {
			fprintf(stderr, "%s: n2o errors", myname);
			if (n2o_thermo_error == -1)
				fprintf(stderr, ": too cold");
			if (n2o_thermo_error == -2)
				fprintf(stderr, ": too hot");
			fprintf(stderr, "\n");
		}
	}
}

int
main(int argc, char **argv)
{
	myname = *argv; 

	doit();

	exit(0);
}
