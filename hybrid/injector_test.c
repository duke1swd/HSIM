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
 * Attempt to compare the injector model with HDAS 1.5
 *
 * Input
 *  n2o_liquid_density
 *  tank_pressure
 *  chamber_pressure
 *  injector_area
 *  injector_cd
 *
 * Output
 *  n2o_flow_rate
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "ts_parse.h"
#include "scio.h"
#include "linkage.h"
#include "state.h"

#define	L_UNIT	"in"
#define	D_UNIT	"lbm/ft3"
#define	P_UNIT	"psi"
#define	MF_UNIT	"lbm/sec"

static double injectordia;

struct scio_input_parameter_s scio_input[] = {
{
	"density",
	DENSITY,
	REQUIRED,
	&n2o_liquid_density,
	1,
	0,
},
{
	"tankpressure",
	PRESSURE,
	REQUIRED,
	&tank_pressure,
	1,
	0,
},
{
	"chamberpressure",
	PRESSURE,
	REQUIRED,
	&chamber_pressure,
	1,
	0,
},
{
	"injectordia",
	LENGTH,
	REQUIRED,
	&injectordia,
	1,
	0,
},
{
	"injectorcd",
	NUMBER,
	REQUIRED,
	&injector_cd,
	1,
	0,
},

};

static void
report_input()
{
	printf("Inputs:\n");
	printf("\tN2O Liquid Density: %12.5f (%s)\n", 
			scio_convert(n2o_liquid_density, DENSITY, D_UNIT),
				D_UNIT);
	printf("\t     Tank Pressure: %12.5f (%s)\n",
			scio_convert(tank_pressure, PRESSURE, P_UNIT), P_UNIT);
	printf("\t  Chamber Pressure: %12.5f (%s)\n",
			scio_convert(chamber_pressure,
				PRESSURE, P_UNIT), P_UNIT);
	printf("\t      Injector Dia: %12.5f (%s)\n",
			scio_convert(injectordia, LENGTH, L_UNIT), L_UNIT);
	printf("\t       Injector Cd: %12.5f\n", injector_cd);
}

static void
report_output()
{
	printf("\nOutput:\n");
	printf("\t     N2O Flow Rate: %12.5f (%s)\n",
			scio_convert(n2o_flow_rate, MASSFLOW, MF_UNIT),
				MF_UNIT);
}
	

char *myname;

static void
initialize()
{
	struct ts_parsed_s *input_buffer;
	FILE *input;

	constants_init();

	ts_parse_init();
	scio_init(scio_input, sizeof (scio_input) / sizeof (scio_input[0]));

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

	injector_count = 1;
	injector_area = pi/4. * injectordia * injectordia;
}


void
doit()
{
	injector();
}

int
main(int argc, char **argv)
{
	myname = *argv; 

	initialize();
	doit();
	report_input();
	report_output();

	exit(0);
}
