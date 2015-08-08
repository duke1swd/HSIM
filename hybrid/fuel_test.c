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
 * Attempt to compare the fuel regression model with HDAS 1.5
 *
 * Input is port diameter, grain length, and oxidizer mass flow rate.
 * Output is fuel burn rate in lbm/sec
 */

#include <stdio.h>
#include <stdlib.h>
#include "ts_parse.h"
#include "scio.h"
#include "linkage.h"
#include "state.h"

#define	L_UNIT	"in"
#define	MF_UNIT	"lbm/sec"
#define	V_UNIT	"in/sec"

struct scio_input_parameter_s scio_input[] = {
{
	"flux",
	MASSFLOW,
	REQUIRED,
	&n2o_flow_rate,
	1,
	0,
},
{
	"port",
	LENGTH,
	REQUIRED,
	&grain_core,
	1,
	0,
},
{
	"length",
	LENGTH,
	REQUIRED,
	&grain_length,
	1,
	0,
},

};

static void
report_input()
{
	printf("Inputs:\n");
	printf("\t      Mass Flow: %12.5f (%s)\n", 
			scio_convert(n2o_flow_rate, MASSFLOW, MF_UNIT), MF_UNIT);
	printf("\t       Port Dia: %12.5f (%s)\n",
			scio_convert(grain_core, LENGTH, L_UNIT), L_UNIT);
	printf("\t   Grain Length: %12.5f (%s)\n",
			scio_convert(grain_length, LENGTH, L_UNIT), L_UNIT);
}

static void
report_output()
{
	printf("\nOutputs:\n");
	printf("\tRegression Rate: %12.5f (%s)\n",
			scio_convert(fuel_rb, VELOCITY, V_UNIT), V_UNIT);
	printf("\t      Fuel Flux: %12.5f (%s)\n",
			scio_convert(fuel_flow_rate, MASSFLOW, MF_UNIT), MF_UNIT);
}
	

char *myname;

static void
initialize()
{
	struct ts_parsed_s *input_buffer;
	FILE *input;

	constants_init();
	fuel_init();

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
}


void
doit()
{
	/*
	 * calculate the mass of fuel remaining.
	 */
	grain_diameter = grain_core + .002;
	fuel_mass = fuel_density * pi / 4. * grain_length *
		(grain_diameter * grain_diameter - grain_core * grain_core);
	fuel_regression();
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
