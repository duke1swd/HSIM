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
 * Unit test of the chamber model
 * This test does not call the tank model,
 * does implicitly depend on the injector and fuel models.
 *
 * Inputs
 *  n2o_liquid_density
 *  tank_pressure
 *  injector_area
 *  injector_cd
 *  grain core (port diameter)
 *  grain length
 *  nozzle throat diameter
 *  nozzle exit diameter
 *  combustion efficiency
 *  nozzle cf effectivness.
 *
 * Output
 *  chamber_pressure
 *  n2o_flow_rate
 *  cstar
 *  thrust
 *  ISP
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
#define	V1_UNIT	"fps"
#define	V2_UNIT	"m/s"
#define	F1_UNIT	"N"
#define	F2_UNIT	"lbf"
#define	T_UNIT	"sec"

static double injectordia;
static double noz_t_dia, noz_e_dia;

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
{
	"cstaradj",
	NUMBER,
	REQUIRED,
	&combustion_efficiency,
	1,
	0,
},
{
	"nozcfadj",
	NUMBER,
	REQUIRED,
	&nozzle_cf_correction,
	1,
	0,
},
};

static void
report_input()
{
	printf("Inputs:\n");
	printf("\t   N2O Liquid Density: %12.5f (%s)\n", 
			scio_convert(n2o_liquid_density, DENSITY, D_UNIT),
				D_UNIT);
	printf("\t        Tank Pressure: %12.5f (%s)\n",
			scio_convert(tank_pressure, PRESSURE, P_UNIT), P_UNIT);
	printf("\t         Injector Dia: %12.5f (%s)\n",
			scio_convert(injectordia, LENGTH, L_UNIT), L_UNIT);
	printf("\t          Injector Cd: %12.5f\n", injector_cd);
	printf("\t             Port Dia: %12.5f (%s)\n",
			scio_convert(grain_core, LENGTH, L_UNIT), L_UNIT);
	printf("\t         Grain Length: %12.5f (%s)\n",
			scio_convert(grain_length, LENGTH, L_UNIT), L_UNIT);
	printf("\t        Nozzle Throat: %12.5f (%s)\n",
			scio_convert(noz_t_dia, LENGTH, L_UNIT), L_UNIT);
	printf("\t          Nozzle Exit: %12.5f (%s)\n",
			scio_convert(noz_e_dia, LENGTH, L_UNIT), L_UNIT);
	printf("\tCombustion Efficiency: %9.2f\n", combustion_efficiency);
	printf("\t Nozzle Cf Adjustment: %9.2f\n", nozzle_cf_correction);
}

static void
report_output()
{
	printf("\nOutput:\n");
	printf("\t     Chamber Pressure: %12.5f (%s)\n",
			scio_convert(chamber_pressure,
				PRESSURE, P_UNIT), P_UNIT);
	printf("\t        N2O Flow Rate: %12.5f (%s)\n",
			scio_convert(n2o_flow_rate, MASSFLOW, MF_UNIT),
				MF_UNIT);
	printf("\t       Fuel Flow Rate: %12.5f (%s)\n",
			scio_convert(fuel_flow_rate, MASSFLOW, MF_UNIT),
				MF_UNIT);
	printf("\t             OF Ratio: %9.2f\n",
					n2o_flow_rate /fuel_flow_rate);
	printf("\t                   C*: %12.5f (%s)\n",
			scio_convert(c_star, VELOCITY, V1_UNIT),
				V1_UNIT);
	printf("\t        Exit Pressure: %12.5f (%s)\n",
			scio_convert(exit_pressure, PRESSURE, P_UNIT),
				P_UNIT);
	printf("\t            Nozzle Cf: %12.5f\n", nozzle_cf);
	printf("\t               Thrust: %12.5f (%s)\n",
			scio_convert(thrust, FORCE, F1_UNIT),
				F1_UNIT);
	printf("\t               Thrust: %12.5f (%s)\n",
			scio_convert(thrust, FORCE, F2_UNIT),
				F2_UNIT);
	printf("\t                  ISP: %12.5f (%s)\n",
			scio_convert(isp, VELOCITY, V2_UNIT), V2_UNIT);
	printf("\t                  ISP: %12.5f (%s)\n",
			scio_convert(isp/earth_gravity, TIME, T_UNIT),
				T_UNIT);
}
	

char *myname;

static void
initialize()
{
	struct ts_parsed_s *input_buffer;
	FILE *input;

	constants_init();
	fuel_init();
	chamber_init();

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

	injector_area = pi/4. * injectordia * injectordia;
	injector_count = 1;
	nozzle_throat_area = pi/4. * noz_t_dia * noz_t_dia;
	nozzle_exit_area = pi/4. * noz_e_dia * noz_e_dia;

	/*
	 * calculate the mass of fuel remaining.
	 */
	grain_diameter = grain_core + .002;
	fuel_mass = fuel_density * pi / 4. * grain_length *
		(grain_diameter * grain_diameter - grain_core * grain_core);
/*xxx*/if(0)chamber_pressure = 240 * 6894.7573;
}


void
doit()
{
	chamber();
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
