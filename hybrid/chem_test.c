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
 * Simple test of the cpropep interface.
 *
 * Inputs
 *  n2o flow rate
 *  fuel flow rate
 *  chamber_pressure
 *  nozzle throat diameter
 *  nozzle exit diameter
 *
 * Outputs
 *  C*
 *  exit pressure
 *  nozzle cf
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "ts_parse.h"
#include "scio.h"
#include "linkage.h"
#include "state.h"

#define	L_UNIT	"in"
#define	V_UNIT	"fps"
#define	P_UNIT	"psi"
#define	MF_UNIT	"lbm/sec"

static double noz_t_dia, noz_e_dia;

struct scio_input_parameter_s scio_input[] = {
{
	"n2oflow",
	MASSFLOW,
	REQUIRED,
	&n2o_flow_rate,
	1,
	0,
},
{
	"fuelflow",
	MASSFLOW,
	REQUIRED,
	&fuel_flow_rate,
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
	"nozzlethroat",
	LENGTH,
	REQUIRED,
	&noz_t_dia,
	1,
	0,
},
{
	"nozzleexit",
	LENGTH,
	REQUIRED,
	&noz_e_dia,
	1,
	0,
},

};

static void
report_input()
{
	printf("Inputs:\n");
	printf("\t   N2O Flow Rate: %12.5f (%s)\n", 
			scio_convert(n2o_flow_rate, MASSFLOW, MF_UNIT),
				MF_UNIT);
	printf("\t  Fuel Flow Rate: %12.5f (%s)\n", 
			scio_convert(fuel_flow_rate, MASSFLOW, MF_UNIT),
				MF_UNIT);
	printf("\tChamber Pressure: %12.5f (%s)\n",
			scio_convert(chamber_pressure,
				PRESSURE, P_UNIT), P_UNIT);
	printf("\t   Nozzle Throat: %12.5f (%s)\n",
			scio_convert(noz_t_dia, LENGTH, L_UNIT), L_UNIT);
	printf("\t     Nozzle Exit: %12.5f (%s)\n",
			scio_convert(noz_e_dia, LENGTH, L_UNIT), L_UNIT);
	printf("\n");
	printf("\tNozzle Expansion: %12.4f\n",
			nozzle_exit_area / nozzle_throat_area);
	printf("\t        OF Ratio: %12.4f\n",
			n2o_flow_rate / fuel_flow_rate);

}

static void
report_output()
{
	printf("\nOutputs:\n");
	printf("\t              C*: %12.5f (%s)\n",
			scio_convert(c_star, VELOCITY, V_UNIT),
				V_UNIT);
	printf("\t   Exit Pressure: %12.5f (%s)\n",
			scio_convert(exit_pressure, PRESSURE, P_UNIT),
				P_UNIT);
	printf("\t       Nozzle Cf: %12.5f\n", nozzle_cf);
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

	nozzle_throat_area = pi/4. * noz_t_dia * noz_t_dia;
	nozzle_exit_area = pi/4. * noz_e_dia * noz_e_dia;
}


void
doit()
{
	cpropep();
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
