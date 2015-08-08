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
#include "linkage.h"
#include "state.h"

#define	P_UNIT	"psi"
#define	T_UNIT	"F"
#define	V_UNIT	"liter"
#define	L_UNIT	"in"
#define	M_UNIT	"kg"
#define	D_UNIT	"g/cc"
#define	E_UNIT	"joule"

double injector_dia;
double temp;

struct scio_input_parameter_s scio_input[] = {
{
	"tankvol",
	VOLUME,
	REQUIRED,
	&tank_volume,
	1,
	0,
},
{
	"injectordia",
	LENGTH,
	REQUIRED,
	&injector_dia,
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
	"tankn2omass",
	MASS,
	REQUIRED,
	&tank_n2o_mass,
	1,
	0,
},
{
	"temp",
	TEMPERATURE,
	REQUIRED,
	&temp,
	1,
	0,
},

};

static void
report_input()
{
	printf("Inputs:\n");
	printf("\t      Desired Temp: %12.5f (%s)\n", 
			scio_convert(temp, TEMPERATURE, T_UNIT), T_UNIT);
	printf("\t       Tank Volume: %12.5f (%s)\n",
			scio_convert(tank_volume, VOLUME, V_UNIT), V_UNIT);
	printf("\t          N2O Mass: %12.5f (%s)\n",
			scio_convert(tank_n2o_mass, MASS, M_UNIT), M_UNIT);
	printf("\t Injector Diameter: %12.5f (%s)\n",
			scio_convert(injector_dia, LENGTH, L_UNIT), L_UNIT);
	printf("\t       Injector Cd: %12.5f\n", injector_cd);
}

static void
report_output()
{
	printf("\nOutputs:\n");
	printf("\t       Tank Energy: %12.5f (%s)\n",
			scio_convert(tank_energy, ENERGY, E_UNIT), E_UNIT);
	printf("\t     Tank Pressure: %12.5f (%s)\n",
			scio_convert(tank_pressure, PRESSURE, P_UNIT), P_UNIT);
	printf("\t  Tank Temperature: %12.5f (%s)\n",
			scio_convert(tank_temperature, TEMPERATURE, T_UNIT),
				T_UNIT);
	printf("\t   N2O Liquid Mass: %12.5f (%s)\n",
			scio_convert(n2o_liquid_mass, MASS, M_UNIT), M_UNIT);
	printf("\t    N2O Vapor Mass: %12.5f (%s)\n",
			scio_convert(tank_n2o_mass - n2o_liquid_mass,
				MASS, M_UNIT), M_UNIT);
	printf("\tN2O Liquid Density: %12.5f (%s)\n",
			scio_convert(n2o_liquid_density, DENSITY, D_UNIT),
				D_UNIT);
}
	

char *myname;

static void
initialize()
{
	struct ts_parsed_s *input_buffer;
	FILE *input;

	constants_init();
	n2o_thermo_init();
	vent_area = 0.;	/* no vent for now */
	vent_cd = 1.;

	ts_parse_init();
	scio_init(scio_input, sizeof (scio_input) / sizeof (scio_input[0]));

	injector_area = injector_dia * injector_dia * pi / 4.;

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

static int
converged()
{
	double delta;

	delta = tank_temperature - temp;
	if (delta < 0)
		delta = -delta;

	return (delta < 0.01);
}

/*
 * Find the energy at which the tank is at the requested temp.
 */
void
doit()
{
	int i;
	int low_set;
	int high_set;
	double low_energy;
	double high_energy;

	low_set = 0;
	high_set = 0;
	tank_energy = 32768;

	i = 0;
	while (!low_set || !high_set || !converged()) {
		tank_temperature = temp;
		tank();

		if (n2o_thermo_error == TOO_HOT) {
			high_energy = tank_energy;
			high_set = 1;
			if (!low_set)
				tank_temperature -= 1;
		} else if (n2o_thermo_error == TOO_COLD) {
			low_energy = tank_energy;
			low_set= 1;
			if (!high_set)
				tank_temperature += 1;
		} else if (tank_temperature < temp) {
			low_energy = tank_energy;
			low_set= 1;
		} else {
			high_energy = tank_energy;
			high_set = 1;
		}

		if (!low_set)
			tank_energy /= 1.125;
		else if (!high_set)
			tank_energy *= 1.125;
		else
			tank_energy = (low_energy + high_energy) / 2.;
		if (tank_energy < 1.) {
			printf("FAILED: enery == 0\n");
			break;
		} else if (tank_energy > 1e6) {
			printf("FAILED: energy > 1,000,000\n");
			break;
		}
		if (i++ > 1000) {
			printf("FAILED %d iterations\n", i);
			break;
		}
	}
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
